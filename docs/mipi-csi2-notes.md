# MIPI CSI-2 Notes

These notes summarize the parts of MIPI CSI-2 that are most useful during camera driver bring-up and debugging.

## 1. What CSI-2 does

MIPI CSI-2 is the packet-based interface used by many image sensors to send pixel data to a receiver such as an ISP, bridge chip, or SoC camera controller.

In practice, a camera driver engineer usually needs to understand:

- how many lanes are active
- what pixel format is being sent
- whether the sensor is actually streaming
- whether the receiver is decoding the stream correctly
- whether the frame data is stable and complete

## 2. Key concepts

### Lane count

The number of data lanes used by the sensor.

Common values:

- 1 lane
- 2 lanes
- 4 lanes

The lane count configured in the sensor, device tree, and receiver must match.

### Lane mapping

The physical mapping of data lanes to the receiver input.

A mismatch here often causes:

- no frame start
- timeout errors
- intermittent capture
- receiver sync errors

### Virtual channel

A logical channel identifier used to distinguish multiple streams on the same CSI-2 link.

It is less common in simple bring-up, but it matters in multi-sensor or multiplexed designs.

### Data type

CSI-2 uses packet data types to describe the payload.

The receiver must interpret the incoming data type correctly, or the output may appear corrupted even if the link is active.

### Short packet and long packet

- Short packets carry control information such as frame start and frame end.
- Long packets carry the actual image payload.

If short packets are missing, the receiver may never detect a valid frame boundary.

## 3. Typical bring-up checklist for CSI-2

### Before streaming

- Confirm sensor power rails are stable
- Confirm sensor clock input is enabled
- Confirm reset and power-down lines are in the expected state
- Read back sensor ID over I2C
- Verify the sensor driver has bound successfully
- Confirm the device tree lane configuration matches the board
- Confirm the receiver lane mapping is correct

### During streaming

- Check whether the sensor reports stream-on success
- Confirm frames are arriving at the receiver
- Verify the first frame start appears in logs or analyzer output
- Check for timeout or sync error messages
- Confirm the negotiated format is consistent with the sensor output

### After streaming

- Verify frame content is stable
- Check for repeated lines, tearing, or color shifts
- Confirm stream-off resets the pipeline cleanly

## 4. Common failure patterns

### No image

Likely causes:

- sensor not powered correctly
- missing clock
- reset sequence incorrect
- I2C probe failed
- receiver not enabled

### Black frame

Likely causes:

- sensor is probed but not streaming
- stream-on sequence incomplete
- exposure or test pattern configuration issue
- receiver is receiving data but userspace is not interpreting it correctly

### Color distortion

Likely causes:

- pixel format mismatch
- wrong bit depth or packing mode
- Bayer order mismatch
- stride or buffer interpretation issue

### Timeout

Likely causes:

- lane mismatch
- wrong lane count
- incorrect link frequency or timing
- sensor clock instability
- receiver configuration issue

### Intermittent frames

Likely causes:

- marginal signal integrity
- unstable clock
- weak power sequencing
- buffer queueing or userspace timing issue

## 5. Practical debug flow

1. Confirm the sensor is powered and clocked
2. Read chip ID successfully over I2C
3. Check device tree and media graph configuration
4. Verify lane count and lane mapping
5. Start streaming with a known-good resolution
6. Inspect kernel logs for timeout or sync errors
7. Compare output against a known-good sensor register table
8. Reduce variables until the failure becomes reproducible

## 6. What to record during CSI debugging

- lane count and lane mapping
- sensor output format
- link frequency
- receiver configuration
- kernel log snippets
- raw frame samples
- known-good register tables

## 7. Good engineering habits

- Keep one known-good CSI configuration for reference
- Change only one parameter at a time
- Save logs before and after each change
- Document whether the issue is repeatable or intermittent
- Tie every timeout or sync error back to the exact hardware and software state
