#pragma once
#include "ImGui/imgui.h"

enum class UiThemeMode : int { Dark = 0, Light = 1 };

struct UiTheme {
    UiThemeMode mode = UiThemeMode::Dark;

    ImVec4 surface, surface_variant, on_surface, on_surface_var, outline;
    ImVec4 primary, on_primary, primary_container, on_primary_container;
    ImVec4 secondary_container, on_secondary_container;

    ImVec4 island_rim, island_fill, island_glass, island_text, island_text_shadow;
    ImVec4 island_glow, island_glow_soft;
    ImVec4 droplet, droplet_glow;

    ImVec4 panel_bg, panel_border, panel_glow;
    ImVec4 sidebar_bg;
    ImVec4 tab_selected_bg, tab_unselected_bg, tab_text, tab_text_selected;
    ImVec4 divider;

    ImVec4 field_bg, field_bg_focus, field_border, field_border_focus, field_text, field_hint;
    ImVec4 switch_track_off, switch_track_on, switch_thumb, switch_thumb_off;
    ImVec4 slider_track, slider_active, slider_thumb;
    ImVec4 btn_press_overlay, ripple;
};

UiTheme& UiTheme_Current();
UiThemeMode UiTheme_GetMode();
void UiTheme_SetMode(UiThemeMode mode);
void UiTheme_Toggle();
void UiTheme_Apply(UiThemeMode mode);
// 主题交互动画：每帧推进混合
void UiTheme_Tick(float dt);
bool UiTheme_IsAnimating();
ImU32 UiCol(const ImVec4& c, float alpha_mul = 1.0f);