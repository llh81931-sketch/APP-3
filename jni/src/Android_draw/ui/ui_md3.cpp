#include "ui/ui_md3.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

namespace Md3 {

static float g_dt = 1.f / 60.f;
static Metrics g_m;
const Metrics& M() { return g_m; }

static float Clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
static float FMax(float a, float b) { return a > b ? a : b; }
static float FMin(float a, float b) { return a < b ? a : b; }
static float Lerpf(float a, float b, float t) { return a + (b - a) * t; }
static float Smooth01(float t) { t = Clampf(t, 0.f, 1.f); return t * t * (3.f - 2.f * t); }
static float Approach(float cur, float target, float speed) {
    return Lerpf(cur, target, 1.f - std::exp(-speed * g_dt));
}
static bool Hovered(const ImVec2& a, const ImVec2& b) {
    const ImVec2 p = ImGui::GetIO().MousePos;
    return p.x >= a.x && p.x <= b.x && p.y >= a.y && p.y <= b.y;
}
static bool Hit(const ImVec2& a, const ImVec2& b, const char* id) {
    ImGui::SetCursorScreenPos(a);
    ImGui::InvisibleButton(id, ImVec2(FMax(1.f, b.x - a.x), FMax(1.f, b.y - a.y)));
    return ImGui::IsItemClicked(ImGuiMouseButton_Left);
}

struct SwA { float t = 0.f; float bounce = 0.f; float press = 0.f; };
struct BtnA {
    float press = 0.f;
    float hover = 0.f;
    float ripple = 0.f;   // 0..1 点击波纹
    float flash = 0.f;    // 点击高光
    float rx = 0.f, ry = 0.f; // 波纹中心（相对按钮）
};
struct TabA {
    float sel = 0.f;   // 选中强度
    float hover = 0.f;
    float press = 0.f;
    float bounce = 0.f;
};
struct DropA {
    float open = 0.f; bool opened = false; bool open_up = false;
    float press = 0.f; float hover = 0.f;
    int* selected = nullptr; const char* const* items = nullptr; int count = 0;
    ImVec2 hmin{}, hmax{}, lmin{}, lmax{};
};
struct SldA { float vis = 0.f; float th = 1.f; bool active = false; }; // 滑块保持原样，不加额外动效

static std::unordered_map<std::string, SwA> g_sw;
static std::unordered_map<std::string, BtnA> g_btn;
static std::unordered_map<std::string, DropA> g_drop;
static std::unordered_map<std::string, SldA> g_sld;
static std::unordered_map<std::string, TabA> g_tab;
static std::vector<std::string> g_drop_ids;
static std::unordered_map<std::string, bool> g_sb_drag;
static std::unordered_map<std::string, float> g_sb_off;

void BeginFrame(float dt) {
    g_dt = dt > 0.f ? (dt < 0.05f ? dt : 0.05f) : (1.f / 60.f);
    g_drop_ids.clear();
}

void Text(ImDrawList* dl, ImFont* font, float size, const ImVec2& pos, const ImVec4& col, const char* text) {
    if (!font || !text) return;
    dl->AddText(font, size, pos, UiCol(col), text);
}
void Divider(ImDrawList* dl, float x0, float x1, float y, float alpha) {
    dl->AddLine(ImVec2(x0, y), ImVec2(x1, y), UiCol(UiTheme_Current().divider, alpha), 2.f);
}
void Card(ImDrawList* dl, const ImVec2& min, const ImVec2& max, float rounding) {
    dl->AddRectFilled(min, max, UiCol(UiTheme_Current().surface_variant), rounding);
}
void ListRow(ImDrawList* dl, const ImVec2& min, const ImVec2& max, ImFont* font, const char* title, const char* subtitle) {
    const UiTheme& t = UiTheme_Current();
    if (!font || !title) return;
    const float h = max.y - min.y;
    const float pad = 24.f;
    dl->AddText(font, g_m.body_fs, ImVec2(min.x + pad, min.y + h * (subtitle ? 0.18f : 0.30f)), UiCol(t.on_surface), title);
    if (subtitle)
        dl->AddText(font, g_m.hint_fs, ImVec2(min.x + pad, min.y + h * 0.55f), UiCol(t.on_surface_var), subtitle);
}

bool Button(ImDrawList* dl, const char* id, const char* label, const ImVec2& min, const ImVec2& max, ImFont* font, bool filled) {
    const UiTheme& t = UiTheme_Current();
    BtnA& an = g_btn[id];
    const bool hov = Hovered(min, max);
    ImGui::SetCursorScreenPos(min);
    ImGui::InvisibleButton(id, ImVec2(FMax(1.f, max.x - min.x), FMax(1.f, max.y - min.y)));
    const bool held = ImGui::IsItemActive();
    const bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);

    an.hover = Approach(an.hover, (hov || held) ? 1.f : 0.f, 16.f);
    an.press = Approach(an.press, held ? 1.f : 0.f, 28.f);
    if (clicked) {
        an.ripple = 0.001f;
        an.flash = 1.f;
        ImVec2 mp = ImGui::GetIO().MousePos;
        an.rx = Clampf((mp.x - min.x) / FMax(1.f, max.x - min.x), 0.f, 1.f);
        an.ry = Clampf((mp.y - min.y) / FMax(1.f, max.y - min.y), 0.f, 1.f);
    }
    if (an.ripple > 0.f && an.ripple < 1.f)
        an.ripple = Clampf(an.ripple + g_dt * 3.6f, 0.f, 1.f);
    else if (an.ripple >= 1.f)
        an.ripple = 0.f;
    an.flash = Approach(an.flash, 0.f, 10.f);

    // 悬停微胀 + 按下收缩
    const float scale = 1.f + an.hover * 0.025f - an.press * 0.055f;
    const ImVec2 c((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
    const float hw = (max.x - min.x) * 0.5f * scale;
    const float hh = (max.y - min.y) * 0.5f * scale;
    ImVec2 a(c.x - hw, c.y - hh), b(c.x + hw, c.y + hh);
    const float round = (b.y - a.y) * 0.5f;
    const float fs = g_m.body_fs;

    // 悬停外发光
    if (an.hover > 0.02f) {
        float pad = 3.f + an.hover * 6.f;
        dl->AddRectFilled(ImVec2(a.x - pad, a.y - pad), ImVec2(b.x + pad, b.y + pad),
                          UiCol(t.primary, 0.10f * an.hover), round + pad);
    }

    if (filled) {
        dl->AddRectFilled(a, b, UiCol(t.primary), round);
        // 悬停/按下叠加
        float ov = 0.06f * an.hover + 0.12f * an.press + 0.18f * an.flash;
        if (ov > 0.001f) dl->AddRectFilled(a, b, UiCol(ImVec4(1, 1, 1, ov)), round);
        // 点击波纹
        if (an.ripple > 0.f) {
            float rr = an.ripple * (max.x - min.x) * 0.85f;
            ImVec2 rc(Lerpf(min.x, max.x, an.rx), Lerpf(min.y, max.y, an.ry));
            float ra = (1.f - an.ripple) * 0.28f;
            dl->PushClipRect(a, b, true);
            dl->AddCircleFilled(rc, rr, UiCol(ImVec4(1, 1, 1, ra)), 48);
            dl->PopClipRect();
        }
        if (font && label) {
            ImVec2 ts = font->CalcTextSizeA(fs, FLT_MAX, 0.f, label);
            dl->AddText(font, fs, ImVec2(c.x - ts.x * 0.5f, c.y - ts.y * 0.5f), UiCol(t.on_primary), label);
        }
    } else {
        dl->AddRect(a, b, UiCol(t.outline), round, 0, 2.5f + an.hover * 0.8f);
        float ov = 0.05f * an.hover + 0.10f * an.press + 0.14f * an.flash;
        if (ov > 0.001f) dl->AddRectFilled(a, b, UiCol(t.primary, ov), round);
        if (an.ripple > 0.f) {
            float rr = an.ripple * (max.x - min.x) * 0.8f;
            ImVec2 rc(Lerpf(min.x, max.x, an.rx), Lerpf(min.y, max.y, an.ry));
            dl->PushClipRect(a, b, true);
            dl->AddCircleFilled(rc, rr, UiCol(t.primary, (1.f - an.ripple) * 0.22f), 48);
            dl->PopClipRect();
        }
        if (font && label) {
            ImVec2 ts = font->CalcTextSizeA(fs, FLT_MAX, 0.f, label);
            dl->AddText(font, fs, ImVec2(c.x - ts.x * 0.5f, c.y - ts.y * 0.5f), UiCol(t.on_surface), label);
        }
    }
    return clicked;
}

bool Switch(ImDrawList* dl, const char* id, bool* value, const ImVec2& pos, float height) {
    if (!value) return false;
    const UiTheme& t = UiTheme_Current();
    if (height <= 1.f) height = g_m.switch_h;
    const float w = g_m.switch_w;
    ImVec2 tmin = pos, tmax(pos.x + w, pos.y + height);
    const float round = height * 0.5f;

    SwA& an = g_sw[id];
    const float prev = an.t;
    an.t = Approach(an.t, *value ? 1.f : 0.f, 16.f);
    // 切换越过中点时弹一下
    if ((prev < 0.5f && an.t >= 0.5f) || (prev > 0.5f && an.t <= 0.5f))
        an.bounce = 1.f;
    an.bounce = Approach(an.bounce, 0.f, 12.f);

    const bool hov = Hovered(ImVec2(tmin.x - 8, tmin.y - 10), ImVec2(tmax.x + 8, tmax.y + 10));
    an.press = Approach(an.press, hov && ImGui::GetIO().MouseDown[0] ? 1.f : 0.f, 22.f);
    const float tt = Smooth01(an.t);

    // 轨道色插值 + 悬停微亮
    ImVec4 track;
    track.x = Lerpf(t.switch_track_off.x, t.switch_track_on.x, tt);
    track.y = Lerpf(t.switch_track_off.y, t.switch_track_on.y, tt);
    track.z = Lerpf(t.switch_track_off.z, t.switch_track_on.z, tt);
    track.w = 1.f;
    // 轻微缩放反馈
    float sc = 1.f - an.press * 0.04f + an.bounce * 0.04f;
    ImVec2 c((tmin.x + tmax.x) * 0.5f, (tmin.y + tmax.y) * 0.5f);
    float hw = (tmax.x - tmin.x) * 0.5f * sc;
    float hh = (tmax.y - tmin.y) * 0.5f * sc;
    ImVec2 a(c.x - hw, c.y - hh), b(c.x + hw, c.y + hh);
    float rr = (b.y - a.y) * 0.5f;

    if (hov || an.bounce > 0.05f) {
        float glow = 0.08f * (hov ? 1.f : 0.f) + 0.14f * an.bounce;
        dl->AddRectFilled(ImVec2(a.x - 4, a.y - 4), ImVec2(b.x + 4, b.y + 4), UiCol(track, glow), rr + 4);
    }
    dl->AddRectFilled(a, b, UiCol(track), rr);
    if (tt < 0.99f)
        dl->AddRect(a, b, UiCol(t.outline, (1.f - tt) * 0.65f), rr, 0, 2.5f);

    const float pad = height * 0.12f;
    const float r_off = (height - pad * 2.f) * 0.36f;
    const float r_on  = (height - pad * 2.f) * 0.50f;
    float r = Lerpf(r_off, r_on, tt);
    r *= (1.f + an.bounce * 0.18f - an.press * 0.06f);
    const float x0 = a.x + pad + r_on;
    const float x1 = b.x - pad - r_on;
    const ImVec2 tc(Lerpf(x0, x1, tt), c.y);

    dl->AddCircleFilled(ImVec2(tc.x, tc.y + 2.f), r + 1.f, UiCol(ImVec4(0, 0, 0, 0.20f)), 32);
    ImVec4 thumb;
    thumb.x = Lerpf(t.switch_thumb_off.x, t.switch_thumb.x, tt);
    thumb.y = Lerpf(t.switch_thumb_off.y, t.switch_thumb.y, tt);
    thumb.z = Lerpf(t.switch_thumb_off.z, t.switch_thumb.z, tt);
    thumb.w = 1.f;
    dl->AddCircleFilled(tc, r, UiCol(thumb), 32);

    if (Hit(ImVec2(tmin.x - 12, tmin.y - 14), ImVec2(tmax.x + 12, tmax.y + 14), id)) {
        *value = !*value;
        an.bounce = 1.f;
        return true;
    }
    return false;
}

// MD3 Expressive / 新版 Slider：
// - 整条粗轨道，圆角胶囊（两端半圆）
// - 左侧 active 色更深/主色，右侧 inactive 浅色
// - 拇指是竖向圆角矩形（竖条），不是圆点
bool Slider(ImDrawList* dl, const char* id, float* value, float v_min, float v_max,
            const ImVec2& track_min, const ImVec2& track_max, ImFont* font, bool show_value) {
    if (!value) return false;
    const UiTheme& t = UiTheme_Current();
    SldA& an = g_sld[id];

    const float area_h = track_max.y - track_min.y;
    const float track_h = Clampf(area_h * 0.55f, 36.f, 52.f);
    const float cy = (track_min.y + track_max.y) * 0.5f;
    // 拇指宽 + 半宽边距，避免起点圆点贴边
    const float handle_w0 = Clampf(track_h * 0.24f, 12.f, 18.f);
    const float pad_x = handle_w0 * 0.5f + 4.f;
    const float x0 = track_min.x + pad_x;
    const float x1 = track_max.x - pad_x;
    const float span = FMax(1.f, x1 - x0);
    const float ty0 = cy - track_h * 0.5f;
    const float ty1 = cy + track_h * 0.5f;
    const float track_round = track_h * 0.5f;

    // 拇指：高于轨道，穿出上下
    const float handle_w = handle_w0;
    const float handle_h = track_h * 1.48f;
    const float handle_round = handle_w * 0.5f;

    float t01 = Clampf((*value - v_min) / (v_max - v_min + 1e-6f), 0.f, 1.f);
    if (!an.active) an.vis = Approach(an.vis, t01, 22.f);
    else an.vis = t01;

    ImGui::SetCursorScreenPos(track_min);
    ImGui::InvisibleButton(id, ImVec2(FMax(1.f, track_max.x - track_min.x), FMax(1.f, track_max.y - track_min.y)));
    const bool hov = Hovered(track_min, track_max) || ImGui::IsItemHovered();
    bool changed = false;
    if (ImGui::IsItemActive()) {
        an.active = true;
        an.th = Approach(an.th, 1.08f, 18.f);
        float nt = Clampf((ImGui::GetIO().MousePos.x - x0) / span, 0.f, 1.f);
        float nv = v_min + nt * (v_max - v_min);
        if (std::fabs(nv - *value) > 1e-4f) { *value = nv; changed = true; }
        an.vis = nt;
    } else {
        an.active = false;
        an.th = Approach(an.th, hov ? 1.04f : 1.f, 16.f);
    }

    const float act_x = x0 + an.vis * span;
    const float hw = handle_w * an.th;
    const float hh = handle_h * an.th;
    const float hl = act_x - hw * 0.5f; // 拇指左
    const float hr = act_x + hw * 0.5f; // 拇指右

    // 两段轨道：与拇指左右边严丝合缝（外端圆角、贴拇指端直角）
    // 左 active
    if (hl > x0 + 0.5f) {
        float lx1 = FMax(x0, hl);
        dl->AddRectFilled(ImVec2(x0, ty0), ImVec2(lx1, ty1), UiCol(t.slider_active),
                          track_round, ImDrawFlags_RoundCornersLeft);
    }
    // 右 inactive
    if (hr < x1 - 0.5f) {
        float rx0 = FMin(x1, hr);
        dl->AddRectFilled(ImVec2(rx0, ty0), ImVec2(x1, ty1), UiCol(t.slider_track),
                          track_round, ImDrawFlags_RoundCornersRight);
    }

    // 阶段点：仅中间刻度，不含两端起点/终点
    const int mid_stops = 3;
    for (int i = 1; i <= mid_stops; ++i) {
        float st = (float)i / (float)(mid_stops + 1); // 0.25 0.5 0.75
        float sx = x0 + st * span;
        if (std::fabs(sx - act_x) <= hw * 0.85f) continue;
        bool passed = st < an.vis;
        float dr = track_h * 0.11f;
        dl->AddCircleFilled(ImVec2(sx, cy), dr,
            UiCol(passed ? t.on_primary : t.on_surface_var, passed ? 0.50f : 0.32f), 16);
    }

    // 竖条拇指（盖住接缝）
    ImVec2 hmin(hl, cy - hh * 0.5f);
    ImVec2 hmax(hr, cy + hh * 0.5f);
    dl->AddRectFilled(ImVec2(hmin.x + 1.f, hmin.y + 2.f), ImVec2(hmax.x + 1.f, hmax.y + 2.f),
                      UiCol(ImVec4(0, 0, 0, 0.16f)), handle_round);
    dl->AddRectFilled(hmin, hmax, UiCol(t.slider_thumb), handle_round);

    if (show_value && font && (an.active || hov)) {
        char buf[24];
        std::snprintf(buf, sizeof(buf), "%.0f", *value);
        const float fs = g_m.hint_fs;
        ImVec2 ts = font->CalcTextSizeA(fs, FLT_MAX, 0.f, buf);
        ImVec2 p(act_x - ts.x * 0.5f, cy - hh * 0.5f - 8.f - ts.y);
        ImVec2 b0(p.x - 12.f, p.y - 6.f), b1(p.x + ts.x + 12.f, p.y + ts.y + 6.f);
        dl->AddRectFilled(b0, b1, UiCol(t.primary), 12.f);
        dl->AddText(font, fs, p, UiCol(t.on_primary), buf);
    }
    return changed;
}

// Tab：选中动画 + 悬停/按下缩放 + 指示条滑动感
bool TextTabList(ImDrawList* dl, const char* id, int* selected,
                 const char* const* labels, int count,
                 const ImVec2& min, const ImVec2& max, ImFont* font) {
    if (!selected || !labels || count <= 0) return false;
    const UiTheme& t = UiTheme_Current();
    // 按可用高度自适配行高，避免缩放面板后与频道标签重叠
    const float top_pad = 8.f;
    const float bot_pad = 6.f;
    const float avail = FMax(1.f, max.y - min.y - top_pad - bot_pad);
    float gap = 12.f;
    float item_h = g_m.tab_item_h;
    if (count > 0) {
        // 先用目标高度；不够则压缩 gap 与 item_h
        float need = item_h * count + gap * (count - 1);
        if (need > avail) {
            gap = FMax(6.f, gap * (avail / need));
            item_h = (avail - gap * (count - 1)) / (float)count;
            item_h = Clampf(item_h, 44.f, g_m.tab_item_h);
            // 再平衡 gap
            float used = item_h * count;
            gap = (count > 1) ? FMax(4.f, (avail - used) / (float)(count - 1)) : 0.f;
        }
    }
    float y = min.y + top_pad;
    bool changed = false;
    for (int i = 0; i < count; ++i) {
        char bid[64];
        std::snprintf(bid, sizeof(bid), "%s_t%d", id, i);
        TabA& an = g_tab[bid];
        ImVec2 raw_a(min.x + 12.f, y), raw_b(max.x - 12.f, y + item_h);
        const bool sel = (*selected == i);
        const bool hov = Hovered(raw_a, raw_b);

        an.sel = Approach(an.sel, sel ? 1.f : 0.f, 14.f);
        an.hover = Approach(an.hover, (hov && !sel) ? 1.f : 0.f, 16.f);
        // 按下反馈
        bool held = hov && ImGui::GetIO().MouseDown[0];
        an.press = Approach(an.press, held ? 1.f : 0.f, 26.f);

        float st = Smooth01(an.sel);
        float scale = 1.f + an.hover * 0.03f - an.press * 0.04f + st * 0.02f;
        ImVec2 c((raw_a.x + raw_b.x) * 0.5f, (raw_a.y + raw_b.y) * 0.5f);
        float hw = (raw_b.x - raw_a.x) * 0.5f * scale;
        float hh = (raw_b.y - raw_a.y) * 0.5f * scale;
        ImVec2 a(c.x - hw, c.y - hh), b(c.x + hw, c.y + hh);
        const float round = (b.y - a.y) * 0.30f;

        ImVec4 bg;
        bg.x = Lerpf(t.tab_unselected_bg.x, t.tab_selected_bg.x, st);
        bg.y = Lerpf(t.tab_unselected_bg.y, t.tab_selected_bg.y, st);
        bg.z = Lerpf(t.tab_unselected_bg.z, t.tab_selected_bg.z, st);
        bg.w = 1.f;
        // 悬停提亮
        if (an.hover > 0.01f && st < 0.95f) {
            bg.x = Clampf(bg.x * (1.f - 0.08f * an.hover), 0, 1);
            bg.y = Clampf(bg.y * (1.f - 0.08f * an.hover), 0, 1);
            bg.z = Clampf(bg.z * (1.f - 0.08f * an.hover), 0, 1);
        }

        // 选中外光
        if (st > 0.05f) {
            float pad = 2.f + st * 4.f;
            dl->AddRectFilled(ImVec2(a.x - pad, a.y - pad), ImVec2(b.x + pad, b.y + pad),
                              UiCol(t.primary, 0.08f * st), round + pad);
        }
        dl->AddRectFilled(a, b, UiCol(bg), round);

        if (font && labels[i]) {
            ImVec4 tc;
            tc.x = Lerpf(t.tab_text.x, t.tab_text_selected.x, st);
            tc.y = Lerpf(t.tab_text.y, t.tab_text_selected.y, st);
            tc.z = Lerpf(t.tab_text.z, t.tab_text_selected.z, st);
            tc.w = 1.f;
            const float tfs = FMin(g_m.body_fs, FMax(16.f, item_h * 0.36f));
            ImVec2 ts = font->CalcTextSizeA(tfs, FLT_MAX, 0.f, labels[i]);
            dl->AddText(font, tfs,
                        ImVec2(c.x - ts.x * 0.5f, c.y - ts.y * 0.5f),
                        UiCol(tc), labels[i]);
        }
        if (Hit(raw_a, raw_b, bid)) {
            if (*selected != i) {
                *selected = i;
                an.bounce = 1.f;
                changed = true;
            }
        }
        an.bounce = Approach(an.bounce, 0.f, 14.f);
        y += item_h + gap;
    }
    return changed;
}

bool Dropdown(ImDrawList* dl, const char* id, int* selected,
              const char* const* items, int count,
              const ImVec2& min, const ImVec2& max, ImFont* font) {
    if (!selected || !items || count <= 0) return false;
    const UiTheme& t = UiTheme_Current();
    DropA& an = g_drop[id];
    an.selected = selected; an.items = items; an.count = count;
    an.hmin = min; an.hmax = max;
    an.open = Approach(an.open, an.opened ? 1.f : 0.f, 16.f);

    const float h = max.y - min.y;
    const float round = Clampf(h * 0.22f, 14.f, 22.f);
    const bool hov = Hovered(min, max);
    an.hover = Approach(an.hover, (hov || an.opened) ? 1.f : 0.f, 16.f);
    an.press = Approach(an.press, (hov && ImGui::GetIO().MouseDown[0]) ? 1.f : 0.f, 24.f);
    float dscale = 1.f + an.hover * 0.015f - an.press * 0.03f;
    ImVec2 dc((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
    float dhw = (max.x - min.x) * 0.5f * dscale;
    float dhh = (max.y - min.y) * 0.5f * dscale;
    ImVec2 dmin(dc.x - dhw, dc.y - dhh), dmax(dc.x + dhw, dc.y + dhh);

    if (an.hover > 0.02f)
        dl->AddRectFilled(ImVec2(dmin.x - 3, dmin.y - 3), ImVec2(dmax.x + 3, dmax.y + 3),
                          UiCol(t.primary, 0.06f * an.hover), round + 3);
    dl->AddRectFilled(dmin, dmax, UiCol(t.field_bg), round);
    dl->AddRect(dmin, dmax, UiCol((hov || an.opened) ? t.field_border_focus : t.field_border),
                round, 0, (an.opened ? 2.5f : 2.f) + an.hover * 0.6f);

    if (*selected < 0) *selected = 0;
    if (*selected >= count) *selected = count - 1;
    if (font) {
        const char* cur = items[*selected] ? items[*selected] : "";
        const float fs = g_m.body_fs;
        ImVec2 ts = font->CalcTextSizeA(fs, FLT_MAX, 0.f, cur);
        dl->AddText(font, fs, ImVec2(min.x + 22.f, min.y + (h - ts.y) * 0.5f), UiCol(t.field_text), cur);
        float ax = max.x - 34.f, ay = min.y + h * 0.5f;
        float o = an.open;
        dl->AddTriangleFilled(
            ImVec2(ax - 10, ay - 5 + o * 2),
            ImVec2(ax + 10, ay - 5 + o * 2),
            ImVec2(ax, ay + 7 - o * 12),
            UiCol(t.on_surface_var)
        );
    }
    if (Hit(min, max, id)) an.opened = !an.opened;

    // 菜单高度：紧凑行高，避免弹出层过长
    const float item_h = 40.f;
    const float menu_pad = 6.f;
    const float gap = 6.f;
    const float full_h = menu_pad * 2.f + item_h * count;

    // 剩余空间自动检测：下方不够则翻到上方（仅用公开 API）
    const ImGuiIO& io = ImGui::GetIO();
    // 默认屏幕空间
    float bound_top = 0.f;
    float bound_bot = io.DisplaySize.y;
    // 若有主视口，用视口范围
    if (ImGuiViewport* vp = ImGui::GetMainViewport()) {
        bound_top = vp->Pos.y;
        bound_bot = vp->Pos.y + vp->Size.y;
    }
    // 再与当前窗口工作区取交集（公开 API）
    ImVec2 wpos = ImGui::GetWindowPos();
    ImVec2 wsize = ImGui::GetWindowSize();
    if (wsize.x > 1.f && wsize.y > 1.f) {
        bound_top = FMax(bound_top, wpos.y);
        bound_bot = FMin(bound_bot, wpos.y + wsize.y);
    }
    float space_bottom = bound_bot - max.y - gap;
    float space_top = min.y - bound_top - gap;
    // 下方不够完整菜单，且上方空间更大（或够放）→ 向上
    bool prefer_up = false;
    if (space_bottom < full_h) {
        if (space_top >= full_h || space_top > space_bottom)
            prefer_up = true;
    }
    // 打开瞬间锁定方向，避免滚动中抖动；关闭后下次重新判定
    if (an.opened && an.open < 0.15f)
        an.open_up = prefer_up;
    else if (!an.opened && an.open < 0.05f)
        an.open_up = prefer_up; // 预计算，展开首帧即正确

    if (an.open_up) {
        an.lmin = ImVec2(min.x, min.y - gap - full_h);
        an.lmax = ImVec2(max.x, min.y - gap);
    } else {
        an.lmin = ImVec2(min.x, max.y + gap);
        an.lmax = ImVec2(max.x, max.y + gap + full_h);
    }
    if (an.open > 0.02f || an.opened) g_drop_ids.push_back(id);
    return false;
}

void EndFrameOverlays(ImDrawList* dl, ImFont* font) {
    if (!dl) return;
    const UiTheme& t = UiTheme_Current();
    for (const auto& id : g_drop_ids) {
        auto it = g_drop.find(id);
        if (it == g_drop.end()) continue;
        DropA& an = it->second;
        if (an.open <= 0.02f || !an.items || an.count <= 0) continue;

        const float item_h = 40.f;
        const float menu_pad = 6.f;
        const float full_h = menu_pad * 2.f + item_h * an.count;
        // 完整圆角矩形 + alpha；位移方向随 open_up
        const float slide = (1.f - an.open) * 8.f;
        ImVec2 lmin = an.lmin;
        ImVec2 lmax = an.lmax;
        // 保证高度
        lmax.y = lmin.y + full_h;
        if (an.open_up) lmin.y -= slide; // 向上展开时略从上方滑入
        else            lmin.y += slide; // 向下展开时略从上方滑入
        lmax.y = lmin.y + full_h;

        const float round = 14.f;
        const float a = an.open;

        dl->AddRectFilled(ImVec2(lmin.x + 1.5f, lmin.y + 2.f), ImVec2(lmax.x + 1.5f, lmax.y + 2.f),
                          UiCol(ImVec4(0, 0, 0, 0.18f * a)), round);
        dl->AddRectFilled(lmin, lmax, UiCol(t.surface, a), round);
        dl->AddRect(lmin, lmax, UiCol(t.outline, 0.45f * a), round, 0, 1.4f);

        for (int i = 0; i < an.count; ++i) {
            ImVec2 a0(lmin.x + 10.f, lmin.y + menu_pad + item_h * i);
            ImVec2 b0(lmax.x - 10.f, a0.y + item_h);
            bool sel = an.selected && (*an.selected == i);
            bool hov = Hovered(a0, b0) && an.open > 0.85f;
            if (sel || hov)
                dl->AddRectFilled(a0, b0, UiCol(sel ? t.primary_container : t.secondary_container, a), 10.f);
            if (font && an.items[i]) {
                const float fs = g_m.body_fs * 0.92f;
                ImVec2 ts = font->CalcTextSizeA(fs, FLT_MAX, 0.f, an.items[i]);
                dl->AddText(font, fs, ImVec2(a0.x + 12.f, a0.y + (item_h - ts.y) * 0.5f),
                            UiCol(sel ? t.on_primary_container : t.on_surface, a), an.items[i]);
            }
            if (an.open > 0.9f) {
                char bid[80];
                std::snprintf(bid, sizeof(bid), "%s_o%d", id.c_str(), i);
                if (Hit(a0, b0, bid)) {
                    if (an.selected) *an.selected = i;
                    an.opened = false;
                }
            }
        }
        if (an.opened && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            ImVec2 mp = ImGui::GetIO().MousePos;
            bool in_h = Hovered(an.hmin, an.hmax);
            bool in_l = mp.x >= lmin.x && mp.x <= lmax.x && mp.y >= lmin.y && mp.y <= lmax.y;
            if (!in_h && !in_l) an.opened = false;
        }
    }
}

// 滚动条：仅溢出；胖椭圆拇指；内容区强制避让
// 补丁：底部安全余量 + 内容区拖拽滚动，避免「到底还有内容却滚不动/点不到」
float ScrollArea(ImDrawList* dl, const char* id, const ImVec2& min, const ImVec2& max,
                 float content_height, float& scroll_io) {
    const UiTheme& t = UiTheme_Current();
    const float view_h = max.y - min.y;
    // 底部额外可滚空间：最后一行控件完整露出 + 手指可点
    const float bottom_slack = 96.f;
    const float eff_content = content_height + bottom_slack;
    const float max_scroll = FMax(0.f, eff_content - view_h);
    const bool overflow = max_scroll > 1.f;
    scroll_io = Clampf(scroll_io, 0.f, max_scroll);

    ImGuiIO& io = ImGui::GetIO();
    const bool hover_area = Hovered(min, max);

    // 滚轮（触控板/鼠标）
    if (overflow && hover_area) {
        float wheel = io.MouseWheel;
        if (wheel != 0.f) scroll_io = Clampf(scroll_io - wheel * 96.f, 0.f, max_scroll);
    }

        // 内容区拖拽已去掉：全屏 InvisibleButton 会抢控件点击

    // 始终为滚动条预留槽位
    const float sb_w = 26.f;
    const float gap = 12.f;
    const float track_x0 = max.x - sb_w - 6.f;
    const float track_x1 = max.x - 6.f;
    float content_right = overflow ? (track_x0 - gap) : (max.x - 4.f);

    if (overflow) {
        const float track_top = min.y + 10.f;
        // 轨道底部多留一点，避免拇指到不了视觉底部
        const float track_bot = max.y - 12.f;
        const float track_h = FMax(1.f, track_bot - track_top);

        dl->AddRectFilled(ImVec2(track_x0, track_top), ImVec2(track_x1, track_bot),
                          UiCol(t.outline, 0.18f), sb_w * 0.5f);

        const float thumb_h = FMax(48.f, track_h * (view_h / FMax(1.f, eff_content)));
        const float travel = FMax(1.f, track_h - thumb_h);
        float ratio = (max_scroll > 0.f) ? (scroll_io / max_scroll) : 0.f;
        float thumb_y = track_top + ratio * travel;

        ImGui::SetCursorScreenPos(ImVec2(track_x0 - 10.f, min.y));
        ImGui::InvisibleButton(id, ImVec2(sb_w + 24.f, view_h));
        auto& dragging = g_sb_drag[id];
        auto& drag_off = g_sb_off[id];
        if (ImGui::IsItemActive()) {
            if (!dragging) {
                dragging = true;
                drag_off = io.MousePos.y - thumb_y;
            }
            float ny = io.MousePos.y - drag_off;
            float nt = Clampf((ny - track_top) / travel, 0.f, 1.f);
            scroll_io = nt * max_scroll;
            thumb_y = track_top + nt * travel;
        } else {
            dragging = false;
        }

        // 点击轨道空白处跳转
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && !dragging) {
            float my = io.MousePos.y;
            if (my < thumb_y || my > thumb_y + thumb_h) {
                float nt = Clampf((my - track_top - thumb_h * 0.5f) / travel, 0.f, 1.f);
                scroll_io = nt * max_scroll;
                thumb_y = track_top + nt * travel;
            }
        }

        dl->AddRectFilled(ImVec2(track_x0, thumb_y), ImVec2(track_x1, thumb_y + thumb_h),
                          UiCol(t.primary, 0.92f), sb_w * 0.5f);
    } else {
        scroll_io = 0.f;
        g_sb_drag[id] = false;
    }

    // 轻微再 clamp，防止数值漂移
    scroll_io = Clampf(scroll_io, 0.f, max_scroll);

    dl->PushClipRect(min, ImVec2(content_right, max.y), true);
    return min.y - scroll_io;
}

void ScrollAreaEnd(ImDrawList* dl) { dl->PopClipRect(); }

} // namespace Md3