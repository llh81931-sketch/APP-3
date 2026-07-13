#pragma once
// 统一内存后端：仅 RT / TWT 内核驱动，无 syscall，无回退
#include "rt_driver.h"
#include "twt_driver.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>

enum class MemBackendKind {
    None = 0,
    Twt,
    Rt
};

class MemBackend {
public:
    TwtDriver twt;
    RtDriver rt;
    MemBackendKind kind = MemBackendKind::None;
    pid_t pid = -1;
    char last_path[256] = {0};

    bool connect_twt() {
        if (twt.connect()) {
            kind = MemBackendKind::Twt;
            snprintf(last_path, sizeof(last_path), "%s", twt.last_path());
            return true;
        }
        kind = MemBackendKind::None;
        last_path[0] = 0;
        return false;
    }

    bool connect_rt() {
        if (rt.connect()) {
            kind = MemBackendKind::Rt;
            snprintf(last_path, sizeof(last_path), "%s", rt.last_path());
            return true;
        }
        kind = MemBackendKind::None;
        last_path[0] = 0;
        return false;
    }

    // 自动对接：优先 TWT，其次 RT
    bool connect_auto() {
        if (kind != MemBackendKind::None) return true;
        if (connect_twt()) return true;
        if (connect_rt()) return true;
        return false;
    }

    bool ready() const {
        if (kind == MemBackendKind::Twt) return twt.ready();
        if (kind == MemBackendKind::Rt) return rt.ready();
        return false;
    }

    const char* name() const {
        if (kind == MemBackendKind::Twt) return "TWT";
        if (kind == MemBackendKind::Rt) return "RT";
        return "NONE";
    }

    const char* path() const { return last_path; }

    void set_pid(pid_t p) {
        pid = p;
        if (kind == MemBackendKind::Twt) twt.set_pid(p);
        else if (kind == MemBackendKind::Rt) rt.set_pid(p);
    }

    bool read(uintptr_t addr, void* buffer, size_t size) {
        if (!ready() || pid <= 0) return false;
        if (kind == MemBackendKind::Twt) return twt.read_mem(addr, buffer, size);
        if (kind == MemBackendKind::Rt) return rt.read_mem(addr, buffer, size);
        return false;
    }

    bool write(uintptr_t addr, void* buffer, size_t size) {
        if (!ready() || pid <= 0) return false;
        if (kind == MemBackendKind::Twt) return twt.write_mem(addr, buffer, size);
        if (kind == MemBackendKind::Rt) return rt.write_mem(addr, buffer, size);
        return false;
    }

    uintptr_t module_base(const char* module_name) {
        if (!ready() || pid <= 0 || !module_name) return 0;
        if (kind == MemBackendKind::Twt) {
            uintptr_t b = twt.module_base(module_name);
            if (b) return b;
        } else if (kind == MemBackendKind::Rt) {
            uintptr_t b = rt.module_base(module_name);
            if (b) return b;
        }
        return maps_module_base(pid, module_name);
    }

    static uintptr_t maps_module_base(pid_t p, const char* module_name, int index = 1) {
        if (p <= 0 || !module_name || index < 1) return 0;
        char filename[64];
        char line[1024];
        snprintf(filename, sizeof(filename), "/proc/%d/maps", p);
        FILE* fp = fopen(filename, "r");
        if (!fp) return 0;
        uintptr_t addr = 0;
        int hit = 0;
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, module_name)) {
                ++hit;
                if (hit == index) {
                    sscanf(line, "%lx-%*lx", &addr);
                    if (addr == 0x8000) addr = 0;
                    break;
                }
            }
        }
        fclose(fp);
        return addr;
    }

    // 取模块后的第 bss_index 个 [anon:.bss]（默认 1 = bss[1]）
    // 语义对齐原工程 GetModuleBase(pid, "libxxx.so:bss", index)
    static uintptr_t maps_module_bss(pid_t p, const char* module_name, int module_index = 1, int bss_index = 1) {
        if (p <= 0 || !module_name || module_index < 1 || bss_index < 1) return 0;
        char filename[64];
        char line[1024];
        snprintf(filename, sizeof(filename), "/proc/%d/maps", p);
        FILE* fp = fopen(filename, "r");
        if (!fp) return 0;

        int mod_hit = 0;
        bool armed = false; // 已命中目标模块映射，开始找随后的 bss
        int bss_hit = 0;
        uintptr_t addr = 0;

        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, module_name)) {
                ++mod_hit;
                if (mod_hit == module_index) {
                    armed = true;
                }
                // 目标模块自身多段映射：保持 armed，继续向后扫 bss
                continue;
            }
            if (!armed) continue;

            // 遇到其它已命名 .so 文件段则停止，防止跨到下一模块
            if (strstr(line, ".so") && strchr(line, '/')) {
                break;
            }

            if (strstr(line, "[anon:.bss]")) {
                ++bss_hit;
                if (bss_hit == bss_index) {
                    sscanf(line, "%lx-%*lx", &addr);
                    break;
                }
            }
        }
        fclose(fp);
        return addr;
    }

    static int find_pid_by_name(const char* packageName) {
        if (!packageName) return -1;
        DIR* dir = opendir("/proc");
        if (!dir) return -1;
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            int id = atoi(entry->d_name);
            if (id <= 0) continue;
            char filename[64];
            char cmdline[256];
            snprintf(filename, sizeof(filename), "/proc/%d/cmdline", id);
            FILE* fp = fopen(filename, "r");
            if (!fp) continue;
            if (fgets(cmdline, sizeof(cmdline), fp)) {
                fclose(fp);
                if (strcmp(packageName, cmdline) == 0) {
                    closedir(dir);
                    return id;
                }
            } else {
                fclose(fp);
            }
        }
        closedir(dir);
        return -1;
    }
};

inline MemBackend& Mem() {
    static MemBackend g;
    return g;
}
