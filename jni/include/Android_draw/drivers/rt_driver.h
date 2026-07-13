#pragma once
// RT 内核驱动对接（静默，无交互）
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

class RtDriver {
private:
    int fd = -1;
    pid_t pid = -1;
    char last_path_[256] = {0};

    struct COPY_MEMORY {
        pid_t pid;
        uintptr_t addr;
        void* buffer;
        size_t size;
    };
    struct MODULE_BASE {
        pid_t pid;
        char* name;
        uintptr_t base;
    };
    enum OPERATIONS {
        OP_INIT_KEY = 0x800,
        OP_READ_MEM = 0x801,
        OP_WRITE_MEM = 0x802,
        OP_MODULE_BASE = 0x803
    };

    static char* find_dev_driver() {
        const char* dev_path = "/dev";
        DIR* dir = opendir(dev_path);
        if (!dir) return nullptr;

        const char* known[] = { "wanbai", "CheckMe", "Ckanri", "lanran", "video188" };
        struct dirent* entry;
        char* found = nullptr;

        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            size_t path_length = strlen(dev_path) + strlen(entry->d_name) + 2;
            char* file_path = (char*)malloc(path_length);
            if (!file_path) continue;
            snprintf(file_path, path_length, "%s/%s", dev_path, entry->d_name);

            for (int i = 0; i < 5; ++i) {
                if (strcmp(entry->d_name, known[i]) == 0) {
                    closedir(dir);
                    return file_path;
                }
            }

            struct stat file_info;
            if (stat(file_path, &file_info) < 0) { free(file_path); continue; }
            if (strstr(entry->d_name, "gpiochip") != nullptr) { free(file_path); continue; }

            if ((S_ISCHR(file_info.st_mode) || S_ISBLK(file_info.st_mode))
                && strchr(entry->d_name, '_') == nullptr
                && strchr(entry->d_name, '-') == nullptr
                && strchr(entry->d_name, ':') == nullptr) {
                if (strcmp(entry->d_name, "stdin") == 0 || strcmp(entry->d_name, "stdout") == 0
                    || strcmp(entry->d_name, "stderr") == 0) {
                    free(file_path);
                    continue;
                }
                size_t name_len = strlen(entry->d_name);
                time_t current_time; time(&current_time);
                int file_year = localtime(&file_info.st_ctime)->tm_year + 1900;
                if (file_year <= 1980) { free(file_path); continue; }
                if (file_info.st_atime == file_info.st_ctime
                    && (file_info.st_mode & S_IFMT) == 8192
                    && file_info.st_size == 0
                    && file_info.st_gid == 0 && file_info.st_uid == 0
                    && name_len <= 9) {
                    found = file_path;
                    closedir(dir);
                    return found;
                }
            }
            free(file_path);
        }
        closedir(dir);
        return nullptr;
    }

    static char* find_proc_driver() {
        DIR* dr = opendir("/proc");
        if (!dr) return nullptr;
        struct dirent* de;
        char* device_path = nullptr;
        while ((de = readdir(dr)) != nullptr) {
            if (strlen(de->d_name) != 6) continue;
            if (strcmp(de->d_name, "NVTSPI") == 0 || strcmp(de->d_name, "ccci_log") == 0
                || strcmp(de->d_name, "aputag") == 0 || strcmp(de->d_name, "asound") == 0
                || strcmp(de->d_name, "clkdbg") == 0 || strcmp(de->d_name, "crypto") == 0
                || strcmp(de->d_name, "modules") == 0 || strcmp(de->d_name, "mounts") == 0
                || strcmp(de->d_name, "pidmap") == 0 || strcmp(de->d_name, "phoenix") == 0
                || strcmp(de->d_name, "uptime") == 0 || strcmp(de->d_name, "vmstat") == 0)
                continue;
            int valid = 1;
            for (int i = 0; i < 6; ++i) {
                if (!isalnum((unsigned char)de->d_name[i])) { valid = 0; break; }
            }
            if (!valid) continue;
            device_path = (char*)malloc(11 + strlen(de->d_name));
            if (!device_path) continue;
            sprintf(device_path, "/proc/%s", de->d_name);
            struct stat sb;
            if (stat(device_path, &sb) == 0 && S_ISREG(sb.st_mode)) {
                closedir(dr);
                return device_path;
            }
            free(device_path);
            device_path = nullptr;
        }
        closedir(dr);
        return nullptr;
    }

    bool open_path(const char* path) {
        if (!path) return false;
        int tfd = open(path, O_RDWR);
        if (tfd < 0) return false;
        fd = tfd;
        snprintf(last_path_, sizeof(last_path_), "%s", path);
        return true;
    }

public:
    bool connect() {
        if (fd >= 0) return true;
        char* p = find_dev_driver();
        if (p) {
            bool ok = open_path(p);
            free(p);
            if (ok) return true;
        }
        p = find_proc_driver();
        if (p) {
            bool ok = open_path(p);
            free(p);
            if (ok) return true;
        }
        return false;
    }

    void close_driver() {
        if (fd >= 0) { close(fd); fd = -1; }
        pid = -1;
    }

    bool ready() const { return fd >= 0; }
    const char* last_path() const { return last_path_; }
    void set_pid(pid_t p) { pid = p; }
    pid_t get_pid() const { return pid; }

    bool read_mem(uintptr_t addr, void* buffer, size_t size) {
        if (fd < 0 || pid <= 0 || !buffer || size == 0) return false;
        COPY_MEMORY cm{};
        cm.pid = pid;
        cm.addr = addr;
        cm.buffer = buffer;
        cm.size = size;
        return ioctl(fd, OP_READ_MEM, &cm) == 0;
    }

    bool write_mem(uintptr_t addr, void* buffer, size_t size) {
        if (fd < 0 || pid <= 0 || !buffer || size == 0) return false;
        COPY_MEMORY cm{};
        cm.pid = pid;
        cm.addr = addr;
        cm.buffer = buffer;
        cm.size = size;
        return ioctl(fd, OP_WRITE_MEM, &cm) == 0;
    }

    uintptr_t module_base(const char* name) {
        if (fd < 0 || pid <= 0 || !name) return 0;
        MODULE_BASE mb{};
        char buf[0x100];
        snprintf(buf, sizeof(buf), "%s", name);
        mb.pid = pid;
        mb.name = buf;
        mb.base = 0;
        if (ioctl(fd, OP_MODULE_BASE, &mb) != 0) return 0;
        return mb.base;
    }
};
