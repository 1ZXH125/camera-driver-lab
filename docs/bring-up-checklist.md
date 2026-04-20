# Camera Bring-up Checklist

This checklist is meant to help camera driver engineers verify the full path from sensor hardware to userspace capture.

## 1. Hardware preparation

### Power and reset

- Confirm all sensor power rails are present and stable
- Verify the power-up sequence matches the sensor datasheet
- Check reset and power-down GPIO polarity
- Confirm the sensor enters a known idle state after power-on
- Validate standby and shutdown timing if the board uses them

### Clock and signal routing

- Verify input clock frequency and duty cycle
- Confirm the sensor master clock is enabled before probe
- Check MIPI lane routing and lane count
- Verify the CSI receiver lane mapping matches the sensor output
- Inspect connector or board-level signal integrity if capture is unstable

### I2C and board-level wiring

- Confirm I2C pull-ups are present and within spec
- Verify the sensor address matches the board design
- Check for bus contention or unexpected devices on the same bus
- Read back the chip ID register before enabling streaming

## 2. Device tree and kernel integration

### Device tree

- Confirm the sensor node exists in the device tree
- Verify `compatible` matches the driver binding
- Check endpoint and port connections
- Confirm clock, regulator, and GPIO phandles are correct
- Verify link frequency, data lanes, and remote endpoint settings

### Driver binding

- Confirm the sensor driver is built or loaded
- Check whether probe succeeds without warnings
- Verify subdevice registration is visible in kernel logs
- Make sure controls and formats are exposed as expected
- Confirm the media graph matches the physical pipeline

## 3. Basic functional validation

### Sensor identification

- Read the sensor chip ID
- Compare the value with the expected register map
- Check whether probe failure happens before or after ID read

### Format negotiation

- Query supported pixel formats
- Set the target resolution and fourcc format
- Confirm the negotiated format matches expectations
- Validate bytes-per-line and image size if the driver reports them

### Streaming

- Start streaming with a minimal userspace test
- Verify buffers are queued and dequeued correctly
- Confirm frames are delivered to userspace
- Check whether the first frame appears within the expected timeout
- Stop streaming cleanly and confirm the pipeline resets

## 4. Common failure categories

### Probe failures

Symptoms:

- sensor driver fails during initialization
- chip ID readback is invalid
- kernel reports `-EREMOTEIO`, `-ENODEV`, or similar errors

Checks:

- I2C wiring and pull-ups
- power rails
- reset/pwdn sequence
- correct sensor address
- device tree binding and regulator setup

### CSI timeout or sync errors

Symptoms:

- no image or black frame
- frame start timeout
- sync error in receiver logs
- capture works intermittently

Checks:

- lane count and lane mapping
- MIPI clock and link frequency
- sensor stream state
- receiver configuration
- board signal integrity

### Stream timeout

Symptoms:

- stream on succeeds but no frame is received
- userspace blocks waiting for buffer done
- frame rate is zero or extremely low

Checks:

- buffer queueing sequence
- stream-on/off order
- format and resolution settings
- whether the sensor is actually transmitting

### Format mismatch

Symptoms:

- image appears corrupted, green, or shifted
- resolution is wrong
- stride or buffer size does not match output format

Checks:

- pixel format
- bit depth and packing mode
- sensor and receiver configuration
- whether userspace interprets the frame correctly

## 5. Suggested debug order

1. Confirm power, reset, and clock are correct
2. Read back chip ID over I2C
3. Verify the device tree and driver binding
4. Check the media graph and endpoint connections
5. Validate the negotiated format
6. Start streaming with a simple userspace tool
7. Inspect kernel logs for timeout or sync issues
8. Compare sensor register configuration with a known-good reference

## 6. What to save during bring-up

- kernel logs from probe and streaming
- device tree snippets
- sensor register tables
- media graph output
- sample raw frames or screenshots
- notes about the exact board state and configuration

## 7. Good habits

- Change one variable at a time
- Keep known-good register tables under version control
- Save the exact command line used for each test
- Record whether failures are repeatable or intermittent
- Build a checklist from every issue you solve
