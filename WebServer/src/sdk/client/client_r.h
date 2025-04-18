#define RETURN_JSON_STRING(data) \
    return nlohmann::json(data).dump();

#define CHECK_JSON_VALIDITY(json) \
    if (!(json)) { \
        return nlohmann::json({ {"error", "Invalid JSON"} }).dump(); \
    }

std::vector<std::uint8_t> LoadDllToVector(const std::string & dllPath) {
    std::ifstream file(dllPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open DLL file");
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<std::uint8_t> buffer(fileSize);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        throw std::runtime_error("Failed to read DLL file");
    }

    return buffer;
}


std::string internal_api_resp(const crow::request& req) {
    auto json = crow::json::load(req.body);

    std::string action = json["action"].s();

    if (action == "login") {
        if (!json.has("username") || !json.has("password") || !json.has("hwid")) {
            return nlohmann::json({
                { "status", "err" },
                { "message", "Missing required fields" }
                }).dump();
        }

        std::string username = json["username"].s();
        std::string password = json["password"].s();
        std::string hwid = json["hwid"].s();

        if (!db.userExists(username)) {
            return nlohmann::json({
                { "status", "error" },
                { "message", "User not found" }
                }).dump();
        }

        User* current = db.getUser(username);

        if (current->hwid.empty())
            db.sethwid(username, hwid);

        if (!db.validateUserInLoader(username, password, hwid)) {
            return nlohmann::json({
                { "status", "error" },
                { "message", "Invalid credentials or HWID mismatch" }
                }).dump();
        }

        if (std::time(nullptr) >= current->subscription_expiry) {
            return nlohmann::json({
                { "status", "error" },
                { "message", "Subscription expired" }
                }).dump();
        }

        std::string token = db.generateTokenForUser(username);


        return nlohmann::json({
            { "status", "Login successful" },
            { "unical_id", current->uid },
            { "role", UserRole(current->role).toString() },
            { "subscription_expiry", current->subscription_expiry },
            { "username", current->username },
            { "password", current->password }
            }).dump();

    }

    if (action == "hash_login") {
        if (!json.has("hash") || !json.has("hwid")) {
            return nlohmann::json({ { "status", "error" }, { "message", "Missing hash" } }).dump();
        }


        std::cout << req.body << std::endl;

        std::string hash = json["hash"].s();
        std::string hwid = json["hwid"].s();

        if (hash.empty())
            return nlohmann::json({ { "status", "error" }, { "message", "Missing hash" } }).dump();

        User* current = db.getUserByHash(hash);

        if (!current)
            return nlohmann::json({ { "status", "error" }, { "message", "User not found" } }).dump();

        if (current->hwid == hwid) {
            db.sethash(current->username, "");

            return nlohmann::json({
                { "status", "Login successful" },
                { "unical_id", current->uid },
                { "role", UserRole(current->role).toString() },
                { "subscription_expiry", current->subscription_expiry },
                { "username", current->username },
                { "password", current->password }
                }).dump();
        }
    }

    if (action == "GetDllExport") {
        if (!json.has("username") || !json.has("password") || !json.has("hwid")) {
            return nlohmann::json({
                { "status", "err" },
                { "message", "Missing required fields" }
                }).dump();
        }

        std::string username = json["username"].s();
        std::string password = json["password"].s();
        std::string hwid = json["hwid"].s();

        if (!db.userExists(username)) {
            return nlohmann::json({
                { "status", "error" },
                { "message", "User not found" }
                }).dump();
        }

        User* current = db.getUser(username);

        if (db.validateUserInLoader(username, password, hwid)) {
            if (db.isSubscriptionActive(current->username)) {
                std::vector<std::uint8_t> DataDll = LoadDllToVector("./files/loaderexe/_[(4(1(14(88)88)8)20)]/_[(4(1(14(8(4(1(14(8(4(1(14(88)88)8)20)8)88)8)20)8)88)8)20)]/dll/HUETA.dll");

                return nlohmann::json({
                    { "status", "Successfly" },
                    { "Bytes", DataDll }
                    }).dump();
            }
            return nlohmann::json({
                { "status", "error" },
                { "message", "Sub expirity" }
                }).dump();
        }
        return nlohmann::json({
            { "status", "error" },
            { "message", "Invalid user" }
            }).dump();

    }
    else {
        return nlohmann::json({
            { "status", "error" },
            { "message", "Unknown action" }
            }).dump();
    }
}