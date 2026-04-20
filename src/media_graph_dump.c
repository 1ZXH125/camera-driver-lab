#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define SYS_MEDIA_PATH "/sys/class/video4linux"
#define MAX_PATH 512

static void usage(const char *prog) {
    fprintf(stderr,
            "Usage: %s [video_device]\n"
            "  If no device is given, all video devices under /sys/class/video4linux are scanned.\n",
            prog);
}

static int read_first_line(const char *path, char *buf, size_t size) {
    FILE *fp = fopen(path, "r");
    if (!fp) return -1;
    if (!fgets(buf, (int)size, fp)) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    buf[strcspn(buf, "\n")] = '\0';
    return 0;
}

static void print_video_device(const char *name) {
    char base[MAX_PATH];
    snprintf(base, sizeof(base), "%s/%s", SYS_MEDIA_PATH, name);

    char devnode[MAX_PATH];
    char device_path[MAX_PATH];
    char name_path[MAX_PATH];
    char driver_path[MAX_PATH];
    char modalias_path[MAX_PATH];

    snprintf(devnode, sizeof(devnode), "%s/dev", base);
    snprintf(device_path, sizeof(device_path), "%s/device", base);
    snprintf(name_path, sizeof(name_path), "%s/name", base);
    snprintf(driver_path, sizeof(driver_path), "%s/device/driver/module/drivers", base);
    snprintf(modalias_path, sizeof(modalias_path), "%s/device/modalias", base);

    char buf[256];
    char target[512];
    ssize_t len;

    printf("=== %s ===\n", name);

    if (read_first_line(name_path, buf, sizeof(buf)) == 0) {
        printf("name: %s\n", buf);
    }

    if (read_first_line(devnode, buf, sizeof(buf)) == 0) {
        printf("dev: %s\n", buf);
    }

    len = readlink(device_path, target, sizeof(target) - 1);
    if (len >= 0) {
        target[len] = '\0';
        printf("device link: %s\n", target);
    }

    if (read_first_line(modalias_path, buf, sizeof(buf)) == 0) {
        printf("modalias: %s\n", buf);
    }

    if (access(driver_path, R_OK) == 0) {
        printf("driver info: available under %s\n", driver_path);
    }

    printf("\n");
}

static void scan_all_devices(void) {
    DIR *dir = opendir(SYS_MEDIA_PATH);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "video", 5) == 0) {
            print_video_device(ent->d_name);
        }
    }

    closedir(dir);
}

int main(int argc, char **argv) {
    if (argc > 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (argc == 1) {
        scan_all_devices();
        return EXIT_SUCCESS;
    }

    const char *device = argv[1];
    const char *base = strrchr(device, '/');
    if (base) {
        device = base + 1;
    }

    if (strncmp(device, "video", 5) != 0) {
        fprintf(stderr, "Expected a video device name such as video0 or /dev/video0\n");
        return EXIT_FAILURE;
    }

    print_video_device(device);
    return EXIT_SUCCESS;
}
