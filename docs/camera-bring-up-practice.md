# Camera Bring-up Practice Notes

This document is written like a real bring-up case review. The goal is to show how a camera issue is usually investigated in practice, what evidence matters, and how to turn a one-off debug session into reusable knowledge.

## Case background

A new sensor was integrated on a Linux platform. The target was simple:

- the sensor should bind successfully
- `VIDIOC_STREAMON` should work
- frames should arrive in userspace
- the output should match the expected resolution and format

The early symptom was not a clean image, but a mix of probe warnings, timeout logs, and inconsistent frame delivery.

## 1. First impression: do not start from the driver

The first mistake many junior engineers make is to immediately change driver code.

A better approach is to first answer three questions:

1. Is the sensor powered and clocked correctly?
2. Does I2C communication work reliably?
3. Does the kernel see the correct hardware topology?

If any of these are wrong, driver changes are usually noise.

### Hardware checks

Start with the most basic board-level items:

- sensor analog and digital rails are stable
- master clock exists and matches the datasheet expectation
- reset and power-down GPIO polarity are correct
- I2C pull-ups are present and the bus is not stuck low
- MIPI lanes are wired and mapped correctly

A useful habit is to write down the exact power-up and reset sequence before touching anything else.

## 2. Symptom-driven debugging

### Symptom A: chip ID cannot be read

This is usually the clearest sign that the problem is below the driver logic.

Typical log patterns:

- `-EREMOTEIO`
- chip ID read returns `0x0000`
- repeated I2C retry failures

What to check first:

- the I2C address matches the board wiring
- pull-ups are actually populated and within spec
- the sensor is not being held in reset
- the power rails are enabled in the correct order
- the clock is present before the first register access

In one real bring-up flow, the issue was not the register table at all. The sensor simply was not leaving reset in time for the first ID read.

### Symptom B: probe fails after the ID read

If the chip ID is readable but the driver still fails to bind, the problem is often in platform integration.

Check:

- device tree `compatible` string
- regulator configuration
- clock configuration
- endpoint and port connection
- subdevice registration path

This is the stage where a driver can appear “almost correct” but still fail because one resource in the dependency chain is wrong.

### Symptom C: stream on succeeds but no frame arrives

This is a classic camera issue.

Logs may show:

- stream-on success
- buffer queued
- frame timeout
- CSI receiver timeout or sync error

What to inspect:

- lane count and lane mapping
- whether the sensor actually enabled output
- whether the receiver is configured for the same format
- whether the queue/dequeue sequence is correct in userspace
- whether the first frame timeout is caused by a startup delay rather than a real failure

At this stage, `v4l2_capture_demo` is useful because it tells you whether the pipeline can actually deliver frames, independent of any vendor test application.

## 3. Use logs to narrow the failure class

A lot of time is wasted because log messages are read as isolated lines instead of categories.

A practical camera debug classification looks like this:

### I2C failure class

Signs:

- chip ID read fails
- `-EREMOTEIO`, `-ENXIO`, or similar
- probe aborts before streaming starts

Likely causes:

- address mismatch
- pull-up issue
- reset or power sequencing issue
- sensor not powered when I2C is accessed

### Probe failure class

Signs:

- sensor driver never binds cleanly
- initialization stops early
- dependency resources are missing

Likely causes:

- device tree mismatch
- regulator or clock not ready
- wrong endpoint routing
- unexpected platform data

### CSI timeout class

Signs:

- stream-on succeeds but frame start never arrives
- receiver reports timeout
- sync errors or lane errors appear

Likely causes:

- lane mapping mismatch
- sensor output disabled
- MIPI timing issue
- signal integrity problem

### Format mismatch class

Signs:

- image is green, scrambled, or rotated unexpectedly
- resolution is wrong
- bytes-per-line or packing mode does not match

Likely causes:

- pixel format mismatch
- Bayer order mismatch
- incorrect bit depth or packing
- wrong receiver configuration

## 4. A good practical workflow

A reliable workflow is usually boring, but it saves time:

1. Confirm board power, reset, and clock
2. Read sensor chip ID over I2C
3. Verify driver probe logs
4. Inspect the media graph and endpoint links
5. Confirm the negotiated format
6. Start streaming with a small V4L2 tool
7. Capture one raw frame and inspect it
8. Compare the working register table with the failing one
9. Classify the failure and document it for the next case

The key is to avoid guessing. Each step should either eliminate a class of problems or prove the current hypothesis.

## 5. How to use the tools in this repository

This repository is designed to support that workflow.

### `camera_log_analyzer`

Use it when you already have kernel logs and want to quickly sort them into rough failure categories.

Example:

```bash
./camera_log_analyzer examples/camera_probe_fail.log
```

### `v4l2_capture_demo`

Use it when you want to validate the capture path from userspace.

Example:

```bash
./v4l2_capture_demo -d /dev/video0 -w 640 -h 480 -f YUYV -n 30 -o frame.raw
```

### `sensor_reg_tool`

Use it when you want to compare two tuning sets and understand what changed.

Example:

```bash
./sensor_reg_tool examples/sensor_regs/base_regs.txt examples/sensor_regs/tuned_regs.txt
```

### `media_graph_dump`

Use it when you want a quick view of the video nodes exposed by the kernel.

Example:

```bash
./media_graph_dump /dev/video0
```

## 6. What mattered most in the actual debug

The most useful information usually came from the following artifacts:

- exact kernel logs from power-on to stream-off
- the working and failing sensor register tables
- device tree snippets for the camera pipeline
- the output of the capture tool
- notes on whether the failure was repeatable or intermittent

These are the pieces that make a bring-up case reusable instead of one-time memory.

## 7. Lessons learned

A good camera bring-up process is not about changing many things quickly.

It is about:

- keeping the hardware state explicit
- checking the dependency chain in order
- recording the exact log evidence
- comparing against a known-good reference
- turning every failure into a documented case

If you do that consistently, your personal knowledge base becomes stronger than a single project log.
