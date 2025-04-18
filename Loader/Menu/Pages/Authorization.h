namespace Authorization {

    bool hashPage = false;
    float PageOffsetX = 0.f;

    void Render() {
        Particles::Render();

        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 300));

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.9f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

        static float maxY = 0.f;

        static float PageWight = ImGui::GetWindowWidth();

        static float HashInputY = 0.f;
        static float ButtonY = 0.f;

        if (maxY <= 40.f)
            maxY += 1.f;

        ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Reaction.wtf").x) * 0.4785f, 100 - maxY));
        ImGui::PushFont(medium[5]);

        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.4f, 1.0f), "Reaction.wtf");

        ImGui::PopFont();

        HashInputY = ImGui::GetCursorPos().y;

        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + 60 - PageOffsetX, ImGui::GetCursorPos().y + 20 + maxY * 0.6f / 2));

        CustomInputText("Username", usname, IM_ARRAYSIZE(usname));

        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + 60 - PageOffsetX, ImGui::GetCursorPos().y + maxY * 0.8f / 2));

        CustomInputText("Password", pass, IM_ARRAYSIZE(pass));

        ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - 64) * 0.5f - PageOffsetX, ImGui::GetCursorPos().y + maxY / 10));

        ButtonY = ImGui::GetCursorPos().y;

        if (CustomButtons::Button("Login", ImVec2(64, 32))) {
            server.loginUser(usname, pass);
            if (server.success) {
                pages[0] = false;
                pages[1] = true;
            }
        }

        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x + 60 + PageWight - PageOffsetX, HashInputY + 20 + maxY * 0.6f / 2));

        CustomInputText("hash", hashe, IM_ARRAYSIZE(hashe));

        ImGui::SetCursorPos(ImVec2((ImGui::GetWindowWidth() - 86) * 0.5f + PageWight - PageOffsetX, ButtonY - maxY));

        if (ImGui::Button("Hash login", ImVec2(86, 32))) {
            server.hash_login(hashe);
            if (server.success) {
                pages[0] = false;
                pages[1] = true;
            }
        }

        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() * 0.545f + 28 - PageOffsetX, 192));

        CustomCheckbox::Checkbox("Remember Me", &rememberMe);

        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() * 0.545f + 28 + PageWight - PageOffsetX, 192 - maxY));

        CustomCheckbox::Checkbox(" Remember Me", &rememberMe);

        if (hashPage && PageOffsetX <= PageWight) {
            PageOffsetX += PageWight / 12;
            PageOffsetX += PageOffsetX / 21;
            PageOffsetX += PageOffsetX / 15;
        }

        if (!hashPage && PageOffsetX >= 0) {
            PageOffsetX -= PageWight / 12;
            PageOffsetX -= PageOffsetX / 21;
            PageOffsetX -= PageOffsetX / 15;
        }

        if (PageOffsetX > PageWight) {
            PageOffsetX = PageWight;
        }

        if (PageOffsetX < 0) {
            PageOffsetX = 0;
        }


        if (!hashPage) {
            ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 20, ImGui::GetWindowHeight() * 0.5f));
            if (ImGui::Button(">")) {
                hashPage = true;
            }
        }
        else {
            ImGui::SetCursorPos(ImVec2(0, ImGui::GetWindowHeight() * 0.5f));
            if (ImGui::Button("<")) {
                hashPage = false;
            }
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
}