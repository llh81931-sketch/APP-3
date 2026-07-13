#include "ui/ui_island.h"
#include "ui/ui_theme.h"
#include <cmath>

static float Clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
static float FMin(float a, float b) { return a < b ? a : b; }
static float FMax(float a, float b) { return a > b ? a : b; }

void UiIsland_DrawGlow(ImDrawList* dl, const ImVec2& min, const ImVec2& max, float rounding, float intensity) {
    const UiTheme& t = UiTheme_Current();
    intensity = Clampf(intensity, 0.0f, 1.5f);
    // 暗=黑晕 亮=白晕（主题反转）
    for (int i = 9; i >= 1; --i) {
        const float k = (float)i / 9.0f;
        const float pad = 1.0f + k * 16.0f;
        dl->AddRectFilled(
            ImVec2(min.x - pad, min.y - pad), ImVec2(max.x + pad, max.y + pad),
            UiCol(t.island_glow, 0.085f * (1.0f - k * 0.70f) * intensity),
            rounding + pad * 0.92f
        );
    }
    for (int i = 0; i < 3; ++i) {
        const float expand = 0.5f + (float)i * 1.1f;
        dl->AddRect(
            ImVec2(min.x - expand, min.y - expand), ImVec2(max.x + expand, max.y + expand),
            UiCol(t.island_glow_soft, (0.45f - (float)i * 0.10f) * intensity),
            rounding + expand * 0.8f, 0, 1.4f + (float)i * 0.35f
        );
    }
}

static void DrawInnerGlass(ImDrawList* dl, const ImVec2& outer_min, const ImVec2& outer_max, float outer_rounding, float alpha_mul) {
    const UiTheme& t = UiTheme_Current();
    const float h = outer_max.y - outer_min.y;
    const float inset = Clampf(h * 0.14f, 5.0f, 10.0f);
    const ImVec2 min(outer_min.x + inset, outer_min.y + inset);
    const ImVec2 max(outer_max.x - inset, outer_max.y - inset);
    const float rounding = FMax(2.0f, outer_rounding - inset);
    alpha_mul = Clampf(alpha_mul, 0.0f, 1.0f);

    dl->AddRectFilled(min, max, UiCol(t.island_glass, alpha_mul), rounding);

    for (int i = 0; i < 3; ++i) {
        const float k = (float)i / 2.0f;
        const float pad = inset * (0.08f + k * 0.16f);
        ImVec4 frost = t.island_glass;
        frost.w = 0.06f * (1.0f - k) * (1.0f - k);
        dl->AddRect(
            ImVec2(min.x + pad, min.y + pad), ImVec2(max.x - pad, max.y - pad),
            UiCol(frost, alpha_mul), FMax(1.0f, rounding - pad), 0, 1.5f
        );
    }

    const float hi_h = Clampf(h * 0.22f, 5.0f, 11.0f);
    const float side = FMax(2.0f, (max.x - min.x) * 0.06f);
    const bool light = (t.mode == UiThemeMode::Light);
    dl->AddRectFilled(
        ImVec2(min.x + side, min.y + 1.2f),
        ImVec2(max.x - side, min.y + 1.2f + hi_h),
        UiCol(ImVec4(1, 1, 1, light ? 0.35f : 0.065f), alpha_mul),
        FMin(rounding * 0.9f, hi_h * 0.5f)
    );

    dl->AddRect(min, max,
        UiCol(light ? ImVec4(0.7f, 0.75f, 0.85f, 0.35f) : ImVec4(0.90f, 0.94f, 1.0f, 0.16f), alpha_mul),
        rounding, 0, 1.15f);
}

void UiIsland_DrawCapsule(
    ImDrawList* dl,
    const ImVec2& min, const ImVec2& max, float rounding,
    ImFont* font, const char* time_text, float font_size,
    float alpha, float text_alpha
) {
    const UiTheme& t = UiTheme_Current();
    alpha = Clampf(alpha, 0.0f, 1.0f);
    text_alpha = Clampf(text_alpha, 0.0f, 1.0f);
    const float h = max.y - min.y;
    const float rim = Clampf(h * 0.14f, 5.0f, 10.0f);

    UiIsland_DrawGlow(dl, min, max, rounding, alpha);

    dl->AddRectFilled(min, max, UiCol(t.island_fill, alpha), rounding);
    for (int i = 0; i < 4; ++i) {
        const float k = (float)i / 3.0f;
        const float expand = rim * (0.12f + k * 0.65f);
        dl->AddRect(
            ImVec2(min.x + expand * 0.12f, min.y + expand * 0.12f),
            ImVec2(max.x - expand * 0.12f, max.y - expand * 0.12f),
            UiCol(t.island_rim, (0.90f - k * 0.50f) * alpha),
            FMax(1.0f, rounding - expand * 0.10f), 0, 1.8f + (1.0f - k) * 1.2f
        );
    }
    dl->AddRect(min, max, UiCol(t.island_rim, alpha), rounding, 0, 1.5f);
    DrawInnerGlass(dl, min, max, rounding, alpha);

    if (font && time_text && text_alpha > 0.05f) {
        const float cur_h = max.y - min.y;
        const float fs = Clampf(font_size > 1.0f ? font_size : cur_h * 0.52f, 12.0f, 32.0f);
        ImVec2 ts = font->CalcTextSizeA(fs, FLT_MAX, 0.0f, time_text);
        if (ts.x < (max.x - min.x) * 0.90f) {
            const ImVec2 tp(
                min.x + (max.x - min.x - ts.x) * 0.5f,
                min.y + (max.y - min.y - ts.y) * 0.5f - 0.5f
            );
            dl->AddText(font, fs, ImVec2(tp.x + 0.8f, tp.y + 1.0f),
                        UiCol(t.island_text_shadow, text_alpha * alpha), time_text);
            dl->AddText(font, fs, tp, UiCol(t.island_text, text_alpha * alpha), time_text);
        }
    }
}

void UiIsland_DrawDroplet(ImDrawList* dl, const ImVec2& center, float radius, float drip, float intensity) {
    const UiTheme& t = UiTheme_Current();
    drip = Clampf(drip, 0.0f, 1.0f);
    intensity = Clampf(intensity, 0.0f, 1.5f);
    const float rx = radius * (1.0f - drip * 0.08f);
    const float ry = radius * (1.0f + drip * 0.32f);
    const ImVec2 c(center.x, center.y + radius * drip * 0.10f);

    for (int i = 5; i >= 1; --i) {
        const float k = (float)i / 5.0f;
        const float pad = 1.0f + k * radius * 1.05f;
        dl->AddEllipseFilled(c, ImVec2(rx + pad, ry + pad),
            UiCol(t.droplet_glow, 0.10f * (1.0f - k * 0.75f) * intensity), 0, 0);
    }
    dl->AddEllipseFilled(c, ImVec2(rx, ry), UiCol(t.droplet, intensity), 0, 0);
    // 高光
    const bool light = (t.mode == UiThemeMode::Light);
    dl->AddEllipseFilled(
        ImVec2(c.x - rx * 0.18f, c.y - ry * 0.28f),
        ImVec2(rx * 0.28f, ry * 0.18f),
        UiCol(ImVec4(1, 1, 1, light ? 0.45f : 0.12f), intensity), 0, 0
    );
}