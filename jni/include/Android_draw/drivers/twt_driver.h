#pragma once
// TWT 内核驱动对接（静默，无交互；不启用陀螺仪/触摸初始化）
#include <cstdint>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <limits.h>

#define TWT_MY_CALL(magic1, magic2, cmd, arg) ({\
    long _ret;\
    register long _x0 __asm__("x0") = (long)(magic1);\
    register long _x1 __asm__("x1") = (long)(magic2);\
    register long _x2 __asm__("x2") = (long)(cmd);\
    register long _x3 __asm__("x3") = (long)(arg);\
    register long _nr __asm__("x8") = __NR_reboot;\
    __asm__ __volatile__("svc #0" : "=r"(_x0) : "r"(_x0), "r"(_x1), "r"(_x2), "r"(_x3), "r"(_nr) : "memory", "cc");\
    _ret = _x0;\
    _ret;\
})

#define TWT_MARK 'T'
#define TWT_GET_PID     _IOW(TWT_MARK, 0, TwtRequest)
#define TWT_MODULE_BASE _IOW(TWT_MARK, 1, TwtRequest)
#define TWT_MODULE_BSS  _IOW(TWT_MARK, 3, TwtRequest)
#define TWT_READ_MEM    _IOW(TWT_MARK, 4, TwtRequest)
#define TWT_READ_MEM_V2 _IOW(TWT_MARK, 11, TwtRequest)
#define TWT_WRITE_MEM   _IOW(TWT_MARK, 5, TwtRequest)

struct TwtRequest {
    pid_t pid;
    uintptr_t addr;
    void* buffer;
    size_t size;
};

class TwtDriver {
private:
    int fd = -1;
    pid_t pid = -1;
    char last_path_[256] = {0};

    static int find_anon_fd(const char* key) {
        DIR* dir = opendir("/proc/self/fd");
        if (!dir) return -1;
        int found = -1;
        struct dirent* entry;
        char path[PATH_MAX], link[PATH_MAX];
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_name[0] == '.') continue;
            snprintf(path, sizeof(path), "/proc/self/fd/%s", entry->d_name);
            ssize_t len = readlink(path, link, sizeof(link) - 1);
            if (len < 0) continue;
            link[len] = '\0';
            if (strstr(link, key) && strstr(link, "anon_inode:")) {
                found = atoi(entry->d_name);
                break;
            }
        }
        closedir(dir);
        return found;
    }

public:
    bool connect() {
        if (fd >= 0) return true;
#if defined(__aarch64__)
        int tmp = -1;
        TWT_MY_CALL(0x114514, 0x1919810, 0x2778, &tmp);
        if (tmp >= 0) {
            fd = tmp;
            snprintf(last_path_, sizeof(last_path_), "anon_inode:TwT_driver (fd=%d, call)", fd);
            return true;
        }
#endif
        int afd = find_anon_fd("TwT_driver");
        if (afd >= 0) {
            fd = afd;
            snprintf(last_path_, sizeof(last_path_), "anon_inode:TwT_driver (fd=%d)", fd);
            return true;
        }
        last_path_[0] = 0;
        return false;
    }

    void close_driver() {
        // anon fd 由内核侧持有，这里只断开本地引用
        fd = -1;
        pid = -1;
    }

    bool ready() const { return fd >= 0; }
    const char* last_path() const { return last_path_; }
    void set_pid(pid_t p) { pid = p; }
    pid_t get_pid() const { return pid; }

    bool read_mem(uintptr_t addr, void* buffer, size_t size) {
        if (fd < 0 || pid <= 0 || !buffer || size == 0) return false;
        TwtRequest req{};
        addr &= 0xFFFFFFFFFFFFULL;
        req.pid = pid;
        req.addr = addr;
        req.buffer = buffer;
        req.size = size;
        // 优先 V2
        if (ioctl(fd, TWT_READ_MEM_V2, &req) == 0) return true;
        return ioctl(fd, TWT_READ_MEM, &req) == 0;
    }

    bool write_mem(uintptr_t addr, void* buffer, size_t size) {
        if (fd < 0 || pid <= 0 || !buffer || size == 0) return false;
        TwtRequest req{};
        addr &= 0xFFFFFFFFFFFFULL;
        req.pid = pid;
        req.addr = addr;
        req.buffer = buffer;
        req.size = size;
        return ioctl(fd, TWT_WRITE_MEM, &req) == 0;
    }

    pid_t name_to_pid(const char* name) {
        if (fd < 0 || !name) return -1;
        TwtRequest req{};
        char buf[0x100];
        snprintf(buf, sizeof(buf), "%s", name);
        req.pid = 0;
        req.buffer = buf;
        if (ioctl(fd, TWT_GET_PID, &req) != 0) return -1;
        return req.pid;
    }

    uintptr_t module_base(const char* name) {
        if (fd < 0 || pid <= 0 || !name) return 0;
        TwtRequest req{};
        char buf[0x100];
        snprintf(buf, sizeof(buf), "%s", name);
        req.pid = pid;
        req.addr = 0;
        req.buffer = buf;
        if (ioctl(fd, TWT_MODULE_BASE, &req) != 0) return 0;
        return req.addr;
    }

    uintptr_t module_bss(const char* name) {
        if (fd < 0 || pid <= 0 || !name) return 0;
        TwtRequest req{};
        char buf[0x100];
        snprintf(buf, sizeof(buf), "%s", name);
        req.pid = pid;
        req.addr = 0;
        req.buffer = buf;
        if (ioctl(fd, TWT_MODULE_BSS, &req) != 0) return 0;
        return req.addr;
    }
};
