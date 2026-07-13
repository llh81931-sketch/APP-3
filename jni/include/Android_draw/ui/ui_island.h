#pragma once
#include "ImGui/imgui.h"

void UiIsland_DrawGlow(ImDrawList* dl, const ImVec2& min, const ImVec2& max, float rounding, float intensity = 1.0f);

void UiIsland_DrawCapsule(
    ImDrawList* dl,
    const ImVec2& min, const ImVec2& max, float rounding,
    ImFont* font, const char* time_text, float font_size,
    float alpha, float text_alpha
);

void UiIsland_DrawDroplet(
    ImDrawList* dl, const ImVec2& center, float radius, float drip, float intensity
);