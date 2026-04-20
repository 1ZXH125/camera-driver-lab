# Examples

This folder contains sample logs and sensor register tables for the tools in this repository.

## Log samples

- `camera_probe_fail.log`
- `csi_timeout.log`
- `stream_timeout.log`
- `i2c_read_fail.log`

Use them with `camera_log_analyzer`:

```bash
./camera_log_analyzer examples/camera_probe_fail.log
```

## Register table samples

- `sensor_regs/base_regs.txt`
- `sensor_regs/tuned_regs.txt`

Use them with `sensor_reg_tool`:

```bash
./sensor_reg_tool examples/sensor_regs/base_regs.txt examples/sensor_regs/tuned_regs.txt
```
