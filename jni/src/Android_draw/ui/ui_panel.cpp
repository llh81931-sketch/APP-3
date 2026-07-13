#include "ui/ui_panel.h"
#include "ui/ui_theme.h"
#include "ui/ui_md3.h"
#include "game/game_esp.h"
#include "sys读写.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

static float Clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }

static int g_tab = 0;
static float g_scroll[4] = {0, 0, 0, 0};

static void DrawChrome(ImDrawList* dl, const ImVec2& min, const ImVec2& max, float rounding, float a) {
    const UiTheme& t = UiTheme_Current();
    const bool light = (t.mode == UiThemeMode::Light);
    const int layers = light ? 8 : 5;
    for (int i = layers; i >= 1; --i) {
        const float k = (float)i / (float)layers;
        const float pad = 1.f + k * (light ? 24.f : 16.f);
        dl->AddRectFilled(
            ImVec2(min.x - pad, min.y - pad), ImVec2(max.x + pad, max.y + pad),
            UiCol(t.panel_glow, (light ? 0.10f : 0.07f) * (1.f - k * 0.72f) * a),
            rounding + pad * 0.9f
        );
    }
    if (light) {
        dl->AddRect(ImVec2(min.x - 3, min.y - 3), ImVec2(max.x + 3, max.y + 3),
                    UiCol(ImVec4(1, 1, 1, 0.28f), a), rounding + 3, 0, 2.f);
    }
    dl->AddRectFilled(min, max, UiCol(t.panel_bg, a), rounding);
    dl->AddRect(min, max, UiCol(t.panel_border, a), rounding, 0, 1.8f);
}

static float HHome()  { return 100 + 120 + 16 + 120 + 16 + 88 + 48; }
static float HDraw() {
    // 透视开关 + 一键
    return 100 + 80 + 28 + 3 * 112 + 120;
}
static float HAim() {
    return 100 + 28 + 2 * 112 + 28 + 2 * 110 + 26 + 72 + 140;
}
static float HSettings() {
    return 100 + 118 + 156 + 88 + 120;
}

static void Sw2(ImDrawList* dl, float x0, float x1, float& y, ImFont* font,
                const char* t1, const char* s1, bool* v1, const char* id1,
                const char* t2, const char* s2, bool* v2, const char* id2) {
    const auto& m = Md3::M();
    float gap = 12.f;
    float cw = (x1 - x0 - gap) * 0.5f;
    float h = m.row_h;
    Md3::Card(dl, ImVec2(x0, y), ImVec2(x0 + cw, y + h), 18.f);
    Md3::ListRow(dl, ImVec2(x0, y), ImVec2(x0 + cw - 110.f, y + h), font, t1, s1);
    Md3::Switch(dl, id1, v1, ImVec2(x0 + cw - 20.f - m.switch_w, y + (h - m.switch_h) * 0.5f), m.switch_h);
    Md3::Card(dl, ImVec2(x0 + cw + gap, y), ImVec2(x1, y + h), 18.f);
    Md3::ListRow(dl, ImVec2(x0 + cw + gap, y), ImVec2(x1 - 110.f, y + h), font, t2, s2);
    Md3::Switch(dl, id2, v2, ImVec2(x1 - 20.f - m.switch_w, y + (h - m.switch_h) * 0.5f), m.switch_h);
    y += h + 12.f;
}

static void Sw1(ImDrawList* dl, float x0, float x1, float& y, ImFont* font,
                const char* title, const char* sub, bool* val, const char* sid) {
    const auto& m = Md3::M();
    Md3::Card(dl, ImVec2(x0, y), ImVec2(x1, y + m.row_h), 18.f);
    Md3::ListRow(dl, ImVec2(x0, y), ImVec2(x1 - 140.f, y + m.row_h), font, title, sub);
    Md3::Switch(dl, sid, val, ImVec2(x1 - 24.f - m.switch_w, y + (m.row_h - m.switch_h) * 0.5f), m.switch_h);
    y += m.row_h + 12.f;
}

static void DrawHome(ImDrawList* dl, float x0, float x1, float y, ImFont* font) {
    const UiTheme& t = UiTheme_Current();
    const auto& m = Md3::M();
    auto& gr = GameEsp::R();

    Md3::Text(dl, font, m.title_fs, ImVec2(x0, y), t.on_surface, "首页");
    Md3::Text(dl, font, m.label_fs, ImVec2(x0, y + 48), t.on_surface_var, "驱动状态 · 手动初始化");
    y += 100;

    float gap = 16.f, cw = (x1 - x0 - gap) * 0.5f, ch = 120.f;
    Md3::Card(dl, ImVec2(x0, y), ImVec2(x0 + cw, y + ch), 22.f);
    Md3::Text(dl, font, m.hint_fs, ImVec2(x0 + 24, y + 20), t.on_surface_var, "内核驱动");
    Md3::Text(dl, font, 30.f, ImVec2(x0 + 24, y + 58), t.on_surface, MemDriver_Ready() ? MemDriver_Name() : "未连接");

    Md3::Card(dl, ImVec2(x0 + cw + gap, y), ImVec2(x1, y + ch), 22.f);
    Md3::Text(dl, font, m.hint_fs, ImVec2(x0 + cw + gap + 24, y + 20), t.on_surface_var, "游戏进程");
    char pidbuf[48];
    if (gr.pid > 0) snprintf(pidbuf, sizeof(pidbuf), "pid %d", gr.pid);
    else snprintf(pidbuf, sizeof(pidbuf), "未绑定");
    Md3::Text(dl, font, 28.f, ImVec2(x0 + cw + gap + 24, y + 58), t.on_surface, pidbuf);
    y += ch + 16;

    Md3::Card(dl, ImVec2(x0, y), ImVec2(x1, y + ch), 22.f);
    Md3::Text(dl, font, m.hint_fs, ImVec2(x0 + 24, y + 18), t.on_surface_var, "状态");
    Md3::Text(dl, font, 22.f, ImVec2(x0 + 24, y + 54), t.on_surface, gr.status);
    const char* tip = gr.inited ? "可再次点初始化刷新数据" : "进局后点下方初始化";
    Md3::Text(dl, font, 18.f, ImVec2(x0 + 24, y + 86), t.on_surface_var, tip);
    y += ch + 16;

    // 唯一数据入口：每次点击都重新初始化
    if (Md3::Button(dl, "##home_init", gr.inited ? "重新初始化" : "初始化", ImVec2(x0, y), ImVec2(x1, y + m.btn_h), font, true)) {
        GameEsp::InitAfterDriver();
    }
}

static void DrawDrawPage(ImDrawList* dl, float x0, float x1, float y, ImFont* font) {
    const UiTheme& t = UiTheme_Current();
    const auto& m = Md3::M();
    auto& s = GameEsp::R().s;

    Md3::Text(dl, font, m.title_fs, ImVec2(x0, y), t.on_surface, "绘制");
    Md3::Text(dl, font, m.label_fs, ImVec2(x0, y + 48), t.on_surface_var, "透视显示");
    y += 100;

    float bw = (x1 - x0 - 12.f) * 0.5f;
    if (Md3::Button(dl, "##all_on", "一键全开", ImVec2(x0, y), ImVec2(x0 + bw, y + m.btn_h - 8), font, true))
        GameEsp::SetAllDraw(true);
    if (Md3::Button(dl, "##all_off", "一键全关", ImVec2(x0 + bw + 12.f, y), ImVec2(x1, y + m.btn_h - 8), font, false))
        GameEsp::SetAllDraw(false);
    y += m.btn_h + 8;

    Md3::Text(dl, font, m.hint_fs, ImVec2(x0, y), t.on_surface_var, "透视");
    y += 28;
    Sw2(dl, x0, x1, y, font, "方框", "人物外框", &s.box, "##sw_box", "射线", "准心连线", &s.line, "##sw_line");
    Sw2(dl, x0, x1, y, font, "昵称", "描边名/距离", &s.name, "##sw_name", "骨骼", "火柴人", &s.bone, "##sw_bone");
    Sw1(dl, x0, x1, y, font, "玩家数量", "灵动岛跳动显示", &s.count, "##sw_count");
}

static void DrawAimPage(ImDrawList* dl, float x0, float x1, float y, ImFont* font) {
    const UiTheme& t = UiTheme_Current();
    const auto& m = Md3::M();
    auto& s = GameEsp::R().s;

    Md3::Text(dl, font, m.title_fs, ImVec2(x0, y), t.on_surface, "自瞄");
    Md3::Text(dl, font, m.label_fs, ImVec2(x0, y + 48), t.on_surface_var, "触摸跟枪 · 参数");
    y += 100;

    Md3::Text(dl, font, m.hint_fs, ImVec2(x0, y), t.on_surface_var, "开关");
    y += 28;
    Sw2(dl, x0, x1, y, font, "开启自瞄", "开火跟枪", &s.aim, "##sw_aim", "显示触摸区", "长按拖动校准", &s.aim_touch_pos, "##sw_tp");
    Sw1(dl, x0, x1, y, font, "动态自瞄", "DrawIo[25]", &s.aim_dynamic, "##sw_dyn");

    Md3::Text(dl, font, m.hint_fs, ImVec2(x0, y), t.on_surface_var, "参数");
    y += 28;
    float gap = 12.f;
    float cw = (x1 - x0 - gap) * 0.5f;
    Md3::Text(dl, font, 16.f, ImVec2(x0, y), t.on_surface_var, "范围");
    Md3::Text(dl, font, 16.f, ImVec2(x0 + cw + gap, y), t.on_surface_var, "触摸范围");
    y += 24;
    Md3::Slider(dl, "##fov", &s.aim_fov, 10.f, 500.f, ImVec2(x0, y), ImVec2(x0 + cw, y + m.slider_h), font, true);
    Md3::Slider(dl, "##trange", &s.touch_range, 5.f, 600.f, ImVec2(x0 + cw + gap, y), ImVec2(x1, y + m.slider_h), font, true);
    y += m.slider_h + 14;
    Md3::Text(dl, font, 16.f, ImVec2(x0, y), t.on_surface_var, "力度(小=快)");
    Md3::Text(dl, font, 16.f, ImVec2(x0 + cw + gap, y), t.on_surface_var, "速度(小=快)");
    y += 24;
    Md3::Slider(dl, "##force", &s.aim_force, 0.1f, 50.f, ImVec2(x0, y), ImVec2(x0 + cw, y + m.slider_h), font, true);
    Md3::Slider(dl, "##aspeed", &s.aim_speed, 0.1f, 30.f, ImVec2(x0 + cw + gap, y), ImVec2(x1, y + m.slider_h), font, true);
    y += m.slider_h + 14;

    Md3::Text(dl, font, m.hint_fs, ImVec2(x0, y), t.on_surface_var, "瞄准部位");
    y += 26;
    static const char* kParts[] = { "头部", "胸部", "腿部" };
    Md3::Dropdown(dl, "##aim_part", &s.aim_part, kParts, 3, ImVec2(x0, y), ImVec2(x1, y + m.field_h), font);
}

static void DrawSettings(ImDrawList* dl, float x0, float x1, float y, ImFont* font) {
    const UiTheme& t = UiTheme_Current();
    const auto& m = Md3::M();
    auto& gr = GameEsp::R();

    Md3::Text(dl, font, m.title_fs, ImVec2(x0, y), t.on_surface, "设置");
    Md3::Text(dl, font, m.label_fs, ImVec2(x0, y + 48), t.on_surface_var, "主题 · 调试 · 退出");
    y += 100;

    Md3::Card(dl, ImVec2(x0, y), ImVec2(x1, y + m.row_h + 6), 20.f);
    const char* theme_sub = UiTheme_IsAnimating()
        ? "切换动画中…"
        : (UiTheme_GetMode() == UiThemeMode::Dark ? "当前：暗色" : "当前：亮色");
    Md3::ListRow(dl, ImVec2(x0, y), ImVec2(x1 - 150.f, y + m.row_h), font, "界面主题", theme_sub);
    {
        float bw = 130.f;
        ImVec2 b0(x1 - 22.f - bw, y + (m.row_h - m.btn_h + 8) * 0.5f);
        ImVec2 b1(x1 - 22.f, b0.y + m.btn_h - 8);
        if (Md3::Button(dl, "##theme_toggle",
                        UiTheme_GetMode() == UiThemeMode::Dark ? "亮色" : "暗色",
                        b0, b1, font, true)) {
            UiTheme_Toggle();
        }
    }
    y += m.row_h + 18;

    char l1[96], l2[96], l3[96], l4[96];
    snprintf(l1, sizeof(l1), "Uworld  %p", (void*)gr.uworld);
    snprintf(l2, sizeof(l2), "Matrix  %p", (void*)gr.matrix_addr);
    snprintf(l3, sizeof(l3), "Array   %p", (void*)gr.array_addr);
    snprintf(l4, sizeof(l4), "libunity %p", (void*)gr.libunity);
    Md3::Card(dl, ImVec2(x0, y), ImVec2(x1, y + 140), 20.f);
    Md3::Text(dl, font, 17.f, ImVec2(x0 + 20, y + 16), t.on_surface_var, l1);
    Md3::Text(dl, font, 17.f, ImVec2(x0 + 20, y + 46), t.on_surface_var, l2);
    Md3::Text(dl, font, 17.f, ImVec2(x0 + 20, y + 76), t.on_surface_var, l3);
    Md3::Text(dl, font, 17.f, ImVec2(x0 + 20, y + 106), t.on_surface_var, l4);
    y += 156;

    if (Md3::Button(dl, "##exit_assist", "退出辅助", ImVec2(x0, y), ImVec2(x1, y + m.btn_h), font, true)) {
        exit(0);
    }
}

// 左下角频道标签：随侧栏/面板高度自适配，禁止侵入 Tab 区
// 返回占用高度（含底边距）；放不下则不绘制并返回 0
static float DrawChannelPill(ImDrawList* dl, const ImVec2& panel_min, float sidebar_w, float panel_bottom,
                             float tab_limit_y, ImFont* font, float a) {
    if (a < 0.05f || sidebar_w < 90.f) return 0.f;
    const UiTheme& t = UiTheme_Current();
    const char* text = "@这是频道";

    float fs = Clampf(sidebar_w * 0.12f, 14.f, Md3::M().label_fs);
    ImVec2 ts(0, 0);
    if (font) ts = font->CalcTextSizeA(fs, FLT_MAX, 0.f, text);

    float pad_x = Clampf(sidebar_w * 0.08f, 10.f, 22.f);
    float pad_y = Clampf(fs * 0.45f, 6.f, 14.f);
    float h = ts.y + pad_y * 2.f;
    float w = ts.x + pad_x * 2.f;
    float max_w = sidebar_w - 16.f;
    if (w > max_w && font) {
        float scale = max_w / (w > 1.f ? w : 1.f);
        fs = (12.f > (fs * scale) ? 12.f : (fs * scale));
        ts = font->CalcTextSizeA(fs, FLT_MAX, 0.f, text);
        pad_x = Clampf(sidebar_w * 0.07f, 8.f, 18.f);
        pad_y = Clampf(fs * 0.4f, 5.f, 12.f);
        h = ts.y + pad_y * 2.f;
        w = (max_w < (ts.x + pad_x * 2.f) ? max_w : (ts.x + pad_x * 2.f));
    }

    float panel_h = panel_bottom - panel_min.y;
    float margin = Clampf(sidebar_w * 0.10f, 10.f, 28.f);
    if (panel_h < 520.f) margin = Clampf(margin * 0.75f, 8.f, 18.f);
    if (panel_h < 420.f) margin = Clampf(margin * 0.65f, 6.f, 14.f);

    float max_bottom = panel_bottom - margin;
    float min_top = tab_limit_y + 6.f;
    if (max_bottom - h < min_top) {
        fs = (fs * 0.85f < 11.f) ? 11.f : (fs * 0.85f);
        if (font) ts = font->CalcTextSizeA(fs, FLT_MAX, 0.f, text);
        pad_y = 4.f;
        pad_x = Clampf(pad_x * 0.85f, 6.f, 14.f);
        h = ts.y + pad_y * 2.f;
        w = (max_w < (ts.x + pad_x * 2.f) ? max_w : (ts.x + pad_x * 2.f));
        margin = (margin * 0.6f < 4.f) ? 4.f : (margin * 0.6f);
        max_bottom = panel_bottom - margin;
        if (max_bottom - h < min_top) return 0.f;
    }

    ImVec2 pmin(panel_min.x + (sidebar_w - w) * 0.5f, max_bottom - h);
    if (pmin.y < min_top) pmin.y = min_top;
    if (pmin.y + h > panel_bottom - 4.f) return 0.f;
    if (pmin.x < panel_min.x + 8.f) pmin.x = panel_min.x + 8.f;
    if (pmin.x + w > panel_min.x + sidebar_w - 8.f) pmin.x = panel_min.x + sidebar_w - 8.f - w;

    ImVec2 pmax(pmin.x + w, pmin.y + h);
    const float round = h * 0.5f;
    dl->AddRectFilled(pmin, pmax, UiCol(t.primary_container, a), round);
    dl->AddRect(pmin, pmax, UiCol(t.outline, 0.45f * a), round, 0, 1.4f);
    if (font) {
        dl->AddText(font, fs, ImVec2(pmin.x + (w - ts.x) * 0.5f, pmin.y + (h - ts.y) * 0.5f),
                    UiCol(t.on_primary_container, a), text);
    }
    return (panel_bottom - pmin.y) + 4.f;
}

// 右下角缩放命中区：仅斜线，无小方块底板；命中区加大
static bool DrawResizeHandle(ImDrawList* dl, const ImVec2& panel_min, const ImVec2& panel_max, float a) {
    (void)panel_min;
    const UiTheme& t = UiTheme_Current();
    const float s = 56.f; // 更大命中区
    ImVec2 hmin(panel_max.x - s - 4.f, panel_max.y - s - 4.f);
    ImVec2 hmax(panel_max.x - 4.f, panel_max.y - 4.f);

    // 只画斜线，不画正方形底板
    ImU32 col = UiCol(t.on_surface_var, 0.70f * a);
    ImU32 col2 = UiCol(t.on_surface, 0.35f * a);
    for (int i = 0; i < 3; ++i) {
        float o = 14.f + i * 9.f;
        dl->AddLine(ImVec2(hmax.x - 10.f - o, hmax.y - 10.f),
                    ImVec2(hmax.x - 10.f, hmax.y - 10.f - o), col, 3.0f);
        dl->AddLine(ImVec2(hmax.x - 10.f - o + 1.f, hmax.y - 10.f),
                    ImVec2(hmax.x - 10.f, hmax.y - 10.f - o + 1.f), col2, 1.2f);
    }

    ImGui::SetCursorScreenPos(hmin);
    ImGui::InvisibleButton("##panel_resize", ImVec2(s, s));
    return ImGui::IsItemActive();
}


// 头像下方品牌：LY内核 青蓝静态渐变
static void DrawLyBrand(ImDrawList* dl, const ImVec2& panel_min, float sidebar_w, float top_after_avatar, ImFont* font, float a) {
    if (!font || a < 0.05f || sidebar_w < 70.f) return;
    const char* text = "LY内核";
    // 侧栏窄时略缩小，保证完整显示
    float fs = Clampf(sidebar_w * 0.145f, 13.f, 22.f);
    ImVec2 ts = font->CalcTextSizeA(fs, FLT_MAX, 0.f, text);
    // 过宽则再缩
    float max_w = sidebar_w - 16.f;
    if (ts.x > max_w && ts.x > 1.f) {
        fs *= max_w / ts.x;
        if (fs < 12.f) fs = 12.f;
        ts = font->CalcTextSizeA(fs, FLT_MAX, 0.f, text);
    }
    float cx = panel_min.x + sidebar_w * 0.5f;
    float y = top_after_avatar + 4.f;
    ImVec2 tp(cx - ts.x * 0.5f, y);

    // 阴影
    dl->AddText(font, fs, ImVec2(tp.x + 1.2f, tp.y + 1.4f), UiCol(ImVec4(0.02f, 0.08f, 0.12f, 0.55f), a), text);

    // 青 → 蓝 水平静态渐变（三段 clip 叠字）
    // 左：青绿
    dl->PushClipRect(ImVec2(tp.x - 2.f, tp.y - 2.f), ImVec2(tp.x + ts.x * 0.42f, tp.y + ts.y + 2.f), true);
    dl->AddText(font, fs, tp, UiCol(ImVec4(0.15f, 0.92f, 0.88f, 1.f), a), text);
    dl->PopClipRect();
    // 中：青蓝
    dl->PushClipRect(ImVec2(tp.x + ts.x * 0.32f, tp.y - 2.f), ImVec2(tp.x + ts.x * 0.68f, tp.y + ts.y + 2.f), true);
    dl->AddText(font, fs, tp, UiCol(ImVec4(0.20f, 0.72f, 1.0f, 1.f), a), text);
    dl->PopClipRect();
    // 右：亮蓝
    dl->PushClipRect(ImVec2(tp.x + ts.x * 0.58f, tp.y - 2.f), ImVec2(tp.x + ts.x + 2.f, tp.y + ts.y + 2.f), true);
    dl->AddText(font, fs, tp, UiCol(ImVec4(0.28f, 0.48f, 1.0f, 1.f), a), text);
    dl->PopClipRect();
}

// 左上角人物头像：随侧栏自适应
static void DrawAvatar(ImDrawList* dl, const ImVec2& panel_min, float sidebar_w, float top_pad, const TextureInfo* avatar, float a, float size_override = 0.f) {
    if (!avatar || avatar->DS == 0 || avatar->w <= 0 || avatar->h <= 0 || a < 0.05f) return;
    if (sidebar_w < 70.f) return;
    const float size = (size_override > 1.f) ? size_override : Clampf(sidebar_w * 0.52f, 48.f, 110.f);
    const float cx = panel_min.x + sidebar_w * 0.5f;
    const float cy = panel_min.y + top_pad + size * 0.5f + 4.f;
    ImVec2 amin(cx - size * 0.5f, cy - size * 0.5f);
    ImVec2 amax(cx + size * 0.5f, cy + size * 0.5f);
    dl->PushClipRect(amin, amax, true);
    dl->AddCircleFilled(ImVec2(cx, cy), size * 0.5f, UiCol(UiTheme_Current().surface_variant, a), 48);
    dl->AddImageRounded(
        (ImTextureID)(intptr_t)avatar->DS,
        amin, amax,
        ImVec2(0, 0), ImVec2(1, 1),
        UiCol(ImVec4(1, 1, 1, a)),
        size * 0.5f
    );
    dl->PopClipRect();
    dl->AddCircle(ImVec2(cx, cy), size * 0.5f, UiCol(UiTheme_Current().outline, 0.5f * a), 48, 2.f);
}

// 完整内容仅在展开充分时绘制，避免最小化/缩放动画中控件错位
void UiPanel_Draw(ImDrawList* dl, const ImVec2& min, const ImVec2& max, float rounding, ImFont* font, float content_alpha, const TextureInfo* avatar_tex, bool* out_resize_active) {
    if (out_resize_active) *out_resize_active = false;
    float a = Clampf(content_alpha, 0.f, 1.f);
    DrawChrome(dl, min, max, rounding, a < 0.12f ? 0.12f + a * 0.88f : a);

    // 动画早期/面板过小：只画外壳，不画内部控件（防诡异错位）
    const float W = max.x - min.x;
    const float H = max.y - min.y;
    if (!font || a < 0.18f || W < 220.f || H < 220.f) return;

    // 内容淡入：未充分展开时降低内部可见度，并裁剪到面板内
    const float inner_a = Clampf((a - 0.18f) / 0.55f, 0.f, 1.f);
    if (inner_a <= 0.01f) return;

    dl->PushClipRect(min, max, true);

    const UiTheme& t = UiTheme_Current();
    float pad = Clampf(W * 0.024f, 12.f, 28.f);
    // 侧栏随宽自适应，过窄时压缩
    float sidebar_w = Clampf(W * 0.24f, 120.f, 220.f);
    if (W < 520.f) sidebar_w = Clampf(W * 0.28f, 100.f, 180.f);

    dl->AddRectFilled(min, ImVec2(min.x + sidebar_w, max.y), UiCol(t.sidebar_bg, a * inner_a), rounding, ImDrawFlags_RoundCornersLeft);
    dl->AddLine(ImVec2(min.x + sidebar_w, min.y + 12), ImVec2(min.x + sidebar_w, max.y - 12), UiCol(t.divider, a * inner_a), 1.6f);

    // 侧栏垂直自适应：矮面板压缩头像/品牌，预留频道，Tab 吃剩余高度
    float av_size = Clampf(sidebar_w * 0.52f, 48.f, 110.f);
    bool show_brand = true;
    if (H < 560.f) av_size = Clampf(sidebar_w * 0.42f, 40.f, 84.f);
    if (H < 480.f) { av_size = Clampf(sidebar_w * 0.36f, 36.f, 68.f); show_brand = false; }
    if (H < 400.f) { av_size = Clampf(sidebar_w * 0.30f, 32.f, 56.f); show_brand = false; }

    float brand_h = show_brand ? (Clampf(sidebar_w * 0.18f, 16.f, 28.f) + 14.f) : 0.f;
    float avatar_slot = av_size + 12.f + brand_h;
    // 频道预留：按面板高度动态，矮面板更紧
    float channel_reserve = Clampf(sidebar_w * 0.38f, 40.f, 72.f);
    if (H < 520.f) channel_reserve = Clampf(channel_reserve * 0.85f, 34.f, 56.f);
    if (H < 420.f) channel_reserve = Clampf(channel_reserve * 0.75f, 28.f, 46.f);

    // 保证 Tab 至少有最小高度，否则继续压头像区
    float tab_min_need = 4.f * 48.f + 3.f * 6.f + 16.f; // 紧凑 4 Tab
    float tab_top = min.y + pad + avatar_slot;
    float tab_bottom = max.y - pad - channel_reserve;
    if (tab_bottom - tab_top < tab_min_need && show_brand) {
        show_brand = false;
        brand_h = 0.f;
        avatar_slot = av_size + 12.f;
        tab_top = min.y + pad + avatar_slot;
    }
    if (tab_bottom - tab_top < tab_min_need) {
        // 再压频道预留
        channel_reserve = Clampf(channel_reserve * 0.7f, 24.f, 40.f);
        tab_bottom = max.y - pad - channel_reserve;
    }
    if (tab_bottom - tab_top < tab_min_need) {
        av_size = Clampf(av_size * 0.75f, 28.f, 48.f);
        avatar_slot = av_size + 8.f;
        tab_top = min.y + pad + avatar_slot;
    }

    // 头像/品牌（高度压缩后）
    if (av_size >= 28.f) {
        // 临时用局部缩放：DrawAvatar 内部仍按 sidebar 算 size，这里仅在空间够时画
        // 通过减小 top_pad 视觉贴近
        DrawAvatar(dl, min, sidebar_w, pad, avatar_tex, a * inner_a, av_size);
        if (show_brand) {
            float av_draw = Clampf(sidebar_w * 0.52f, 48.f, 110.f);
            if (H < 560.f) av_draw = av_size;
            DrawLyBrand(dl, min, sidebar_w, min.y + pad + av_size + 6.f, font, a * inner_a);
        }
    }

    // Tab 吃 [tab_top, tab_bottom]，内部再按高度压缩行高
    if (tab_bottom - tab_top > 60.f) {
        static const char* kTabs[] = { "首页", "绘制", "自瞄", "设置" };
        Md3::TextTabList(dl, "##side_tabs", &g_tab, kTabs, 4,
                         ImVec2(min.x, tab_top), ImVec2(min.x + sidebar_w, tab_bottom), font);
    }

    // 频道标签：不得越过 tab_bottom
    DrawChannelPill(dl, min, sidebar_w, max.y, tab_bottom, font, a * inner_a);

    // 右侧内容区：为滚动条 + 右下缩放手柄预留
    const float resize_pad = 52.f;
    const float sb_reserve = 40.f; // 滚动条避让
    ImVec2 rmin(min.x + sidebar_w + pad, min.y + pad);
    ImVec2 rmax(max.x - pad, max.y - pad);
    // 内容右边界先减滚动条预留，再给手柄
    rmax.x -= 4.f;
    rmax.y -= 8.f;

    // 内容区过小则跳过
    if ((rmax.x - rmin.x) > 80.f && (rmax.y - rmin.y) > 80.f && inner_a > 0.35f) {
        float content_h = (g_tab == 0) ? HHome() : (g_tab == 1 ? HDraw() : (g_tab == 2 ? HAim() : HSettings()));
        if (g_tab < 0) g_tab = 0; if (g_tab > 3) g_tab = 3;
        ImVec2 smax(rmax.x, rmax.y - 10.f);
        float y0 = Md3::ScrollArea(dl, "##right_scroll", rmin, smax, content_h, g_scroll[g_tab]);
        float x0 = rmin.x;
        float x1 = rmax.x - sb_reserve;
        if (x1 > x0 + 40.f) {
            if (g_tab == 0) DrawHome(dl, x0, x1, y0, font);
            else if (g_tab == 1) DrawDrawPage(dl, x0, x1, y0, font);
            else if (g_tab == 2) DrawAimPage(dl, x0, x1, y0, font);
            else DrawSettings(dl, x0, x1, y0, font);
        }
        Md3::ScrollAreaEnd(dl);
        Md3::EndFrameOverlays(dl, font);
    }

    // 充分展开才显示缩放手柄
    bool resizing = false;
    if (inner_a > 0.85f) {
        resizing = DrawResizeHandle(dl, min, max, a * inner_a);
    }
    if (out_resize_active) *out_resize_active = resizing;

    dl->PopClipRect();
}
