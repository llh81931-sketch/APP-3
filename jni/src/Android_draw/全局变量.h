// 兼容旧全局名：真实开关在 GameEsp::R().s
// 保留空壳避免旧引用炸编译
#pragma once
#include "game/game_esp.h"

// 分辨率（由 draw 层按需写入）
inline float& 屏幕宽度() { static float v=0; return v; }
inline float& 屏幕高度() { static float v=0; return v; }
