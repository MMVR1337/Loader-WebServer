#ifndef SERVER_API_H
#define SERVER_API_H

#include <atomic>
#include <fstream>

using json = nlohmann::json;

struct UserRole {
    enum class Type {
        ADMIN,
        ALPHA,
        BETA,
        USER
    };

    Type role;

    UserRole() : role(Type::USER) {}

    UserRole(Type r) : role(r) {}

    std::string toString() const {
        switch (role) {
        case Type::ADMIN: return "Admin";
        case Type::ALPHA: return "Alpha";
        case Type::BETA: return "Beta";
        case Type::USER: return "User";
        default: return "unknown";
        }
    }

    static Type fromString(const std::string& str) {
        if (str == "Admin") return Type::ADMIN;
        else if (str == "Alpha") return Type::ALPHA;
        else if (str == "Beta") return Type::BETA;
        else return Type::USER;
    }
};


void ReadConfig(std::string& username, std::string& password) {
    std::string configPath = "C:/ProgramFiles/NVIDIA/Boot/config.json";

    std::ifstream file(configPath);

    if (!file.is_open()) {
        return;
    }

    nlohmann::json config;
    file >> config;

    if (!config.empty()) {
        username = config.begin().key();
        password = config[username]["pass"].get<std::string>();
    }

    file.close();
}

void ReadConfig_char(char* username, size_t username_size, char* password, size_t password_size) {
    const char* configPath = "C:/ProgramFiles/NVIDIA/Boot/config.json";

    std::ifstream file(configPath);
    if (!file.is_open()) {
        return;
    }

    nlohmann::json config;
    file >> config;
    file.close();

    if (!config.empty()) {
        std::string uname = config.begin().key();
        strncpy_s(username, username_size, uname.c_str(), _TRUNCATE);

        std::string pwd = config[uname]["pass"].get<std::string>();
        strncpy_s(password, password_size, pwd.c_str(), _TRUNCATE);
    }
}

class ServerAPI {
public:
    std::string time_end;
    std::string auth_token;
    std::string uid;
    time_t subscriptionExpiry;
    UserRole::Type userRole;
    std::atomic<bool> success{ false };
    std::string user_name;
    std::string user_password;
    ServerAPI(const std::string& server_url) : server_url(server_url) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize CURL");
        }
    }

    ~ServerAPI() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }

    bool InitializeAPI() {
        try {
            if (curl) {
                return true;
            }
            else {
                throw std::runtime_error("CURL инициализация не удалась.");
            }
        }
        catch (const std::exception& e) {
            return false;
        }
    }

    bool StartLogin() {
        try {
            char* password;
            ReadConfig_char(usname, sizeof(usname), password, sizeof(password));
            std::string response = loginUser("user1", "password123");

            return success;
        }
        catch (const std::exception& e) {
            return false;
        }
    }

    std::string loginUser(std::string username, std::string password) {
        return performRequest({
            {"action", "login"},
            {"username", username},
            {"password", password},
            {"hwid", getDiskSerials()}
            });
    }

    std::string hash_login(std::string hash) {
        return performRequest({
            {"action", "hash_login"},
            {"hash", hash},
            {"hwid", getDiskSerials()}
            });
    }

    std::vector<std::uint8_t> GetDllExport() {
        try {
            std::string response = performRequest({
                {"action", "GetDllExport"},
                {"username", user_name},
                {"password", user_password},
                {"hwid", getDiskSerials()}
                });

            nlohmann::json jsonResponse = nlohmann::json::parse(response);

            if (jsonResponse["status"] == "Successfly") {
                std::vector<std::uint8_t> dllBytes = jsonResponse["Bytes"];
                return dllBytes;
            }
            else {
                throw std::runtime_error("Server error: " + jsonResponse["message"].get<std::string>());
            }
        }
        catch (const std::exception& e) {
            throw std::runtime_error("Failed to get DLL: " + std::string(e.what()));
        }
    }

private:
    CURL* curl;
    std::string server_url;

    std::string getDiskSerials() const {
        DWORD serialNumber = 0;
        if (GetVolumeInformationA("C:\\", NULL, 0, &serialNumber, NULL, NULL, NULL, 0)) {
            return std::to_string(serialNumber);
        }
        return "";
    }


    void secureExit() {
        volatile uint64_t* crash = nullptr;
        *crash = 0xDEADBEEF;
    }

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }

    std::string performRequest(const json& request_body) {
        std::string response;
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, (server_url + "/internal").c_str());
            std::string request_body_str = request_body.dump();
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body_str.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request_body_str.size());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

            struct curl_slist* headers = curl_slist_append(NULL, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                throw std::runtime_error("CURL request failed: " + std::string(curl_easy_strerror(res)));
            }

            curl_slist_free_all(headers);

            std::cout << response << std::endl;

            processResponse(response);
        }
        return response;
    }

    void processResponse(const std::string& response) {
        try {
            json response_json = json::parse(response);
            if (response_json.contains("status")) {
                if (response_json.contains("role") && response_json.contains("subscription_expiry")) {
                    std::string status = response_json["status"];
                    std::string role = response_json["role"];
                    time_t subscription_expiry = response_json["subscription_expiry"].get<time_t>();
                    success = (status == "Login successful" || status == "success");
                    uid = std::to_string(response_json["unical_id"].get<int>());
                    userRole = UserRole::fromString(role);
                    subscriptionExpiry = subscription_expiry;
                    user_name = response_json["username"];
                    user_password = response_json["password"];
                }
            }
            else {
                success = false;
            }
        }
        catch (const json::exception& e) {
            success = false;
        }
    }

};
//reactionwtf.ddns.net
ServerAPI server("https://localhost/api/v1");

#endif // SERVER_API_H