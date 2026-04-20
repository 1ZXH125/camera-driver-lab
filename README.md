# camera-driver-lab

`camera-driver-lab` is a practical lab for Linux camera driver engineers.

It collects small, focused tools and notes around the most common camera bring-up and validation tasks:

- V4L2 capture experiments
- camera bring-up checklists
- sensor register table helpers
- log analysis for common camera failures
- MIPI CSI-2 and device tree notes

The goal is to turn day-to-day debugging experience into reusable tools, scripts, and knowledge.

## What this repository is for

Camera driver work usually sits at the intersection of hardware, kernel logs, sensor configuration, and userspace validation. This repository helps with:

- understanding the capture pipeline from sensor to userspace
- reproducing and diagnosing common camera issues
- building a personal knowledge base for bring-up and debugging
- showcasing practical camera driver experience on GitHub and in a résumé

## Included tools

### `v4l2_capture_demo`

A minimal Linux V4L2 capture utility that can:

- open a `/dev/videoX` device
- query device capabilities
- negotiate pixel format and resolution
- allocate `mmap` buffers
- start streaming and capture frames
- print frame sequence, size, and timestamp
- save the first captured frame to disk
- report elapsed time and approximate FPS

### `camera_log_analyzer`

A lightweight log parser for common camera bring-up failures. It scans kernel logs or text logs and highlights patterns such as:

- I2C read failures
- probe failures
- CSI timeout errors
- stream timeout errors
- clock or power issues
- format mismatch issues
- sensor ID problems

It also prints a short suggestion for each detected issue.

### `sensor_reg_tool`

A register table parser and diff tool for sensor tuning work. It can:

- parse simple register table formats
- print a normalized register list
- compare two register sets
- report added, removed, and changed registers

### `media_graph_dump`

A small sysfs-based helper that lists Linux video nodes and related device links. It is useful for quickly checking what the kernel exposes under `/sys/class/video4linux`.

## Repository layout

- `src/` — runnable tools and experiments
- `docs/` — bring-up notes, protocol summaries, and debugging workflows
- `scripts/` — helper scripts for log parsing or config generation
- `examples/` — sample logs, configs, and register tables
- `tests/` — regression cases and fixtures

## Build

```bash
make
```

## Run

### V4L2 capture demo

```bash
./v4l2_capture_demo -d /dev/video0 -w 640 -h 480 -f YUYV -n 30 -o frame.raw
```

### Camera log analyzer

Analyze a log file:

```bash
./camera_log_analyzer examples/camera_probe_fail.log
```

Or pipe a log into it:

```bash
dmesg | ./camera_log_analyzer
```

### Sensor register tool

```bash
./sensor_reg_tool examples/sensor_regs/base_regs.txt examples/sensor_regs/tuned_regs.txt
```

### Media graph dump

```bash
./media_graph_dump
./media_graph_dump /dev/video0
```

## Example workflow

1. Confirm the sensor is probed successfully
2. Check the negotiated format
3. Capture several frames
4. Save one raw frame for inspection
5. Compare the result with expected sensor output
6. Feed kernel logs into the analyzer to classify common failure patterns
7. Compare sensor register tables before and after tuning

## Docs

Start here:

- `docs/bring-up-checklist.md`
- `docs/camera-bring-up-practice.md`
- `docs/mipi-csi2-notes.md`

## Planned improvements

- more real-world failure cases
- additional sensor register samples
- device tree examples
- media controller topology notes
- automated log classification rules

## License

This repository is intended for learning and personal portfolio use unless a separate license is added later.
