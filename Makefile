CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -std=c11

TARGETS = v4l2_capture_demo camera_log_analyzer sensor_reg_tool media_graph_dump

all: $(TARGETS)

v4l2_capture_demo: src/v4l2_capture_demo.c
	$(CC) $(CFLAGS) -o $@ $<

camera_log_analyzer: src/camera_log_analyzer.c
	$(CC) $(CFLAGS) -o $@ $<

sensor_reg_tool: src/sensor_reg_tool.c
	$(CC) $(CFLAGS) -o $@ $<

media_graph_dump: src/media_graph_dump.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
