#include "draw.h"
#include "My_font/zh_Font.h"
#include "My_font/fontawesome-brands.h"
#include "My_font/fontawesome-regular.h"
#include "My_font/fontawesome-solid.h"
#include "My_font/gui_icon.h"
#include "全局变量.h"
#include "sys读写.h"
#include "game/game_esp.h"

#include "ui/ui_theme.h"
#include "ui/ui_anim.h"
#include "ui/ui_island.h"
#include "ui/ui_panel.h"
#include "ui/ui_md3.h"

// 人物图标（工作区根目录 bohe.h，通过 Android.mk 加入 include）
#include "bohe.h"

#include <ctime>
#include <cstdio>
#include <cmath>

bool permeate_record = false;
bool permeate_record_ini = false;
struct Last_ImRect LastCoordinate = {0, 0, 0, 0};

std::unique_ptr<AndroidImgui> graphics;
ANativeWindow *window = NULL;
android::ANativeWindowCreator::DisplayInfo displayInfo;
ImGuiWindow *g_window = NULL;
int abs_ScreenX = 0, abs_ScreenY = 0;
int native_window_screen_x = 0, native_window_screen_y = 0;

ImFont* zh_font = NULL;
TextureInfo Aekun_image = {0, 0, 0};
TextureInfo g_bohe_avatar = {0, 0, 0};

static float g_logic_w = 0.0f;
static float g_logic_h = 0.0f;
static float g_user_panel_scale = 1.0f; // 用户拖右下角缩放

static void FormatLocalTimeHMS(char* out, size_t out_size) {
    std::time_t now = std::time(nullptr);
    std::tm local_tm{};
#if defined(_WIN32)
    localtime_s(&local_tm, &now);
#else
    localtime_r(&now, &local_tm);
#endif
    std::snprintf(out, out_size, "%02d:%02d:%02d",
                  local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
}

static void UpdateLogicScreenSize() {
    if (displayInfo.width > 0 && displayInfo.height > 0) {
        g_logic_w = (float)displayInfo.width;
        g_logic_h = (float)displayInfo.height;
    } else {
        ImGuiIO& io = ImGui::GetIO();
        g_logic_w = io.DisplaySize.x;
        g_logic_h = io.DisplaySize.y;
    }
}

static ImFont* GetUiFont(ImGuiIO& io) {
    if (zh_font != nullptr) return zh_font;
    if (io.FontDefault != nullptr) return io.FontDefault;
    if (io.Fonts != nullptr && !io.Fonts->Fonts.empty()) return io.Fonts->Fonts[0];
    return nullptr;
}

static bool PointInRect(const ImVec2& p, const ImVec2& min, const ImVec2& max) {
    return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y;
}

static float Clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static void EnsureBoheTexture() {
    if (g_bohe_avatar.DS != 0) return;
    if (!graphics) return;
    g_bohe_avatar = graphics->LoadTextureFromMemory((void*)bohe_data, (int)sizeof(bohe_data));
}


bool M_Android_LoadFont(float SizePixels) {
    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    config.SizePixels = SizePixels;
    config.OversampleH = 1;
    ::zh_font = io.Fonts->AddFontFromMemoryTTF((void *)OPPOSans_H, OPPOSans_H_size, 0.0f, &config, io.Fonts->GetGlyphRangesChineseFull());
    io.Fonts->AddFontDefault();
    return zh_font != nullptr;
}

void init_My_drawdata() {
    M_Android_LoadFont(48.0f);
    ImGui::GetStyle().ScaleAllSizes(2.0f);
    UiTheme_Apply(UiTheme_GetMode());
    g_bohe_avatar = {0, 0, 0};
    EnsureBoheTexture();
}


void screen_config() {
    ::displayInfo = android::ANativeWindowCreator::GetDisplayInfo();
}

void drawBegin() {
    if (::permeate_record_ini) {
        if (::g_window != NULL) {
            LastCoordinate.Pos_x = ::g_window->Pos.x;
            LastCoordinate.Pos_y = ::g_window->Pos.y;
            LastCoordinate.Size_x = ::g_window->Size.x;
            LastCoordinate.Size_y = ::g_window->Size.y;
        }
        graphics->Shutdown();
        android::ANativeWindowCreator::Destroy(::window);
        ::window = android::ANativeWindowCreator::Create("simple_window", native_window_screen_x, native_window_screen_y, permeate_record);
        graphics->Init_Render(::window, native_window_screen_x, native_window_screen_y);
        ::init_My_drawdata();
        permeate_record_ini = false;
    }

    static uint32_t orientation = -1;
    screen_config();
    UpdateLogicScreenSize();
    if (orientation != (uint32_t)displayInfo.orientation) {
        orientation = (uint32_t)displayInfo.orientation;
        Touch::setOrientation(displayInfo.orientation);
    }
}

void Layout_tick_UI(bool *main_thread_flag) {
    (void)main_thread_flag;

    UpdateLogicScreenSize();
    if (g_logic_w <= 1.0f || g_logic_h <= 1.0f) return;

    // 游戏 ESP：驱动读写 + 背景绘制
    GameEsp::TickDraw(g_logic_w, g_logic_h);

    EnsureBoheTexture();

    ImGuiIO& io = ImGui::GetIO();
    ImFont* font = GetUiFont(io);

    float dt = io.DeltaTime > 0.0f ? io.DeltaTime : (1.0f / 60.0f);
    UiAnim_Tick(dt);
    UiTheme_Tick(dt);
    Md3::BeginFrame(dt);

    const float short_side = ImMin(g_logic_w, g_logic_h);
    const float ui_scale = short_side / 1080.0f;
    const bool is_landscape = g_logic_w > g_logic_h;

    char time_text[16] = {};
    FormatLocalTimeHMS(time_text, sizeof(time_text));

    // 灵动岛文案：开启「玩家数量」时做数字跳动，最终显示人数；否则显示时间
    static bool s_prev_count_on = false;
    static int s_last_target = -1;
    static float s_count_anim = 0.f; // 0..1 跳动阶段
    static int s_display_num = 0;

    const bool count_on = GameEsp::R().s.count && GameEsp::R().inited;
    const int target_count = GameEsp::R().player_count;
    if (count_on) {
        if (!s_prev_count_on || target_count != s_last_target) {
            s_count_anim = 0.f; // 重新触发跳动
            s_last_target = target_count;
        }
        s_prev_count_on = true;
        s_count_anim += dt * 2.6f; // ~0.38s 跳完
        if (s_count_anim < 1.f) {
            // 快速乱跳：用时间扰动 + 逼近目标
            float k = s_count_anim;
            int span = (target_count < 8) ? 12 : (target_count + 15);
            int noise = (int)(fmodf(ImGui::GetTime() * 37.0f, (float)(span > 0 ? span : 1)));
            int blend = (int)((1.f - k) * (float)noise + k * (float)target_count);
            s_display_num = blend;
            if (s_display_num < 0) s_display_num = 0;
        } else {
            s_display_num = target_count;
            s_count_anim = 1.f;
        }
    } else {
        s_prev_count_on = false;
        s_count_anim = 0.f;
        s_last_target = -1;
    }

    char island_text[32] = {};
    if (count_on) {
        // 跳动阶段可显示纯数字；稳定后 "N人"
        if (s_count_anim < 1.f) snprintf(island_text, sizeof(island_text), "%d", s_display_num);
        else snprintf(island_text, sizeof(island_text), "%d人", s_display_num);
    } else {
        snprintf(island_text, sizeof(island_text), "%s", time_text);
    }

    float island_h = short_side * (is_landscape ? 0.062f : 0.065f);
    island_h = Clampf(island_h, 44.0f * ui_scale, 64.0f * ui_scale);
    // 人数模式略放大字重观感
    const float font_size = Clampf(island_h * (count_on ? 0.56f : 0.52f), 18.0f, 34.0f);

    ImVec2 text_size(0, 0);
    if (font != nullptr) text_size = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, island_text);
    else text_size = ImVec2(island_h * 3.2f, island_h * 0.55f);

    float island_w = text_size.x + island_h * 0.82f * 2.0f;
    // 跳动时略微拉宽胶囊，增加「旁边有动画」的体感
    if (count_on && s_count_anim < 1.f) {
        float pulse = 1.f + 0.06f * sinf(s_count_anim * 3.1415926f * 4.f);
        island_w *= pulse;
    }
    island_w = Clampf(island_w, island_h * 2.75f, short_side * (is_landscape ? 0.30f : 0.36f));

    const float top_margin = short_side * (is_landscape ? 0.030f : 0.038f);
    const ImVec2 island_min((g_logic_w - island_w) * 0.5f, top_margin);
    const ImVec2 island_max(island_min.x + island_w, island_min.y + island_h);

    float base_w = short_side * (is_landscape ? 0.88f : 0.92f);
    float base_h = short_side * (is_landscape ? 0.70f : 0.78f);
    base_w = Clampf(base_w, 520.0f * ui_scale, g_logic_w * 0.94f);
    base_h = Clampf(base_h, 560.0f * ui_scale, g_logic_h * 0.86f);
    float panel_w = base_w * g_user_panel_scale;
    float panel_h = base_h * g_user_panel_scale;
    panel_w = Clampf(panel_w, 420.0f * ui_scale, g_logic_w * 0.98f);
    panel_h = Clampf(panel_h, 480.0f * ui_scale, g_logic_h * 0.94f);
    // 以中心缩放，保持居中
    ImVec2 panel_min((g_logic_w - panel_w) * 0.5f, (g_logic_h - panel_h) * 0.5f);
    ImVec2 panel_max(panel_min.x + panel_w, panel_min.y + panel_h);

    float morph_t = 0.0f;
    const IslandAnimMode mode = UiAnim_Mode();
    if (mode == IslandAnimMode::Collapsed) morph_t = 0.0f;
    else if (mode == IslandAnimMode::Expanded) morph_t = 1.0f;
    else if (mode == IslandAnimMode::Expanding) morph_t = UiAnim_T();
    else morph_t = 1.0f - UiAnim_T();

    MorphGeom morph = UiAnim_EvalMorph(morph_t, island_min, island_max, panel_min, panel_max);

    const bool light = (UiTheme_GetMode() == UiThemeMode::Light);
    const float glow_pad = (light ? 40.0f : 28.0f) * ui_scale + 14.0f;
    ImVec2 host_min, host_max;
    if (mode == IslandAnimMode::Collapsed) {
        host_min = ImVec2(morph.min.x - glow_pad, morph.min.y - glow_pad);
        host_max = ImVec2(morph.max.x + glow_pad, morph.max.y + glow_pad);
    } else {
        host_min = ImVec2(
            ImMin(island_min.x, ImMin(panel_min.x, morph.min.x)) - glow_pad,
            ImMin(island_min.y, ImMin(panel_min.y, morph.min.y)) - glow_pad
        );
        host_max = ImVec2(
            ImMax(island_max.x, ImMax(panel_max.x, morph.max.x)) + glow_pad,
            ImMax(island_max.y, ImMax(panel_max.y, morph.max.y)) + glow_pad
        );
    }

    ImGui::SetNextWindowPos(host_min, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(host_max.x - host_min.x, host_max.y - host_min.y), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);

    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoNav;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

    if (ImGui::Begin("##DynamicIsland", nullptr, flags)) {
        ImDrawList* dl = ImGui::GetWindowDrawList();

        if (morph.form == MorphForm::Panel) {
            bool resize_active = false;
            // 动画中 content_a 降低：内部自适应不画错位控件
            UiPanel_Draw(dl, morph.min, morph.max, morph.rounding, font,
                         morph.content_a * morph.alpha, &g_bohe_avatar, &resize_active);
            // 仅完全展开时可缩放
            if (mode == IslandAnimMode::Expanded && resize_active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                ImVec2 dlt = io.MouseDelta;
                float k = 0.0018f;
                g_user_panel_scale = Clampf(g_user_panel_scale + (dlt.x + dlt.y) * k, 0.80f, 1.30f);
            }
        } else if (morph.form == MorphForm::Droplet) {
            const ImVec2 c((morph.min.x + morph.max.x) * 0.5f, (morph.min.y + morph.max.y) * 0.5f);
            const float rr = ImMin(morph.max.x - morph.min.x, morph.max.y - morph.min.y) * 0.5f;
            UiIsland_DrawDroplet(dl, c, rr, morph.drip, morph.alpha);
        } else {
            const float cur_h = morph.max.y - morph.min.y;
            const float fs = Clampf(cur_h * 0.52f, 12.0f, font_size);
            UiIsland_DrawCapsule(dl, morph.min, morph.max, morph.rounding, font, island_text, fs, morph.alpha, morph.island_a);
        }

        // 缩放拖动中禁止收起；动画中禁止切换
        static bool s_was_resizing = false;
        if (morph.form == MorphForm::Panel) {
            // resize_active 在上面作用域外，用 static 边沿
        }
        if (!UiAnim_IsAnimating() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            const ImVec2 mp = io.MousePos;
            if (mode == IslandAnimMode::Collapsed) {
                if (PointInRect(mp, morph.min, morph.max)) UiAnim_RequestExpand();
            } else if (mode == IslandAnimMode::Expanded) {
                // 右下角手柄区域点击不收起
                const float hs = 56.f;
                bool on_resize = (mp.x >= morph.max.x - hs - 4.f && mp.y >= morph.max.y - hs - 4.f
                                  && mp.x <= morph.max.x && mp.y <= morph.max.y);
                if (!on_resize && !PointInRect(mp, morph.min, morph.max) && !ImGui::IsAnyItemActive())
                    UiAnim_RequestCollapse();
            }
        }

        g_window = ImGui::GetCurrentWindow();
    }
    ImGui::End();
    ImGui::PopStyleVar(3);
}