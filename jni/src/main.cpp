#include "draw.h"
#include "AndroidImgui.h"
#include "GraphicsManager.h"
#include "sys读写.h"
#include "game/game_esp.h"

#include <cstdio>
#include <iostream>

// 0=退出 1=TWT 2=RT 3=仅UI(无驱动)
// 返回: true 继续启动UI；false 退出
// 输出 out_ui_only: 是否无驱动模式
static bool Terminal_SelectAndConnectDriver(bool* out_ui_only) {
    using std::cout;
    using std::cin;
    using std::endl;

    if (out_ui_only) *out_ui_only = false;

    cout << "========================================" << endl;
    cout << "  91UI · 启动模式选择" << endl;
    cout << "========================================" << endl;
    cout << "  1) TWT 驱动" << endl;
    cout << "  2) RT  驱动" << endl;
    cout << "  3) 仅 UI（不连接驱动）" << endl;
    cout << "  0) 退出" << endl;
    cout << "----------------------------------------" << endl;
    cout << "请输入序号: " << std::flush;

    int choice = -1;
    if (!(cin >> choice)) {
        cout << "[!] 输入无效" << endl;
        return false;
    }
    if (choice == 0) {
        cout << "[i] 已退出" << endl;
        return false;
    }
    if (choice == 3) {
        if (out_ui_only) *out_ui_only = true;
        cout << "[+] 仅 UI 模式：跳过内核驱动连接" << endl;
        cout << "[i] 游戏读写/初始化不可用，可调试界面与触摸区" << endl;
        return true;
    }
    if (choice != 1 && choice != 2) {
        cout << "[!] 仅支持 0/1/2/3" << endl;
        return false;
    }

    cout << "[*] 正在对接 " << (choice == 1 ? "TWT" : "RT") << " ..." << endl;
    if (!MemDriver_InitChoice(choice)) {
        cout << "[!] 驱动对接失败，未找到可用节点" << endl;
        return false;
    }

    const char* path = MemDriver_Path();
    cout << "[+] 驱动对接成功" << endl;
    cout << "    类型: " << MemDriver_Name() << endl;
    cout << "    路径: " << (path && path[0] ? path : "(unknown)") << endl;
    cout << "[*] 请在 UI 主页点击「初始化」绑定游戏数据" << endl;
    return true;
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    bool ui_only = false;
    if (!Terminal_SelectAndConnectDriver(&ui_only)) {
        return 1;
    }

    if (ui_only) {
        // 无驱动：标记状态，避免误以为已连接
        snprintf(GameEsp::R().status, sizeof(GameEsp::R().status), "仅 UI · 未连接驱动");
        GameEsp::R().inited = false;
        GameEsp::R().attached = false;
    }

    ::graphics = GraphicsManager::getGraphicsInterface(GraphicsManager::VULKAN);
    if (!::graphics) {
        std::cerr << "[!] graphics init failed" << std::endl;
        return 2;
    }

    ::screen_config();

    ::native_window_screen_x = (::displayInfo.height > ::displayInfo.width ? ::displayInfo.height : ::displayInfo.width);
    ::native_window_screen_y = (::displayInfo.height > ::displayInfo.width ? ::displayInfo.height : ::displayInfo.width);
    ::abs_ScreenX = (::displayInfo.height > ::displayInfo.width ? ::displayInfo.height : ::displayInfo.width);
    ::abs_ScreenY = (::displayInfo.height < ::displayInfo.width ? ::displayInfo.height : ::displayInfo.width);

    if (::native_window_screen_x <= 0 || ::native_window_screen_y <= 0) {
        std::cerr << "[!] invalid display size" << std::endl;
        return 3;
    }

    ::window = android::ANativeWindowCreator::Create("test", native_window_screen_x, native_window_screen_y, permeate_record);
    if (!::window) {
        std::cerr << "[!] native window create failed" << std::endl;
        return 4;
    }

    if (!graphics->Init_Render(::window, native_window_screen_x, native_window_screen_y)) {
        std::cerr << "[!] Init_Render failed" << std::endl;
        return 5;
    }

    Touch::Init({(float)::abs_ScreenX, (float)::abs_ScreenY}, false);
    Touch::setOrientation(displayInfo.orientation);

    ::init_My_drawdata();

    static bool flag = true;
    while (flag) {
        drawBegin();
        graphics->NewFrame();
        Layout_tick_UI(&flag);
        graphics->EndFrame();
    }

    graphics->Shutdown();
    android::ANativeWindowCreator::Destroy(::window);
    return 0;
}
