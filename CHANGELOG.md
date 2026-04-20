# Changelog

All notable changes to `camera-driver-lab` are documented in this file.

The project is still early and focused on building practical camera driver learning material.

## [Unreleased]

### Added

- `v4l2_capture_demo` for basic Linux V4L2 capture validation
- `camera_log_analyzer` for classifying common camera bring-up failures
- `sensor_reg_tool` for parsing and comparing sensor register tables
- `media_graph_dump` for listing video devices and related sysfs links
- sample logs under `examples/` for probe, CSI timeout, stream timeout, and I2C read failures
- sample sensor register tables under `examples/sensor_regs/`
- bring-up notes under `docs/`
- a practical bring-up checklist and CSI-2 notes

### Changed

- expanded the project README into a more complete GitHub-style overview
- added a more realistic workflow for camera bring-up and debugging
- improved the documentation structure for easier navigation

## 0.1.0 - 2026-04-20

### Initial release

- initial project scaffold
- first camera capture demo
- log analysis tool
- register table utility
- media graph inspection helper
- documentation and examples for camera bring-up learning
