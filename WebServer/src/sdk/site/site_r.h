std::string site_api_resp(const crow::request& req) {
    try {
        std::cout << "Request body: " << req.body << std::endl;

        auto json = crow::json::load(req.body);
        if (!json) {
            std::cout << "Invalid JSON format" << std::endl;
            return nlohmann::json({
                { "status", "Error" },
                { "message", "Invalid JSON format" }
                }).dump();
        }

        if (!json.has("action")) {
            std::cout << "Missing action" << std::endl;
            return crow::json::wvalue({ {"error", "Missing action"} }).dump();
        }

        std::string action = json["action"].s();
        std::cout << "Action: " << action << std::endl;

        if (action == "logout") {
            if (!json.has("username") || !json.has("password")) {
                std::cout << "Missing username or password" << std::endl;
                return nlohmann::json({
                    { "status", "Error" },
                    { "message", "Missing username or password" }
                    }).dump();
            }

            std::string username = json["username"].s();
            std::string password = json["password"].s();
            std::cout << "Logout attempt for: " << username << std::endl;

            if (db.validateUser(username, password)) {
                db.endSession(username);
                db.generateTokenForUser(username); // Генерируем новый токен при выходе
                std::cout << "User logged out: " << username << std::endl;
                return nlohmann::json({
                    { "status", "LoggedOut" }
                    }).dump();
            }
            else {
                std::cout << "Invalid credentials for logout: " << username << std::endl;
                return nlohmann::json({
                    { "status", "Error" },
                    { "message", "Invalid username or password" }
                    }).dump();
            }
        }

        if (action == "login") {
            if (!json.has("username") || !json.has("password")) {
                std::cout << "Missing username or password" << std::endl;
                return nlohmann::json({
                    { "status", "Error" },
                    { "message", "Missing username or password" }
                    }).dump();
            }

            std::string username = json["username"].s();
            std::string password = json["password"].s();
            std::cout << "Login attempt - Username: " << username << ", Password: " << password << std::endl;

            User* user = db.getUser(username);
            if (!user) {
                std::cout << "User not found: " << username << std::endl;
                return nlohmann::json({
                    { "status", "Error" },
                    { "message", "User not found" }
                    }).dump();
            }

            if (!db.validateUser(username, password)) {
                std::cout << "Invalid credentials for: " << username << std::endl;
                return nlohmann::json({
                    { "status", "Error" },
                    { "message", "Invalid username or password" }
                    }).dump();
            }

            std::string new_token = db.generateTokenForUser(username);
            db.startSession(username);
            std::cout << "Login successful for: " << username << ", New token: " << new_token << std::endl;

            return nlohmann::json({
                { "status", "Login successful" },
                { "token", new_token },
                { "hwid", user->hwid },
                { "unical_id", user->uid },
                { "role", UserRole(user->role).toString() },
                { "balance", user->balance },
                { "subscription_expiry", user->subscription_expiry }
                }).dump();
        }

        if (action == "register") {
            if (!json.has("username") || !json.has("password") || !json.has("invite_key")) {
                std::cout << "Missing fields for register" << std::endl;
                return nlohmann::json({
                    { "status", "Error" },
                    { "message", "Missing username, password, or invite_key" }
                    }).dump();
            }

            std::string username = json["username"].s();
            std::string password = json["password"].s();
            std::string invite_key = json["invite_key"].s();
            std::cout << "Register attempt - Action: " << action << ", Username: " << username << ", Password: " << password << ", Invite Key: " << invite_key << std::endl;

            if (db.isInviteKeyValid(invite_key)) {
                if (db.userExists(username)) {
                    std::cout << "User already exists: " << username << std::endl;
                    return nlohmann::json({
                        { "status", "Error" },
                        { "message", "User already exists" }
                        }).dump();
                }

                InviteKey* key = db.getKey(invite_key);
                if (key) {
                    if (db.registerUser(username, password, "", invite_key, UserRole(key->role).toString(), "", 0)) {
                        std::string new_token = db.generateTokenForUser(username);
                        std::cout << "User registered: " << username << std::endl;
                        return nlohmann::json({
                            { "status", "Success" },
                            { "token", new_token },
                            { "unical_id", db.getUser(username)->uid },
                            { "role", UserRole(db.getUser(username)->role).toString() },
                            { "balance", db.getUser(username)->balance },
                            { "subscription_expiry", db.getUser(username)->subscription_expiry },
                            { "hwid", db.getUser(username)->hwid }
                            }).dump();
                    }
                    else {
                        std::cout << "Failed to register user: " << username << std::endl;
                        return nlohmann::json({
                            { "status", "Error" },
                            { "message", "Failed to register user" }
                            }).dump();
                    }
                }
                else {
                    std::cout << "Invalid invite key: " << invite_key << std::endl;
                    return nlohmann::json({
                        { "status", "Error" },
                        { "message", "Invalid invite key" }
                        }).dump();
                }
            }
            else {
                std::cout << "Invalid invite key: " << invite_key << std::endl;
                return nlohmann::json({
                    { "status", "Error" },
                    { "message", "Invalid invite key" }
                    }).dump();
            }
        }

        if (action == "validate_token") {
            if (!json.has("username") || !json.has("token")) {
                return nlohmann::json({
                    { "status", "Error" },
                    { "message", "Missing username or token" }
                    }).dump();
            }

            std::string username = json["username"].s();
            std::string token = json["token"].s();

            if (!db.validateToken(username, token)) {
                return nlohmann::json({
                    { "status", "Error" },
                    { "message", "Invalid token" }
                    }).dump();
            }

            return nlohmann::json({
                { "status", "Success" },
                { "message", "Token is valid" }
                }).dump();
        }

        if (action == "generate_hash") {
            std::string username = json["username"].s();
            std::string token = json["token"].s();

            if (db.userExists(username)) {
                User* current = db.getUser(username);
                if (db.validateToken(username, token)) {
                    std::string hash = db.genUrlWL();

                    db.sethash(username, hash);

                    return nlohmann::json({ { "status", "success" }, { "user_hash", hash } }).dump();
                }
                return nlohmann::json({ { "status", "error" }, { "message", "Invalid token" } }).dump();
            }
            return nlohmann::json({ { "status", "error" }, { "message", "User not found" } }).dump();
        }

        return nlohmann::json({ { "status", "error" }, { "message", "Unknown action" } }).dump();
    }

    catch (const std::exception& e) {
        std::cerr << "Exception in site_api_resp: " << e.what() << std::endl;
        return nlohmann::json({
            { "status", "Error" },
            { "message", "Internal server error" }
            }).dump();
    }
}

std::string admin_api_resp(const crow::request& req) {
    std::cout << req.body << std::endl;

    auto json = crow::json::load(req.body);

    std::string action = json["action"].s();

    if (action == "generate_key") {
        std::string username = json["username"].s();
        std::string password = json["password"].s();
        std::string role = json["role"].s();

        std::cout << action << std::endl;
        std::cout << username << std::endl;
        std::cout << role << std::endl;

        if (db.validateUser(username, password)) {
            std::cout << "exists" << std::endl;
            User* current = db.getUser(username);
            if (current->role == UserRole::Type::ADMIN) {
                if (db.addKey(role)) {
                    return nlohmann::json({
                        { "status", "Succesfly" },
                        { "message", "Registed" }
                        }).dump();
                }
            }
        }
        else {
            return nlohmann::json({
                { "error", "Missing action" }
                }).dump();
        }
    }

    if (action == "get_key_list") {
        std::string username = json["username"].s();
        std::string password = json["password"].s();

        std::cout << "Action: " << action << std::endl;
        std::cout << "Username: " << username << std::endl;

        if (db.validateUser(username, password)) {
            std::string new_token = db.generateTokenForUser(username);
            std::cout << "User authenticated" << std::endl;

            User* current = db.getUser(username);
            if (current->role == UserRole::Type::ADMIN) {
                nlohmann::json key_list = nlohmann::json::array();

                for (const auto& [key_str, key] : invite_keys) {
                    nlohmann::json key_info;
                    key_info["key"] = key.key;
                    key_info["role"] = key.role == UserRole::Type::ADMIN ? "Admin" :
                        key.role == UserRole::Type::ALPHA ? "Alpha" :
                        key.role == UserRole::Type::BETA ? "Beta" : "User";
                    key_info["used"] = key.used;

                    key_list.push_back(key_info);
                }

                if (key_list.empty()) {
                    return nlohmann::json({
                        { "status", "No keys available" }
                        }).dump();
                }

                return nlohmann::json({
                    { "status", "Successfully" },
                    { "keys", key_list }
                    }).dump();
            }
            else {
                return nlohmann::json({
                    { "error", "User does not have admin privileges" }
                    }).dump();
            }
        }
        else {
            return nlohmann::json({
                { "error", "Invalid username or password" }
                }).dump();
        }
    }

    return crow::json::wvalue({
        { "error", "Missing action" }
        }).dump();
}

std::string shop_api_resp(const crow::request& req) {
    std::cout << req.body << std::endl;

    auto json = crow::json::load(req.body);

    if (!json.has("action")) {
        return crow::json::wvalue({
            { "error", "Missing action" }
            }).dump();
    }

    std::string action = json["action"].s();

    if (action == "buy_product") {
        if (!json.has("username") || !json.has("token") || !json.has("product_id")) {
            return crow::json::wvalue({
                { "error", "Missing required fields" }
                }).dump();
        }

        std::string username = json["username"].s();
        std::string token = json["token"].s();
        std::string product_id = json["product_id"].s();

        if (!db.userExists(username)) {
            return crow::json::wvalue({
                { "error", "User does not exist" }
                }).dump();
        }

        if (!db.validateToken(username, token)) {
            return crow::json::wvalue({
                { "error", "Invalid token" }
                }).dump();
        }

        if (db.buyProduct(username, product_id)) {
            User* user = db.getUser(username);
            db.generateTokenForUser(username);

            return crow::json::wvalue({
                { "status", "Success" },
                { "message", "Product purchased successfully" },
                { "product", product_id },
                { "new_token", user->token },
                { "new_balance", user->balance },
                { "new_subscription_expiry", user->subscription_expiry }
                }).dump();
        }
        else {
            return crow::json::wvalue({
                { "error", "Failed to purchase product" }
                }).dump();
        }
    }

    return crow::json::wvalue({
        { "error", "Invalid action" }
        }).dump();
}