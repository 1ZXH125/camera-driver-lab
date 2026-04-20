#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFFER_COUNT 4

typedef struct {
    void *start;
    size_t length;
} Buffer;

static int xioctl(int fd, unsigned long request, void *arg) {
    int r;
    do {
        r = ioctl(fd, request, arg);
    } while (r == -1 && errno == EINTR);
    return r;
}

static void die(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static double now_seconds(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1000000.0;
}

static int save_frame(const char *path, const void *data, size_t size) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return -1;
    size_t written = fwrite(data, 1, size, fp);
    fclose(fp);
    return written == size ? 0 : -1;
}

static void usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s -d /dev/videoX [-w width] [-H height] [-f pixfmt] [-n frames] [-o output]\n"
            "  -d device   video device path\n"
            "  -w width    frame width (default 640)\n"
            "  -H height   frame height (default 480)\n"
            "  -f pixfmt   fourcc, default YUYV\n"
            "  -n frames   number of frames to capture (default 30)\n"
            "  -o output   save first frame to this file\n",
            prog);
}

static uint32_t fourcc_from_string(const char *s) {
    if (strlen(s) != 4) return 0;
    return v4l2_fourcc(s[0], s[1], s[2], s[3]);
}

int main(int argc, char **argv) {
    const char *device = NULL;
    const char *output = NULL;
    const char *fmt_str = "YUYV";
    int width = 640;
    int height = 480;
    int frame_count = 30;

    int opt;
    while ((opt = getopt(argc, argv, "d:w:H:f:n:o:")) != -1) {
        switch (opt) {
            case 'd': device = optarg; break;
            case 'w': width = atoi(optarg); break;
            case 'H': height = atoi(optarg); break;
            case 'f': fmt_str = optarg; break;
            case 'n': frame_count = atoi(optarg); break;
            case 'o': output = optarg; break;
            default:
                usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (!device) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    uint32_t pixfmt = fourcc_from_string(fmt_str);
    if (!pixfmt) {
        fprintf(stderr, "Invalid fourcc format: %s\n", fmt_str);
        return EXIT_FAILURE;
    }

    int fd = open(device, O_RDWR | O_NONBLOCK, 0);
    if (fd < 0) die("open");

    struct v4l2_capability cap;
    if (xioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) die("VIDIOC_QUERYCAP");

    printf("Driver: %s\nCard: %s\nBus: %s\n\n", cap.driver, cap.card, cap.bus_info);

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = pixfmt;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    if (xioctl(fd, VIDIOC_S_FMT, &fmt) < 0) die("VIDIOC_S_FMT");

    printf("Negotiated format: %c%c%c%c %ux%u\n",
           fmt.fmt.pix.pixelformat & 0xff,
           (fmt.fmt.pix.pixelformat >> 8) & 0xff,
           (fmt.fmt.pix.pixelformat >> 16) & 0xff,
           (fmt.fmt.pix.pixelformat >> 24) & 0xff,
           fmt.fmt.pix.width,
           fmt.fmt.pix.height);

    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (xioctl(fd, VIDIOC_REQBUFS, &req) < 0) die("VIDIOC_REQBUFS");
    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory\n");
        close(fd);
        return EXIT_FAILURE;
    }

    Buffer *buffers = calloc(req.count, sizeof(*buffers));
    if (!buffers) die("calloc");

    for (uint32_t i = 0; i < req.count; ++i) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (xioctl(fd, VIDIOC_QUERYBUF, &buf) < 0) die("VIDIOC_QUERYBUF");

        buffers[i].length = buf.length;
        buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (buffers[i].start == MAP_FAILED) die("mmap");

        if (xioctl(fd, VIDIOC_QBUF, &buf) < 0) die("VIDIOC_QBUF");
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (xioctl(fd, VIDIOC_STREAMON, &type) < 0) die("VIDIOC_STREAMON");

    printf("Capturing %d frames...\n", frame_count);

    double start = now_seconds();
    int saved = 0;
    int captured = 0;

    for (int i = 0; i < frame_count; ++i) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        int r = select(fd + 1, &fds, NULL, NULL, &tv);
        if (r < 0) {
            if (errno == EINTR) {
                --i;
                continue;
            }
            die("select");
        }
        if (r == 0) {
            fprintf(stderr, "Frame timeout\n");
            break;
        }

        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (xioctl(fd, VIDIOC_DQBUF, &buf) < 0) {
            if (errno == EAGAIN) {
                --i;
                continue;
            }
            die("VIDIOC_DQBUF");
        }

        ++captured;
        printf("Frame %d: bytes=%u, seq=%u, timestamp=%ld.%06ld\n",
               captured,
               buf.bytesused,
               buf.sequence,
               buf.timestamp.tv_sec,
               buf.timestamp.tv_usec);

        if (output && !saved) {
            if (save_frame(output, buffers[buf.index].start, buf.bytesused) == 0) {
                printf("Saved first frame to %s\n", output);
            } else {
                fprintf(stderr, "Failed to save frame to %s\n", output);
            }
            saved = 1;
        }

        if (xioctl(fd, VIDIOC_QBUF, &buf) < 0) die("VIDIOC_QBUF");
    }

    if (xioctl(fd, VIDIOC_STREAMOFF, &type) < 0) die("VIDIOC_STREAMOFF");

    double elapsed = now_seconds() - start;
    if (elapsed > 0.0 && captured > 0) {
        printf("Elapsed: %.3f sec, FPS: %.2f\n", elapsed, captured / elapsed);
    }

    for (uint32_t i = 0; i < req.count; ++i) {
        munmap(buffers[i].start, buffers[i].length);
    }
    free(buffers);
    close(fd);
    return EXIT_SUCCESS;
}
