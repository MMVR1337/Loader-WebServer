void setCustomStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Background colors (dark theme)
    colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);        // Dark background
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);        // Dark child background

    // Text colors
    colors[ImGuiCol_Text] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f);             // Light text

    // Header and button colors
    colors[ImGuiCol_Header] = ImVec4(0.2f, 0.6f, 0.2f, 1.0f);           // Dark green header
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.7f, 0.3f, 1.0f);    // Lighter green on hover
    colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.5f, 0.15f, 1.0f);   // Even darker green when active

    // Button colors
    colors[ImGuiCol_Button] = ImVec4(0.6f, 0.2f, 0.2f, 1.0f);           // Dark red button
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.8f, 0.3f, 0.3f, 1.0f);    // Lighter red on hover
    colors[ImGuiCol_ButtonActive] = ImVec4(0.7f, 0.1f, 0.1f, 1.0f);     // Dark red when active

    // Combo box colors
    colors[ImGuiCol_PopupBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);         // Dark combo background

    // Tab colors
    colors[ImGuiCol_Tab] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);             // Dark tab
    colors[ImGuiCol_TabHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);      // Lighter tab on hover
    colors[ImGuiCol_TabActive] = ImVec4(0.1f, 0.5f, 0.1f, 1.0f);       // Dark green tab when active
}

void showMenu() {
    static char username[128], password[128], hwid[128], role[32];
    static char keyRole[32] = "USER";  // Роль для нового ключа
    static char newKey[256];  // Ввод ключа для удаления
    static int balance = 0;
    static UserRole::Type selectedRole = UserRole::Type::USER;
    static int selectedUserIndex = -1; // Изначально никакой пользователь не выбран
    static bool serverRunning = true;
    static std::vector<std::string> consoleLogs = { "Сервер запущен.", "Инициализация завершена." };

    // Вкладки
    if (ImGui::BeginTabBar("MainTabBar")) {

        // Страница управления ключами
        if (ImGui::BeginTabItem("Key Management")) {
            // Создание нового ключа с выбором роли
            ImGui::Text("Create New Key:");
            ImGui::Combo("Role for Key", (int*)&selectedRole, "ADMIN\0ALPHA\0BETA\0USER\0");
            if (ImGui::Button("Create Key")) {
                db.addKey(UserRole(selectedRole).toString().c_str());  // Добавляем новый ключ с ролью
                consoleLogs.push_back("Created a new key with role: " + std::string(UserRole(selectedRole).toString().c_str()));
                ImGui::Text("Key created!");
            }

            ImGui::Separator();
            ImGui::Text("Existing Keys:");

            // Список всех существующих ключей
            int keyCount = db.getInviteKeysCount();
            for (int i = 0; i < keyCount; ++i) {
                auto it = std::next(invite_keys.begin(), i);
                const auto& key = it->second;

                ImGui::PushID(i);
                ImGui::Text("Key: %s", key.key.c_str());

                // Комбо для изменения роли ключа
                static int selectedKeyRoleIndex = (int)key.role;
                if (ImGui::Combo("Role", &selectedKeyRoleIndex, "ADMIN\0ALPHA\0BETA\0USER\0")) {
                    UserRole::Type newRole = static_cast<UserRole::Type>(selectedKeyRoleIndex);
                    if (newRole != key.role) {
                        db.updateInviteKeyRole(key.key, newRole);
                        consoleLogs.push_back("Updated role for key: " + key.key + " to " + UserRole(newRole).toString());
                    }
                }

                ImGui::PopID();
            }

            ImGui::Separator();

            // Удаление ключа
            ImGui::Text("Delete Key:");
            ImGui::InputText("Enter Key", newKey, IM_ARRAYSIZE(newKey));
            if (ImGui::Button("Delete Key")) {
                db.remove_invite(newKey);
                db.force_save_data();
                consoleLogs.push_back("Key deleted: " + std::string(newKey));
                ImGui::Text("Key deleted!");
            }

            ImGui::EndTabItem();
        }

        // Страница управления пользователями
        if (ImGui::BeginTabItem("User Management")) {
            ImGui::Text("User List:");
            int userCount = db.getActiveUsersCount();
            for (int i = 0; i < userCount; ++i) {
                auto it = std::next(users.begin(), i);
                ImGui::PushID(it->first.c_str());
                if (ImGui::Selectable(it->first.c_str(), selectedUserIndex == i)) {
                    selectedUserIndex = i;  // Обновляем выбранного пользователя
                }
                ImGui::PopID();
            }

            // Проверка, выбран ли пользователь
            if (selectedUserIndex != -1) {
                auto it = std::next(users.begin(), selectedUserIndex);
                auto selectedUser = db.getUser(it->first);  // Получаем выбранного пользователя

                if (selectedUser) {
                    // Обновляем данные только если выбран новый пользователь
                    static int lastSelectedUserIndex = -1;
                    if (selectedUserIndex != lastSelectedUserIndex) {
                        // Заполняем значения для нового пользователя
                        strcpy(username, selectedUser->username.c_str());
                        strcpy(password, selectedUser->password.c_str());
                        strcpy(hwid, selectedUser->hwid.c_str());
                        balance = selectedUser->balance;
                        selectedRole = selectedUser->role;
                        lastSelectedUserIndex = selectedUserIndex;  // Обновляем индекс последнего выбранного пользователя
                    }

                    ImGui::Text("Editing User: %s", selectedUser->username.c_str());
                    ImGui::InputText("Username", username, IM_ARRAYSIZE(username));
                    ImGui::InputText("New Password", password, IM_ARRAYSIZE(password));
                    ImGui::InputText("HWID", hwid, IM_ARRAYSIZE(hwid));
                    ImGui::InputInt("Balance", &balance);
                    ImGui::Combo("New Role", (int*)&selectedRole, "ADMIN\0ALPHA\0BETA\0USER\0");

                    if (ImGui::Button("Save Changes")) {
                        selectedUser->password = password;
                        selectedUser->balance = balance;
                        db.promoteToRole(selectedUser->username, selectedRole);
                        db.force_save_data();
                        consoleLogs.push_back("User data modified: " + selectedUser->username);
                        ImGui::Text("Changes saved.");
                    }
                    if (ImGui::Button("Reset HWID")) {
                        db.sethwid(selectedUser->username, "");
                        consoleLogs.push_back("HWID reset for user: " + selectedUser->username);
                        ImGui::Text("HWID reset.");
                    }
                }
            }
            else {
                ImGui::Text("No user selected");
            }

            ImGui::EndTabItem();
        }

        // Страница консольных логов
        if (ImGui::BeginTabItem("Console Logs")) {
            ImGui::Text("Console Logs:");
            for (const auto& log : consoleLogs) {
                ImGui::TextUnformatted(log.c_str());
            }
            ImGui::EndTabItem();
        }

        // Страница статуса сервера
        if (ImGui::BeginTabItem("Server Status")) {
            ImGui::Text("Server Status: %s", serverRunning ? "Running" : "Stopped");

            if (ImGui::Button("Restart Server")) {
                consoleLogs.push_back("Server is restarting...");
                serverRunning = false;
                serverRunning = true;
                consoleLogs.push_back("Server successfully restarted.");
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}