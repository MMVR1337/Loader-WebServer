// mainmenu.h (полная переработка)
namespace mainmenu {
    static float animOffset = 1.0f;
    static bool animDirection = true;

    void FormatTimeDynamic(time_t expiry, char* buffer, size_t size) {
        time_t now = time(nullptr);
        time_t diff = expiry - now;

        if (diff <= 0) {
            strncpy_s(buffer, size, "Subscription active", _TRUNCATE);
            return;
        }

        // Автоматический пересчет каждый кадр
        int days = diff / (3600 * 24);
        diff %= (3600 * 24);
        int hours = diff / 3600;
        diff %= 3600;
        int minutes = diff / 60;
        int seconds = diff % 60;

        snprintf(buffer, size, "%02dd %02dh %02dm %02ds",
            days, hours, minutes, seconds);
    }

    void Render() {
        Particles::Render();

        // Анимация плавающего заголовка
        //animOffset += animDirection ? 0.8f : -0.8f;
        //if (abs(animOffset) > 15.0f) animDirection = !animDirection;

        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f,
            ImGui::GetIO().DisplaySize.y * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 400));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.09f, 0.09f, 0.09f, 0.95f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 15.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

        ImGui::Begin("Main Menu", nullptr,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoTitleBar);

        ImGui::SetCursorPos(ImVec2(
            (ImGui::GetWindowWidth() - ImGui::CalcTextSize("Reaction.wtf").x) * 0.5f,
            59 + animOffset
        ));
        ImGui::PushFont(medium[5]);
        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.4f, 1.0f), "Reaction.wtf");
        ImGui::PopFont();

        // Блок информации о пользователе
        ImGui::SetCursorPos(ImVec2(40, 100));
        ImGui::BeginChild("UserInfo", ImVec2(420, 200), true);

        ImGui::TextColored(ImVec4(0.4f, 0.6f, 1.0f, 1.0f), "User Profile");
        ImGui::Separator();

        ImGui::Text("Username: %s", server.user_name.c_str());
        ImGui::Text("Role: %s", UserRole(server.userRole).toString().c_str());
        ImGui::Text("UID: %s", server.uid.c_str());

        static char timerBuffer[32];
        FormatTimeDynamic(server.subscriptionExpiry, timerBuffer, sizeof(timerBuffer));

        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.4f, 1.0f), "Active: %s", timerBuffer);

        if (server.subscriptionExpiry <= time(nullptr)) {
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Subscription expired!");
            exit(1337);
        }

        ImGui::EndChild();

        ImGui::SetCursorPos(ImVec2(
            (ImGui::GetWindowWidth() - 120) * 0.5f,
            ImGui::GetWindowHeight() - 87
        ));
        if (ImGui::Button("Inject", ImVec2(120, 30))) {
            static std::thread a1(Inject);
            a1.detach();
        }

        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }
}