#pragma once
#include "ImGui/imgui.h"
#include "ui/ui_theme.h"

// MD3 风格大控件；滚动条仅内容溢出时显示

namespace Md3 {

struct Metrics {
    // 控件偏大，菜单本身不放大
    float switch_h = 58.0f;
    float switch_w = 104.0f;
    float field_h = 72.0f;
    float btn_h = 72.0f;
    float slider_h = 72.0f;
    float row_h = 100.0f;
    float tab_item_h = 92.0f;
    float title_fs = 40.0f;
    float body_fs = 28.0f;
    float label_fs = 24.0f;
    float hint_fs = 20.0f;
};
const Metrics& M();

void BeginFrame(float dt);
void EndFrameOverlays(ImDrawList* dl, ImFont* font);

bool Button(ImDrawList* dl, const char* id, const char* label,
            const ImVec2& min, const ImVec2& max, ImFont* font, bool filled = true);

bool Switch(ImDrawList* dl, const char* id, bool* value,
            const ImVec2& pos, float height = 0.0f);

// MD3 Expressive Slider：粗圆角轨道 + 竖条拇指（参考官方截图）
bool Slider(ImDrawList* dl, const char* id, float* value,
            float v_min, float v_max,
            const ImVec2& track_min, const ImVec2& track_max,
            ImFont* font = nullptr, bool show_value = true);

bool TextTabList(ImDrawList* dl, const char* id, int* selected,
                 const char* const* labels, int count,
                 const ImVec2& min, const ImVec2& max, ImFont* font);

bool Dropdown(ImDrawList* dl, const char* id, int* selected,
              const char* const* items, int count,
              const ImVec2& min, const ImVec2& max, ImFont* font);

float ScrollArea(ImDrawList* dl, const char* id, const ImVec2& min, const ImVec2& max,
                 float content_height, float& scroll_io);
void ScrollAreaEnd(ImDrawList* dl);

void ListRow(ImDrawList* dl, const ImVec2& min, const ImVec2& max, ImFont* font,
             const char* title, const char* subtitle = nullptr);
void Divider(ImDrawList* dl, float x0, float x1, float y, float alpha = 1.0f);
void Card(ImDrawList* dl, const ImVec2& min, const ImVec2& max, float rounding);
void Text(ImDrawList* dl, ImFont* font, float size, const ImVec2& pos, const ImVec4& col, const char* text);

} // namespace Md3