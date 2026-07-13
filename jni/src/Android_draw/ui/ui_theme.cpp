#include "ui/ui_theme.h"

static UiTheme g_theme;
static ImVec4 V(float r, float g, float b, float a = 1.0f) { return ImVec4(r, g, b, a); }

UiTheme& UiTheme_Current() { return g_theme; }
UiThemeMode UiTheme_GetMode() { return g_theme.mode; }

void UiTheme_Apply(UiThemeMode mode) {
    UiTheme& t = g_theme;
    t.mode = mode;

    if (mode == UiThemeMode::Dark) {
        // 产品向暗色：黑底 + 浅灰强调（非 MD 默认蓝紫）
        t.surface = V(0.07f, 0.07f, 0.08f);
        t.surface_variant = V(0.14f, 0.14f, 0.16f);
        t.on_surface = V(0.96f, 0.96f, 0.97f);
        t.on_surface_var = V(0.68f, 0.69f, 0.72f);
        t.outline = V(0.32f, 0.33f, 0.36f);

        t.primary = V(0.93f, 0.93f, 0.95f);
        t.on_primary = V(0.08f, 0.08f, 0.09f);
        t.primary_container = V(0.22f, 0.23f, 0.26f);
        t.on_primary_container = V(0.95f, 0.95f, 0.97f);
        t.secondary_container = V(0.18f, 0.18f, 0.20f);
        t.on_secondary_container = V(0.90f, 0.90f, 0.92f);

        t.island_rim = V(0.0f, 0.0f, 0.0f, 0.95f);
        t.island_fill = V(0.0f, 0.0f, 0.0f, 0.22f);
        t.island_glass = V(0.95f, 0.95f, 0.98f, 0.09f);
        t.island_text = V(0.98f, 0.98f, 1.0f, 0.98f);
        t.island_text_shadow = V(0.0f, 0.0f, 0.0f, 0.45f);
        t.island_glow = V(0.0f, 0.0f, 0.0f, 1.0f);
        t.island_glow_soft = V(0.0f, 0.0f, 0.0f, 0.50f);

        t.droplet = V(0.0f, 0.0f, 0.0f, 0.96f);
        t.droplet_glow = V(0.0f, 0.0f, 0.0f, 1.0f);

        t.panel_bg = V(0.0f, 0.0f, 0.0f, 0.98f);
        t.panel_border = V(0.22f, 0.23f, 0.26f, 1.0f);
        t.panel_glow = V(0.0f, 0.0f, 0.0f, 1.0f);
        t.sidebar_bg = V(0.04f, 0.04f, 0.05f, 1.0f);
        // Tab：选中更深色块
        t.tab_selected_bg = V(0.20f, 0.20f, 0.22f, 1.0f);
        t.tab_unselected_bg = V(0.10f, 0.10f, 0.11f, 1.0f);
        t.tab_text = V(0.70f, 0.71f, 0.74f, 1.0f);
        t.tab_text_selected = V(0.98f, 0.98f, 0.99f, 1.0f);
        t.divider = V(1.0f, 1.0f, 1.0f, 0.10f);

        t.field_bg = V(0.12f, 0.12f, 0.14f, 1.0f);
        t.field_bg_focus = V(0.16f, 0.16f, 0.18f, 1.0f);
        t.field_border = V(0.36f, 0.37f, 0.40f, 1.0f);
        t.field_border_focus = V(0.90f, 0.90f, 0.92f, 1.0f);
        t.field_text = V(0.96f, 0.96f, 0.97f, 1.0f);
        t.field_hint = V(0.55f, 0.56f, 0.60f, 1.0f);

        t.switch_track_off = V(0.28f, 0.29f, 0.32f, 1.0f);
        t.switch_track_on = V(0.90f, 0.90f, 0.92f, 1.0f);
        t.switch_thumb = V(0.10f, 0.10f, 0.11f, 1.0f);      // on: 深色拇指
        t.switch_thumb_off = V(0.88f, 0.88f, 0.90f, 1.0f);  // off: 浅色拇指

        t.slider_track = V(0.30f, 0.31f, 0.34f, 1.0f);
        t.slider_active = V(0.92f, 0.92f, 0.94f, 1.0f);
        t.slider_thumb = V(0.96f, 0.96f, 0.97f, 1.0f);

        t.btn_press_overlay = V(0.0f, 0.0f, 0.0f, 0.12f);
        t.ripple = V(1.0f, 1.0f, 1.0f, 0.14f);
    } else {
        // 产品向亮色：白底 + 深炭强调 + 白光晕
        t.surface = V(1.0f, 1.0f, 1.0f);
        t.surface_variant = V(0.94f, 0.94f, 0.96f);
        t.on_surface = V(0.10f, 0.10f, 0.12f);
        t.on_surface_var = V(0.40f, 0.41f, 0.44f);
        t.outline = V(0.78f, 0.79f, 0.82f);

        t.primary = V(0.12f, 0.12f, 0.14f);
        t.on_primary = V(1.0f, 1.0f, 1.0f);
        t.primary_container = V(0.90f, 0.90f, 0.92f);
        t.on_primary_container = V(0.12f, 0.12f, 0.14f);
        t.secondary_container = V(0.93f, 0.93f, 0.95f);
        t.on_secondary_container = V(0.16f, 0.16f, 0.18f);

        t.island_rim = V(0.86f, 0.87f, 0.90f, 0.95f);
        t.island_fill = V(1.0f, 1.0f, 1.0f, 0.82f);
        t.island_glass = V(1.0f, 1.0f, 1.0f, 0.60f);
        t.island_text = V(0.10f, 0.10f, 0.12f, 0.98f);
        t.island_text_shadow = V(1.0f, 1.0f, 1.0f, 0.40f);
        t.island_glow = V(1.0f, 1.0f, 1.0f, 0.95f);
        t.island_glow_soft = V(0.85f, 0.86f, 0.90f, 0.55f);

        t.droplet = V(0.96f, 0.96f, 0.98f, 0.96f);
        t.droplet_glow = V(1.0f, 1.0f, 1.0f, 0.80f);

        t.panel_bg = V(1.0f, 1.0f, 1.0f, 0.99f);
        t.panel_border = V(0.86f, 0.87f, 0.90f, 1.0f);
        t.panel_glow = V(1.0f, 1.0f, 1.0f, 1.0f);
        t.sidebar_bg = V(0.96f, 0.96f, 0.97f, 1.0f);
        // Tab：选中深色块 + 白字
        t.tab_selected_bg = V(0.14f, 0.14f, 0.16f, 1.0f);
        t.tab_unselected_bg = V(0.92f, 0.92f, 0.94f, 1.0f);
        t.tab_text = V(0.42f, 0.43f, 0.46f, 1.0f);
        t.tab_text_selected = V(0.98f, 0.98f, 0.99f, 1.0f);
        t.divider = V(0.0f, 0.0f, 0.0f, 0.08f);

        t.field_bg = V(0.95f, 0.95f, 0.97f, 1.0f);
        t.field_bg_focus = V(0.93f, 0.93f, 0.95f, 1.0f);
        t.field_border = V(0.74f, 0.75f, 0.78f, 1.0f);
        t.field_border_focus = V(0.14f, 0.14f, 0.16f, 1.0f);
        t.field_text = V(0.10f, 0.10f, 0.12f, 1.0f);
        t.field_hint = V(0.52f, 0.53f, 0.56f, 1.0f);

        t.switch_track_off = V(0.80f, 0.81f, 0.84f, 1.0f);
        t.switch_track_on = V(0.14f, 0.14f, 0.16f, 1.0f);
        t.switch_thumb = V(1.0f, 1.0f, 1.0f, 1.0f);
        t.switch_thumb_off = V(0.98f, 0.98f, 0.99f, 1.0f);

        t.slider_track = V(0.82f, 0.83f, 0.86f, 1.0f);
        t.slider_active = V(0.14f, 0.14f, 0.16f, 1.0f);
        t.slider_thumb = V(0.14f, 0.14f, 0.16f, 1.0f);

        t.btn_press_overlay = V(1.0f, 1.0f, 1.0f, 0.12f);
        t.ripple = V(0.0f, 0.0f, 0.0f, 0.10f);
    }
}


static UiTheme g_theme_a; // dark snapshot
static UiTheme g_theme_b; // light snapshot
static float g_theme_t = 1.f; // 0=dark target path during anim, 1=settled
static float g_theme_anim = 1.f; // 0..1 progress of current transition
static UiThemeMode g_theme_from = UiThemeMode::Dark;
static UiThemeMode g_theme_to = UiThemeMode::Dark;
static bool g_theme_animating = false;

static ImVec4 Lerp4(const ImVec4& a, const ImVec4& b, float t) {
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
                  a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}

static void CopyTheme(UiTheme& dst, const UiTheme& src) { dst = src; }

static void BuildPalette(UiTheme& t, UiThemeMode mode) {
    // reuse Apply body by temp swap
    UiTheme backup = g_theme;
    UiTheme_Apply(mode);
    t = g_theme;
    g_theme = backup;
    t.mode = mode;
}

// smoothstep
static float Ease(float x) {
    if (x < 0.f) x = 0.f; if (x > 1.f) x = 1.f;
    return x * x * (3.f - 2.f * x);
}

static void BlendTheme(UiTheme& out, const UiTheme& a, const UiTheme& b, float t) {
    t = Ease(t);
#define B(f) out.f = Lerp4(a.f, b.f, t)
    B(surface); B(surface_variant); B(on_surface); B(on_surface_var); B(outline);
    B(primary); B(on_primary); B(primary_container); B(on_primary_container);
    B(secondary_container); B(on_secondary_container);
    B(island_rim); B(island_fill); B(island_glass); B(island_text); B(island_text_shadow);
    B(island_glow); B(island_glow_soft);
    B(droplet); B(droplet_glow);
    B(panel_bg); B(panel_border); B(panel_glow);
    B(sidebar_bg);
    B(tab_selected_bg); B(tab_unselected_bg); B(tab_text); B(tab_text_selected);
    B(divider);
    B(field_bg); B(field_bg_focus); B(field_border); B(field_border_focus); B(field_text); B(field_hint);
    B(switch_track_off); B(switch_track_on); B(switch_thumb); B(switch_thumb_off);
    B(slider_track); B(slider_active); B(slider_thumb);
    B(btn_press_overlay); B(ripple);
#undef B
    out.mode = (t >= 0.5f) ? b.mode : a.mode;
}

void UiTheme_SetMode(UiThemeMode mode) {
    // 立即应用（无动画）
    UiTheme_Apply(mode);
    g_theme_animating = false;
    g_theme_anim = 1.f;
    g_theme_to = mode;
    g_theme_from = mode;
}

void UiTheme_Toggle() {
    UiThemeMode cur = g_theme_animating ? g_theme_to : g_theme.mode;
    UiThemeMode next = (cur == UiThemeMode::Dark) ? UiThemeMode::Light : UiThemeMode::Dark;
    // 快照起止色板
    BuildPalette(g_theme_a, cur);
    BuildPalette(g_theme_b, next);
    g_theme_from = cur;
    g_theme_to = next;
    g_theme_anim = 0.f;
    g_theme_animating = true;
    // 先保持当前画面，Tick 里混合
}

void UiTheme_Tick(float dt) {
    if (!g_theme_animating) return;
    g_theme_anim += dt * 3.2f; // ~0.3s
    if (g_theme_anim >= 1.f) {
        g_theme_anim = 1.f;
        g_theme_animating = false;
        UiTheme_Apply(g_theme_to);
        return;
    }
    BlendTheme(g_theme, g_theme_a, g_theme_b, g_theme_anim);
    g_theme.mode = g_theme_to; // 切换过程中逻辑模式跟目标
}

bool UiTheme_IsAnimating() { return g_theme_animating; }

ImU32 UiCol(const ImVec4& c, float alpha_mul) {
    return ImGui::ColorConvertFloat4ToU32(ImVec4(c.x, c.y, c.z, c.w * alpha_mul));
}
struct UiThemeInit { UiThemeInit() { UiTheme_Apply(UiThemeMode::Dark); } };
static UiThemeInit g_ui_theme_init;