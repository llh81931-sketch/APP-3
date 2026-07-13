#pragma once
// 失控进化完整功能层：绘制/自瞄逻辑来自游戏数据功能工程
// 内存读写统一走 MemDriver（RT/TWT），无 process_vm 回退
#include "ImGui/imgui.h"
#include "sys读写.h"
#include "失控进化数据.h"
#include "TouchHelperA.h"

// from draw layer
extern int abs_ScreenX, abs_ScreenY;

#include <atomic>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <unistd.h>

namespace GameEsp {

inline constexpr const char* kPackage = "com.tencent.rmcn";

// 与原工程一致的坐标布局（X/Z/Y 字段顺序）
struct GameVec3 {
    float X = 0;
    float Z = 0;
    float Y = 0;
};

struct AimStruct {
    GameVec3 ObjAim{};
    GameVec3 MyObjAim{};
    float ScreenDistance = 0;
    float WodDistance = 0;
};

// 原 DrawIo / NumIo / wz 映射到我们的开关滑块
struct Settings {
    // DrawIo
    bool box = false;          // DrawIo[1] 人物方框
    bool line = false;         // DrawIo[2] 显示射线
    bool name = false;         // DrawIo[4] 人物昵称
    bool count = false;        // 玩家数量显示（灵动岛）
    bool bone = false;         // DrawIo[6] 火柴人骨骼
    bool aim = false;          // DrawIo[20] 开启自瞄
    bool aim_touch_pos = false;// DrawIo[21] 触摸位置
    bool aim_dynamic = false;  // DrawIo[25] 动态自瞄

    // NumIo
    float aim_fov = 240.f;     // NumIo[3]
    float aim_force = 13.f;    // NumIo[4] 力度（越小越快）
    float touch_x = 650.f;     // NumIo[5]
    float touch_y = 1500.f;    // NumIo[6]
    float touch_range = 160.f; // NumIo[7]
    float aim_speed = 7.8f;    // NumIo[9] usleep 毫秒

    int aim_part = 0;          // wz: 0头 1胸 2腿
};

struct Runtime {
    bool inited = false;
    bool attached = false;
    bool threads_started = false;
    int pid = -1;
    unsigned long libunity = 0;   // libbase1
    unsigned long libil2cpp = 0;  // libbase
    unsigned long uworld = 0;
    unsigned long matrix_addr = 0;
    unsigned long array_addr = 0;
    unsigned long self = 0;       // oneself

    float matrix[16] = {};
    int player_count = 0;
    int aim_count = 0;
    int max_player_count = 0;
    int gmin = -1;
    int firing = 0;
    int myteam = 0;
    int world_distance = 0;
    float zm_x = 0, zm_y = 0;

    AimStruct aims[100];
    Settings s;
    char status[160] = "未初始化";
    char player_name[100] = {};

    // 逻辑分辨率（横竖屏后）
    float screen_w = 0;
    float screen_h = 0;
};

inline Runtime& R() {
    static Runtime g;
    return g;
}

// 模块代码段基址（第 index 个映射）
inline unsigned long ModuleBaseIndex(int target_pid, const char* name, int index) {
    if (Mem().ready() && target_pid > 0) {
        pid_t old = Mem().pid;
        Mem().set_pid(target_pid);
        uintptr_t b = Mem().module_base(name);
        Mem().set_pid(old > 0 ? old : target_pid);
        if (b && index == 1) return (unsigned long)b;
    }
    return (unsigned long)MemBackend::maps_module_base(target_pid, name, index);
}

// libunity.so:bss[1] —— 新数据全部相对 BSS 基址
inline unsigned long ModuleBssIndex(int target_pid, const char* name, int module_index = 1, int bss_index = 1) {
    // 1) maps 可靠扫 [anon:.bss]
    uintptr_t b = MemBackend::maps_module_bss(target_pid, name, module_index, bss_index);
    if (b) return (unsigned long)b;
    // 2) TWT 驱动 bss
    if (Mem().ready() && Mem().kind == MemBackendKind::Twt && bss_index == 1) {
        pid_t old = Mem().pid;
        Mem().set_pid(target_pid);
        uintptr_t tb = Mem().twt.module_bss(name);
        Mem().set_pid(old > 0 ? old : target_pid);
        if (tb) return (unsigned long)tb;
    }
    // 3) 回退：模块基址（兼容旧逻辑，通常不正确，仅兜底）
    return ModuleBaseIndex(target_pid, name, module_index);
}

// 指针有效性
inline bool ValidPtr(unsigned long p) { return p > 0x1000UL; }

/*
 * 失控进化 新数据（相对 libunity.so:bss[1]）
 * 世界: +0x79be8 -> +0x668 -> +0x4c8 -> -0x3f8 -> +0x80 -> +0x70 -> +0x78
 * 矩阵: +0xABB80 -> +0x40 -> +0x10 再 +0x110（字段，不读指针）
 * 人物: +0x8cd40 -> +0x128 -> +0x4d0 -> +0x410 -> +0x5f0 -> +0x718 -> +0x10 -> +0x18 (+0x0)
 */
inline bool ResolveChains() {
    auto& r = R();
    r.uworld = 0;
    r.matrix_addr = 0;
    r.array_addr = 0;
    r.self = 0;

    const unsigned long bss = r.libunity; // 这里存的是 bss 基址
    if (!ValidPtr(bss)) return false;

    // -------- 1. 世界数组 Uworld --------
    {
        unsigned long a = getPtr64(bss + 0x79be8);
        if (ValidPtr(a)) a = getPtr64(a + 0x668);
        if (ValidPtr(a)) a = getPtr64(a + 0x4c8);
        if (ValidPtr(a)) a = getPtr64(a - 0x3f8);
        if (ValidPtr(a)) a = getPtr64(a + 0x80);
        if (ValidPtr(a)) a = getPtr64(a + 0x70);
        if (ValidPtr(a)) r.uworld = getPtr64(a + 0x78);
    }

    // -------- 2. 矩阵 Matrix（末级 +0x110 为结构内偏移）--------
    {
        unsigned long m = getPtr64(bss + 0xABB80);
        if (ValidPtr(m)) m = getPtr64(m + 0x40);
        if (ValidPtr(m)) m = getPtr64(m + 0x10);
        if (ValidPtr(m)) r.matrix_addr = m + 0x110;
    }

    // -------- 3. 人物数组 Arrayaddr (+0x0) --------
    {
        unsigned long p = getPtr64(bss + 0x8cd40);
        if (ValidPtr(p)) p = getPtr64(p + 0x128);
        if (ValidPtr(p)) p = getPtr64(p + 0x4d0);
        if (ValidPtr(p)) p = getPtr64(p + 0x410);
        if (ValidPtr(p)) p = getPtr64(p + 0x5f0);
        if (ValidPtr(p)) p = getPtr64(p + 0x718);
        if (ValidPtr(p)) p = getPtr64(p + 0x10);
        if (ValidPtr(p)) p = getPtr64(p + 0x18);
        // +0x0：最终指针即数组基
        if (ValidPtr(p)) r.array_addr = p;
    }

    // -------- 4. 自身 --------
    if (ValidPtr(r.uworld)) {
        r.self = getPtr64(r.uworld + 0x18);
        if (!ValidPtr(r.self)) r.self = getPtr64(r.uworld + 0x20);
        if (!ValidPtr(r.self)) r.self = getPtr64(r.uworld + 0x28);
        if (!ValidPtr(r.self)) r.self = getPtr64(r.uworld + 0x30);
        if (!ValidPtr(r.self)) r.self = getPtr64(r.uworld + 0x10);
    }
    if (!ValidPtr(r.self) && ValidPtr(r.array_addr))
        r.self = getPtr64(r.array_addr);

    return ValidPtr(r.uworld) && ValidPtr(r.matrix_addr);
}

inline void DrawBoldText(ImDrawList* dl, float size, float x, float y, ImU32 col, ImU32 outline, const char* str) {
    if (!dl || !str) return;
    dl->AddText(nullptr, size, ImVec2(x - 0.1f, y - 0.1f), outline, str);
    dl->AddText(nullptr, size, ImVec2(x + 0.1f, y + 0.1f), outline, str);
    dl->AddText(nullptr, size, ImVec2(x, y), col, str);
}

inline void DrawOutlineText(ImDrawList* dl, float size, float x, float y, ImU32 col, const char* str) {
    if (!dl || !str) return;
    ImU32 black = IM_COL32(0, 0, 0, 255);
    dl->AddText(nullptr, size, ImVec2(x + 1, y), black, str);
    dl->AddText(nullptr, size, ImVec2(x - 0.1f, y), black, str);
    dl->AddText(nullptr, size, ImVec2(x, y + 1), black, str);
    dl->AddText(nullptr, size, ImVec2(x, y - 1), black, str);
    dl->AddText(nullptr, size, ImVec2(x, y), col, str);
}

// 与原 findminat 一致
inline int FindMinAt() {
    auto& r = R();
    float minv = r.s.aim_fov;
    int minAt = 999;
    for (int i = 0; i < r.max_player_count; i++) {
        if (r.aims[i].ScreenDistance < minv && r.aims[i].ScreenDistance != 0) {
            minv = r.aims[i].ScreenDistance;
            minAt = i;
        }
    }
    if (minAt == 999) {
        r.gmin = -1;
        return -1;
    }
    r.gmin = minAt;
    r.world_distance = (int)r.aims[minAt].WodDistance;
    return minAt;
}


// 火柴人骨骼：原工程有 Bone 偏移与 getBoneXYZ 工具，但 DrawPlayer 未接完整骨骼链。
// 在已有世界坐标 + 屏幕框可用的前提下，用框比例绘制稳定火柴人（头/躯干/四肢）。
inline void DrawStickman(ImDrawList* dl, float left, float right, float top, float bottom) {
    if (!dl) return;
    float h = bottom - top;
    float w = right - left;
    if (h < 8.f || w < 4.f) return;

    float cx = (left + right) * 0.5f;
    float head_r = h * 0.085f;
    if (head_r < 3.f) head_r = 3.f;
    float head_cy = top + head_r * 1.15f;
    float neck_y = head_cy + head_r * 0.95f;
    float shoulder_y = neck_y + h * 0.04f;
    float hip_y = top + h * 0.55f;
    float arm_w = w * 0.42f;
    float leg_w = w * 0.28f;

    ImU32 col = IM_COL32(0, 255, 170, 230);
    ImU32 col2 = IM_COL32(255, 255, 255, 90);
    float tk = h * 0.018f;
    if (tk < 1.2f) tk = 1.2f;
    if (tk > 2.8f) tk = 2.8f;

    // 头
    dl->AddCircle(ImVec2(cx, head_cy), head_r, col, 24, tk);
    // 躯干
    dl->AddLine(ImVec2(cx, neck_y), ImVec2(cx, hip_y), col, tk);
    // 肩线
    dl->AddLine(ImVec2(cx - arm_w, shoulder_y), ImVec2(cx + arm_w, shoulder_y), col, tk * 0.9f);
    // 臂
    float hand_y = shoulder_y + h * 0.22f;
    dl->AddLine(ImVec2(cx - arm_w, shoulder_y), ImVec2(cx - arm_w * 1.05f, hand_y), col, tk);
    dl->AddLine(ImVec2(cx + arm_w, shoulder_y), ImVec2(cx + arm_w * 1.05f, hand_y), col, tk);
    // 髋
    dl->AddLine(ImVec2(cx - leg_w, hip_y), ImVec2(cx + leg_w, hip_y), col, tk * 0.9f);
    // 腿
    dl->AddLine(ImVec2(cx - leg_w, hip_y), ImVec2(cx - leg_w * 1.1f, bottom), col, tk);
    dl->AddLine(ImVec2(cx + leg_w, hip_y), ImVec2(cx + leg_w * 1.1f, bottom), col, tk);
    // 轻描边增强可读
    dl->AddCircle(ImVec2(cx, head_cy), head_r + 0.8f, col2, 24, 1.0f);
}


// 触摸热区可视化 + 主线程也可读坐标（不依赖游戏初始化）
inline void DrawTouchRegion(float screen_w, float screen_h) {
    auto& r = R();
    if (!r.s.aim_touch_pos) return;
    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    if (!dl) return;
    float sw = screen_w > 1.f ? screen_w : (float)abs_ScreenX;
    float sh = screen_h > 1.f ? screen_h : (float)abs_ScreenY;
    if (sw < 1.f || sh < 1.f) return;

    // 半透明遮罩，提示正在校准触摸区
    dl->AddRectFilled(ImVec2(0, 0), ImVec2(sw, sh), IM_COL32(0, 0, 0, 100));

    // 与原版一致：屏幕显示坐标 (touch_y, sh - touch_x)
    float nx = r.s.touch_y;
    float ny = sh - r.s.touch_x;
    float hr = r.s.touch_range * 0.5f;
    if (hr < 8.f) hr = 8.f;

    // 外框 + 填充
    dl->AddRectFilled(ImVec2(nx - hr, ny - hr), ImVec2(nx + hr, ny + hr), IM_COL32(255, 40, 40, 110), 8.f);
    dl->AddRect(ImVec2(nx - hr, ny - hr), ImVec2(nx + hr, ny + hr), IM_COL32(255, 80, 80, 230), 8.f, 0, 2.5f);
    // 十字
    dl->AddLine(ImVec2(nx - hr, ny), ImVec2(nx + hr, ny), IM_COL32(255, 255, 255, 180), 1.5f);
    dl->AddLine(ImVec2(nx, ny - hr), ImVec2(nx, ny + hr), IM_COL32(255, 255, 255, 180), 1.5f);
    dl->AddCircleFilled(ImVec2(nx, ny), 5.f, IM_COL32(255, 255, 255, 230), 16);

    const char* tip = "长按拖动 · 校准自瞄触摸区";
    ImVec2 ts = ImGui::CalcTextSize(tip);
    float fs = 28.f;
    dl->AddText(nullptr, fs, ImVec2(nx - ts.x * 0.5f, ny - hr - fs - 8.f), IM_COL32(255, 255, 255, 255), tip);

    char coord[96];
    snprintf(coord, sizeof(coord), "x=%.0f  y=%.0f  r=%.0f", r.s.touch_x, r.s.touch_y, r.s.touch_range);
    ImVec2 cs = ImGui::CalcTextSize(coord);
    dl->AddText(nullptr, 22.f, ImVec2(nx - cs.x * 0.5f, ny + hr + 8.f), IM_COL32(255, 220, 220, 255), coord);
}

// 主线程拖动校准（比独立线程更稳，ImGui IO 同帧）
inline void UpdateTouchRegionDrag(float screen_w, float screen_h) {
    auto& r = R();
    if (!r.s.aim_touch_pos) return;
    ImGuiIO& io = ImGui::GetIO();
    float sw = screen_w > 1.f ? screen_w : (float)abs_ScreenX;
    float sh = screen_h > 1.f ? screen_h : (float)abs_ScreenY;
    if (sw < 1.f || sh < 1.f) return;

    float nx = r.s.touch_y;
    float ny = sh - r.s.touch_x;
    float hr = r.s.touch_range * 0.5f;
    if (hr < 24.f) hr = 24.f; // 命中放宽

    static bool dragging = false;
    static float hold_t = 0.f;
    float mx = io.MousePos.x;
    float my = io.MousePos.y;
    bool inside = (mx >= nx - hr && mx <= nx + hr && my >= ny - hr && my <= ny + hr);

    if (io.MouseDown[0] && inside) {
        hold_t += io.DeltaTime > 0.f ? io.DeltaTime : (1.f / 60.f);
        if (hold_t > 0.12f) dragging = true; // 短按确认，避免误触
    } else if (!io.MouseDown[0]) {
        hold_t = 0.f;
        dragging = false;
    }
    if (dragging && io.MouseDown[0]) {
        // 反变换写回 touch_x/touch_y
        r.s.touch_y = mx;
        r.s.touch_x = sh - my;
        // 边界钳制
        if (r.s.touch_y < hr) r.s.touch_y = hr;
        if (r.s.touch_y > sw - hr) r.s.touch_y = sw - hr;
        if (r.s.touch_x < hr) r.s.touch_x = hr;
        if (r.s.touch_x > sh - hr) r.s.touch_x = sh - hr;
    }
}

// ---------- 原 DrawPlayer 逻辑（读写=驱动） ----------
inline void DrawPlayer() {
    auto& r = R();
    if (!MemDriver_Ready() || !r.inited) return;

    float screen_x = r.screen_w > 1.f ? r.screen_w : (float)abs_ScreenX;
    float screen_y = r.screen_h > 1.f ? r.screen_h : (float)abs_ScreenY;
    if (screen_x < 1.f || screen_y < 1.f) return;

    py = screen_y / 2.f;
    px = screen_x / 2.f;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    if (!dl) return;

    // 自瞄圈 DrawIo[20]
    if (r.s.aim) {
        dl->AddCircle(ImVec2(px, py), r.s.aim_fov, IM_COL32(255, 255, 0, 255), 0, 2.5f);
    }

    // 触摸区改由 DrawTouchRegion 统一绘制（不依赖 inited）

    memset(r.matrix, 0, sizeof(r.matrix));
    vm_readv(r.matrix_addr, r.matrix, 16 * 4);
    // 同步到全局 matrix，供自瞄 WorldToScreen 使用
    memcpy(::matrix, r.matrix, sizeof(r.matrix));

    int count = getDword(r.uworld + 0x2C);
    if (count <= 0 || count > 1000) count = getDword(r.uworld + 0x10);
    if (count <= 0 || count > 1000) count = getDword(r.uworld + 0x1C);
    if (count <= 0 || count > 1000) count = 0;

    r.player_count = 0;
    r.aim_count = 0;

    for (int i = 0; i < count; i++) {
        unsigned long Objaddr = getPtr64(r.array_addr + 8ULL * (unsigned long)i);
        if (Objaddr == 0 || Objaddr < 0x1000) continue;

        r.myteam = getDword(getPtr64(r.self + 0x80) + 0x14);
        int team = getDword(getPtr64(Objaddr + 0x80) + 0x14);

        int Health = getDword(Objaddr + 0x18);
        if (Health <= 0 || Health > 1000) continue;
        if (team == r.myteam) continue;

        GameVec3 Z{};
        vm_readv(r.self + 0xa0, &Z, sizeof(Z));

        GameVec3 D{};
        vm_readv(Objaddr + 0xa0, &D, sizeof(D));
        if (D.X == 0 && D.Y == 0 && D.Z == 0) continue;
        D.Z = D.Z + 0.5f;

        float camera = r.matrix[3] * D.X + r.matrix[7] * D.Z + r.matrix[11] * D.Y + r.matrix[15];
        if (camera <= 0.01f) continue;

        float r_x = px + (r.matrix[0] * D.X + r.matrix[4] * D.Z + r.matrix[8] * D.Y + r.matrix[12]) / camera * px;
        float r_y = py - (r.matrix[1] * D.X + r.matrix[5] * (D.Z - 0.3f) + r.matrix[9] * D.Y + r.matrix[13]) / camera * py;
        float r_w = py - (r.matrix[1] * D.X + r.matrix[5] * (D.Z + 1.8f) + r.matrix[9] * D.Y + r.matrix[13]) / camera * py;

        float X = r_x - (r_y - r_w) / 4.f;
        float Y = r_y;
        float W = (r_y - r_w) / 3.f;

        float x1 = X + W / 2.f;
        float top = Y - W;
        float bottom = Y + W;
        float left = (X + W / 2.f) - W / 2.6f;
        float right = X + W / 1.12f;

        float Distance = sqrtf(powf(D.X - Z.X, 2) + powf(D.Z - Z.Z, 2) + powf(D.Y - Z.Y, 2));

        memset(r.player_name, 0, sizeof(r.player_name));
        getUTF8((UTF8*)r.player_name, getPtr64(Objaddr + 0x28) + 0x74);

        // 自瞄数组
        if (W > 0 && r.s.aim && r.aim_count < 100) {
            r.aims[r.aim_count].ObjAim = D;
            // 原工程 pow(..., 1) 疑似笔误，保留原写法行为：|py-(Y-W/1.5)|
            r.aims[r.aim_count].ScreenDistance = sqrtf(powf(px - r_x, 2) + powf(py - (Y - W / 1.5f), 1.f));
            r.aims[r.aim_count].WodDistance = Distance;
            r.aim_count++;
        }

        if (W > 0 && r.s.box) {
            dl->AddRect(ImVec2(left, top), ImVec2(right, bottom), IM_COL32(255, 255, 255, 255), 12.f, 0, 0.5f);
        }
        if (W > 0 && r.s.bone) {
            DrawStickman(dl, left, right, top, bottom);
        }
        if (W > 0 && r.s.line) {
            dl->AddLine(ImVec2(px, 118), ImVec2(left - 5, top - 61), IM_COL32(255, 255, 255, 255), 0.1f);
        }
        if (W > 0 && r.s.name) {
            char mz[64];
            snprintf(mz, sizeof(mz), "%s", r.player_name[0] ? r.player_name : "Player");
            ImVec2 textSize = ImGui::CalcTextSize(mz);
            DrawOutlineText(dl, 25.f, x1 - (textSize.x / 2.f) + 17.f, Y - W - 80.f, IM_COL32(255, 255, 255, 245), mz);

            char sm[32];
            snprintf(sm, sizeof(sm), "%dm", (int)Distance);
            ImVec2 textSize1 = ImGui::CalcTextSize(sm);
            DrawOutlineText(dl, 22.5f, x1 - (textSize1.x / 2.f), bottom, IM_COL32(248, 248, 255, 255), sm);
        }

        r.max_player_count = r.aim_count;
        r.player_count++;
    }

    // 玩家数量改由灵动岛显示（s.count），不在屏幕正中画数字

}

// ---------- 原 AimBotAuto（触摸用 Touch::） ----------
inline void AimBotAutoLoop() {
    auto& r = R();
    bool isDown = false;
    double tx = r.s.touch_x, ty = r.s.touch_y;
    static float TargetX = 0;
    static float TargetY = 0;

    while (true) {
        float ScreenX = r.screen_w > 1.f ? r.screen_w : (float)abs_ScreenX;
        float ScreenY = r.screen_h > 1.f ? r.screen_h : (float)abs_ScreenY;
        double ScrXH = ScreenX / 2.0;
        double ScrYH = ScreenY / 2.0;

        if (!r.s.aim) {
            if (isDown) {
                tx = r.s.touch_x; ty = r.s.touch_y;
                Touch::Up();
                isDown = false;
            }
            usleep((useconds_t)(r.s.aim_speed * 1000));
            continue;
        }

        FindMinAt();

        // 开火状态：原 oneself+0x20 -> +0x208
        r.firing = 0;
        if (r.self > 0x1000) {
            r.firing = getDword(getPtr64(r.self + 0x20) + 0x208);
        }

        float ToReticleDistance = (r.gmin >= 0) ? r.aims[r.gmin].ScreenDistance : 99999.f;

        if (r.gmin == -1) {
            if (isDown) { tx = r.s.touch_x; ty = r.s.touch_y; Touch::Up(); isDown = false; }
            usleep((useconds_t)(r.s.aim_speed * 1000));
            continue;
        }

        GameVec3 obj{};
        obj.X = r.aims[r.gmin].ObjAim.X;
        if (r.s.aim_part == 0) obj.Y = r.aims[r.gmin].ObjAim.Z + 0.35f;
        else if (r.s.aim_part == 1) obj.Y = r.aims[r.gmin].ObjAim.Z + 0.15f;
        else obj.Y = r.aims[r.gmin].ObjAim.Z - 0.3f;
        obj.Z = r.aims[r.gmin].ObjAim.Y;

        // 用当前全局 matrix（DrawPlayer 每帧刷新）
        float cameras = ::matrix[3] * obj.X + ::matrix[7] * obj.Y + ::matrix[11] * obj.Z + ::matrix[15];
        Vector3A wobj(obj.X, obj.Y, obj.Z);
        Vector2A vpvp = WorldToScreen(wobj, ::matrix, cameras);

        r.zm_y = vpvp.X;
        r.zm_x = ScreenX - vpvp.Y;

        if (r.zm_x == 0 && r.zm_y == 0) {
            if (isDown) { tx = r.s.touch_x; ty = r.s.touch_y; Touch::Up(); isDown = false; }
            usleep((useconds_t)(r.s.aim_speed * 1000));
            continue;
        }
        if (ToReticleDistance > r.s.aim_fov) {
            if (isDown) { tx = r.s.touch_x; ty = r.s.touch_y; Touch::Up(); isDown = false; }
            usleep((useconds_t)(r.s.aim_speed * 1000));
            continue;
        }
        if (r.zm_x <= 0 || r.zm_x >= ScreenX || r.zm_y <= 0 || r.zm_y >= ScreenY) {
            if (isDown) { tx = r.s.touch_x; ty = r.s.touch_y; Touch::Up(); isDown = false; }
            usleep((useconds_t)(r.s.aim_speed * 1000));
            continue;
        }

        if ((r.firing == 1) && r.aim_count > 0) {
            if (!isDown) {
                Touch::Down((float)tx, (float)ty);
                isDown = true;
            }
            float Aimspeace = r.s.aim_force;
            if (r.zm_x > ScrXH) {
                TargetX = -(float)(ScrXH - r.zm_x);
                TargetX /= Aimspeace;
                if (TargetX + ScrXH > ScrXH * 2) TargetX = 0;
            }
            if (r.zm_x < ScrXH) {
                TargetX = (float)(r.zm_x - ScrXH);
                TargetX /= Aimspeace;
                if (TargetX + ScrXH < 0) TargetX = 0;
            }
            if (r.zm_y > ScrYH) {
                TargetY = -(float)(ScrYH - r.zm_y);
                TargetY /= Aimspeace;
                if (TargetY + ScrYH > ScrYH * 2) TargetY = 0;
            }
            if (r.zm_y < ScrYH) {
                TargetY = (float)(r.zm_y - ScrYH);
                TargetY /= Aimspeace;
                if (TargetY + ScrYH < 0) TargetY = 0;
            }

            if (TargetY >= 35 || TargetX >= 35 || TargetY <= -35 || TargetX <= -35) {
                if (isDown) { tx = r.s.touch_x; ty = r.s.touch_y; Touch::Up(); isDown = false; }
                usleep((useconds_t)(r.s.aim_speed * 1000));
                continue;
            }

            tx += TargetX;
            ty += TargetY;

            if (tx >= r.s.touch_x + r.s.touch_range || tx <= r.s.touch_x - r.s.touch_range ||
                ty >= r.s.touch_y + r.s.touch_range || ty <= r.s.touch_y - r.s.touch_range) {
                tx = r.s.touch_x; ty = r.s.touch_y;
                Touch::Up();
                Touch::Down((float)tx, (float)ty);
            }
            Touch::Move((float)tx, (float)ty);
        } else {
            if (isDown) { tx = r.s.touch_x; ty = r.s.touch_y; Touch::Up(); isDown = false; }
            usleep((useconds_t)(r.s.aim_speed * 1000));
            continue;
        }

        usleep((useconds_t)(r.s.aim_speed * 1000));
    }
}

// ---------- 原 GetTouch：长按拖动自瞄触摸点 ----------
inline void GetTouchLoop() {
    auto& r = R();
    while (true) {
        usleep(2000);
        ImGuiIO& io = ImGui::GetIO();
        float screen_y = r.screen_h > 1.f ? r.screen_h : (float)abs_ScreenY;
        float py2 = screen_y; // 原 py*2 且 py=screen_y/2 => screen_y

        float mx = io.MousePos.x;
        float my = io.MousePos.y;
        if (r.s.aim_touch_pos && io.MouseDown[0] &&
            mx <= r.s.touch_y + r.s.touch_range &&
            my <= py2 - r.s.touch_x + r.s.touch_range &&
            mx >= r.s.touch_y - r.s.touch_range &&
            my >= py2 - r.s.touch_x - r.s.touch_range) {
            usleep(55000);
            if (io.MouseDown[0] &&
                mx <= r.s.touch_y + r.s.touch_range &&
                my <= py2 - r.s.touch_x + r.s.touch_range &&
                mx >= r.s.touch_y - r.s.touch_range &&
                my >= py2 - r.s.touch_x - r.s.touch_range) {
                while (io.MouseDown[0]) {
                    r.s.touch_y = io.MousePos.x;
                    r.s.touch_x = py2 - io.MousePos.y;
                    usleep(500);
                }
            }
        }
    }
}

inline void StartThreads() {
    auto& r = R();
    if (r.threads_started) return;
    r.threads_started = true;
    std::thread(AimBotAutoLoop).detach();
    std::thread(GetTouchLoop).detach();
}

// 手动初始化：每次点击都重新找进程/解析链（允许错误时机后再次点）
inline bool InitAfterDriver() {
    auto& r = R();
    r.inited = false;
    r.attached = false;
    if (!MemDriver_Ready()) {
        snprintf(r.status, sizeof(r.status), "驱动未就绪");
        return false;
    }
    int id = getProcessID(kPackage);
    if (id <= 0 && Mem().kind == MemBackendKind::Twt)
        id = (int)Mem().twt.name_to_pid(kPackage);
    if (id <= 0) {
        snprintf(r.status, sizeof(r.status), "未找到进程 %s", kPackage);
        r.attached = false;
        r.inited = false;
        return false;
    }
    if (!MemDriver_Attach(id)) {
        snprintf(r.status, sizeof(r.status), "绑定失败 pid=%d", id);
        return false;
    }
    r.pid = id;
    // 同步全局 pid，供 getbss/maps 使用
    ::pid = id;

    // 代码段基址（备用）
    unsigned long unity_rx = ModuleBaseIndex(id, "libunity.so", 1);
    r.libil2cpp = ModuleBaseIndex(id, "libil2cpp.so", 1);

    // 新数据要求：libunity.so:bss[1]
    r.libunity = ModuleBssIndex(id, "libunity.so", 1, 1);
    if (r.libunity < 0x1000) {
        snprintf(r.status, sizeof(r.status), "libunity bss[1] 失败 (rx=%lX)", (unsigned long)unity_rx);
        return false;
    }
    if (!ResolveChains()) {
        snprintf(r.status, sizeof(r.status),
                 "链失败 bss=%lX U=%lX M=%lX A=%lX",
                 (unsigned long)r.libunity,
                 (unsigned long)r.uworld,
                 (unsigned long)r.matrix_addr,
                 (unsigned long)r.array_addr);
        r.inited = false;
        return false;
    }
    r.inited = true;
    r.attached = true;
    // 状态显示关键地址，便于核对数据是否接上
    snprintf(r.status, sizeof(r.status),
             "就绪 · %s · pid=%d | bss=%lX U=%lX M=%lX A=%lX S=%lX",
             MemDriver_Name(), id,
             (unsigned long)r.libunity,
             (unsigned long)r.uworld,
             (unsigned long)r.matrix_addr,
             (unsigned long)r.array_addr,
             (unsigned long)r.self);
    StartThreads();
    return true;
}

// 每帧入口：更新分辨率 + DrawPlayer
inline void TickDraw(float screen_w, float screen_h) {
    auto& r = R();
    r.screen_w = screen_w;
    r.screen_h = screen_h;

    // 触摸区校准：不依赖驱动/初始化，开关打开即可显示与拖动
    UpdateTouchRegionDrag(screen_w, screen_h);
    DrawTouchRegion(screen_w, screen_h);

    // 游戏绘制仅在用户手动「初始化」成功后
    if (!MemDriver_Ready() || !r.inited) return;
    DrawPlayer();
}

inline void SetAllDraw(bool on) {
    auto& s = R().s;
    s.box = s.line = s.name = s.count = s.bone = on;
}

} // namespace GameEsp
