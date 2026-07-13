#pragma once
#include "ImGui/imgui.h"

// 更快动画：下落约 0.55s + 展开约 0.45s，总约 1.0s
// 落点不必在屏幕正中，偏上约 35% 高度即可

enum class IslandAnimMode : int {
    Collapsed = 0,
    Expanding,
    Expanded,
    Collapsing
};

enum class MorphForm : int {
    Island = 0,
    Droplet,
    Panel
};

struct MorphGeom {
    MorphForm form = MorphForm::Island;
    ImVec2 min{};
    ImVec2 max{};
    float rounding = 0.0f;
    float drip = 0.0f;
    float content_a = 0.0f;
    float island_a = 1.0f;
    float alpha = 1.0f;
};

IslandAnimMode& UiAnim_Mode();
float& UiAnim_T();
void UiAnim_Tick(float dt);
void UiAnim_RequestExpand();
void UiAnim_RequestCollapse();
bool UiAnim_IsAnimating();

MorphGeom UiAnim_EvalMorph(
    float t_expand,
    const ImVec2& island_min, const ImVec2& island_max,
    const ImVec2& panel_min, const ImVec2& panel_max
);

float UiAnim_Clamp(float v, float lo, float hi);
float UiAnim_Lerp(float a, float b, float t);
ImVec2 UiAnim_Lerp2(const ImVec2& a, const ImVec2& b, float t);
float UiAnim_Smooth01(float t);
float UiAnim_SpringSoft(float t);