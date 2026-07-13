#include "ui/ui_anim.h"
#include <cmath>

static IslandAnimMode g_mode = IslandAnimMode::Collapsed;
static float g_t = 0.0f;

// 总约 1.0 秒：更快更干脆
static constexpr float kDuration = 1.00f;
static constexpr float kMaxDt = 1.0f / 40.0f;

static float FMin(float a, float b) { return a < b ? a : b; }
static float FMax(float a, float b) { return a > b ? a : b; }

IslandAnimMode& UiAnim_Mode() { return g_mode; }
float& UiAnim_T() { return g_t; }

bool UiAnim_IsAnimating() {
    return g_mode == IslandAnimMode::Expanding || g_mode == IslandAnimMode::Collapsing;
}

void UiAnim_RequestExpand() {
    if (g_mode == IslandAnimMode::Collapsed) {
        g_mode = IslandAnimMode::Expanding;
        g_t = 0.0f;
    }
}

void UiAnim_RequestCollapse() {
    if (g_mode == IslandAnimMode::Expanded) {
        g_mode = IslandAnimMode::Collapsing;
        g_t = 0.0f;
    }
}

void UiAnim_Tick(float dt) {
    if (dt > kMaxDt) dt = kMaxDt;
    if (dt < 0.0f) dt = 0.0f;
    if (!UiAnim_IsAnimating()) return;
    g_t += dt / kDuration;
    if (g_t >= 1.0f) {
        g_t = 1.0f;
        g_mode = (g_mode == IslandAnimMode::Expanding)
            ? IslandAnimMode::Expanded
            : IslandAnimMode::Collapsed;
    }
}

float UiAnim_Clamp(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
float UiAnim_Lerp(float a, float b, float t) { return a + (b - a) * t; }
ImVec2 UiAnim_Lerp2(const ImVec2& a, const ImVec2& b, float t) {
    return ImVec2(UiAnim_Lerp(a.x, b.x, t), UiAnim_Lerp(a.y, b.y, t));
}
float UiAnim_Smooth01(float t) {
    t = UiAnim_Clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
float UiAnim_SpringSoft(float t) {
    t = UiAnim_Clamp(t, 0.0f, 1.0f);
    if (t >= 1.0f) return 1.0f;
    float base = 1.0f - std::pow(1.0f - t, 2.8f);
    float bounce = std::exp(-5.5f * t) * std::sin(t * 3.14159265f * 2.0f) * 0.030f;
    float v = base + bounce * (1.0f - t);
    if (t > 0.90f) {
        float u = (t - 0.90f) / 0.10f;
        v = UiAnim_Lerp(v, 1.0f, u * u * (3.0f - 2.0f * u));
    }
    return v;
}

// 连续 morph：
// 0.00-0.45 缩小+下落（约 0.45s）
// 0.35-1.00 展开（重叠，约后半段）
// 落点 = 面板中心（面板本身可偏上，不必屏幕正中）
MorphGeom UiAnim_EvalMorph(
    float t_expand,
    const ImVec2& island_min, const ImVec2& island_max,
    const ImVec2& panel_min, const ImVec2& panel_max
) {
    MorphGeom g{};
    float t = UiAnim_Clamp(t_expand, 0.0f, 1.0f);

    const ImVec2 island_c((island_min.x + island_max.x) * 0.5f, (island_min.y + island_max.y) * 0.5f);
    const ImVec2 panel_c((panel_min.x + panel_max.x) * 0.5f, (panel_min.y + panel_max.y) * 0.5f);
    const float island_w = island_max.x - island_min.x;
    const float island_h = island_max.y - island_min.y;
    const float panel_w = panel_max.x - panel_min.x;
    const float panel_h = panel_max.y - panel_min.y;
    const float panel_round = UiAnim_Clamp(FMin(panel_w, panel_h) * 0.06f, 18.0f, 36.0f);
    const float drop_r = FMax(7.0f, island_h * 0.18f);

    auto ramp = [](float x, float a, float b) {
        if (b <= a) return x >= b ? 1.0f : 0.0f;
        return UiAnim_Smooth01(UiAnim_Clamp((x - a) / (b - a), 0.0f, 1.0f));
    };

    // 更快：shrink+fall 前半段完成，后半段展开
    const float shrink = ramp(t, 0.00f, 0.32f);
    const float fall   = ramp(t, 0.10f, 0.55f); // 下落约占前 0.55s
    const float expand = UiAnim_SpringSoft(ramp(t, 0.42f, 1.00f));

    const ImVec2 c = UiAnim_Lerp2(island_c, panel_c, fall);

    const float w_after_shrink = UiAnim_Lerp(island_w, drop_r * 2.0f, shrink);
    const float h_after_shrink = UiAnim_Lerp(island_h, drop_r * 2.0f, shrink);
    const float cur_w = UiAnim_Lerp(w_after_shrink, panel_w, expand);
    const float cur_h = UiAnim_Lerp(h_after_shrink, panel_h, expand);

    g.min = ImVec2(c.x - cur_w * 0.5f, c.y - cur_h * 0.5f);
    g.max = ImVec2(c.x + cur_w * 0.5f, c.y + cur_h * 0.5f);

    const float round_mid = UiAnim_Lerp(island_h * 0.5f, drop_r, shrink);
    g.rounding = UiAnim_Lerp(round_mid, panel_round, expand);
    const float max_r = (g.max.y - g.min.y) * 0.5f;
    if (g.rounding > max_r) g.rounding = max_r;

    const float drip_in = ramp(t, 0.08f, 0.30f);
    const float drip_out = 1.0f - ramp(t, 0.42f, 0.62f);
    g.drip = drip_in * drip_out * 0.80f;

    if (expand > 0.10f) g.form = MorphForm::Panel;
    else if (shrink > 0.50f) g.form = MorphForm::Droplet;
    else g.form = MorphForm::Island;

    g.island_a = 1.0f - UiAnim_Smooth01(UiAnim_Clamp(shrink / 0.70f, 0.0f, 1.0f));
    g.content_a = UiAnim_Clamp((expand - 0.20f) / 0.55f, 0.0f, 1.0f);
    g.alpha = 1.0f;

    if (t <= 0.0001f) {
        g.form = MorphForm::Island;
        g.min = island_min; g.max = island_max;
        g.rounding = island_h * 0.5f;
        g.island_a = 1.0f; g.content_a = 0.0f; g.drip = 0.0f;
    } else if (t >= 0.9999f) {
        g.form = MorphForm::Panel;
        g.min = panel_min; g.max = panel_max;
        g.rounding = panel_round;
        g.island_a = 0.0f; g.content_a = 1.0f; g.drip = 0.0f;
    }
    return g;
}