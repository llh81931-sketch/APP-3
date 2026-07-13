#pragma once
#include "ImGui/imgui.h"
#include "AndroidImgui.h"

// avatar_tex.DS == 0 时不画头像
// out_resize_active: 右下角缩放手柄是否被拖动
void UiPanel_Draw(
    ImDrawList* dl,
    const ImVec2& min,
    const ImVec2& max,
    float rounding,
    ImFont* font,
    float content_alpha,
    const TextureInfo* avatar_tex = nullptr,
    bool* out_resize_active = nullptr
);
