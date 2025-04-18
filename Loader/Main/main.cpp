#include "../includes.h"

int main() {
    ImGuiInitializate();
    server.InitializeAPI();

    while (true) {
        //SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
        renderImgui([&]() {
            dtime = ImGui::GetIO().DeltaTime;
            if (pages[0]) {
                //login
                Authorization::Render();
            }
            else if (pages[1]) {
                //main
                mainmenu::Render();
            }
            //else if (pages[2]) {
            //    //settings
            //    SettingsPage();
            //}
            //else if (pages[3]) {
            //    //configs
            //    ConfigsPage();
            //}
            //MoveWnd(screen);
            });
    }
    ImGuiCleanupScene();
    return 0;
}

