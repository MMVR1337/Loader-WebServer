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

struct User {
    std::string username;
    std::string password;
    std::string hash;
    std::string hwid;
    UserRole::Type role;
    int uid;
    int balance;
    std::string token;
    std::string inviter;
    time_t subscription_expiry;
    bool active_session = false;
};

struct InviteKey {
    std::string key;
    UserRole::Type role;
    bool used = false;
};

struct Product {
    std::string name;
    int days;
    double price;
};

std::unordered_map<std::string, User> users;
std::unordered_map<std::string, InviteKey> invite_keys;
std::unordered_map<std::string, User*> usersByHash;

class Database {
private:
    int current_uid = 0;
    std::string user_data_path = "./files/data/users.json";
    std::string key_data_path = "./files/data/keys.json";
    std::string web_login_data_path = "./files/data/web_login_requests.json";

    std::string generateToken() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        const char* hex_chars = "0123456789abcdef";
        std::string token;
        for (int i = 0; i < 32; ++i) token += hex_chars[dis(gen)];
        return token;
    }

    std::string generateRandomKey() {
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, chars.size() - 1);

        std::stringstream ss;
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                char rand_char = chars[dis(gen)];
                ss << rand_char;
            }
            if (i < 7) {
                ss << '-';
            }
        }

        return ss.str();
    }


    bool fileExists(const std::string& path) {
        std::ifstream file(path);
        return file.good();
    }

    void createEmptyFileIfNeeded(const std::string& path) {
        if (!fileExists(path)) {
            std::ofstream file(path);
            if (!file) {
                std::cerr << "Error creating file: " << path << std::endl;
            }
        }
    }

    void saveUserData() {
        json root;
        for (auto& [username, user] : users) {
            root[username]["password"] = user.password;
            root[username]["hwid"] = user.hwid;
            root[username]["hash"] = user.hash;
            root[username]["role"] = UserRole(user.role).toString();
            root[username]["uid"] = user.uid;
            root[username]["balance"] = user.balance;
            root[username]["token"] = user.token;
            root[username]["inviter"] = user.inviter;
            root[username]["subscription_expiry"] = user.subscription_expiry;
            root[username]["active_session"] = user.active_session;
        }

        std::ofstream file(user_data_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open user data file for saving." << std::endl;
            return;
        }
        file << std::setw(4) << root << std::endl;
    }

    void saveInviteKeys() {
        json root;
        for (auto& [key_str, key] : invite_keys) {
            root[key_str]["role"] = UserRole(key.role).toString();
            root[key_str]["used"] = key.used;
        }

        std::ofstream file(key_data_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open invite key file for saving." << std::endl;
            return;
        }
        file << std::setw(4) << root << std::endl;
    }

    void loadUserData() {
        createEmptyFileIfNeeded(user_data_path);

        std::ifstream file(user_data_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open user data file: " << user_data_path << std::endl;
            return;
        }

        json root;
        file >> root;

        if (root.empty()) {
            std::cerr << "User data file is empty, no data to load." << std::endl;
            return;
        }

        for (auto& [username, user_data] : root.items()) {
            User user;
            user.username = username;
            user.password = user_data["password"];
            user.hash = user_data["hash"];
            user.hwid = user_data["hwid"];
            user.role = UserRole::fromString(user_data["role"]);
            user.uid = user_data["uid"];
            user.balance = user_data["balance"];
            user.token = user_data["token"];
            user.inviter = user_data["inviter"];
            user.subscription_expiry = user_data["subscription_expiry"];
            user.active_session = user_data["active_session"];
            users[username] = user;
            usersByHash[user.hash] = &users[username];

            current_uid = std::max(current_uid, user.uid + 1);
        }
    }
    void loadInviteKeys() {
        createEmptyFileIfNeeded(key_data_path);

        std::ifstream file(key_data_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open invite key file: " << key_data_path << std::endl;
            return;
        }


        json root;
        file >> root;


        for (auto& [key_str, key_data] : root.items()) {
            InviteKey key;
            key.key = key_str;
            key.role = UserRole::fromString(key_data["role"]);
            key.used = key_data["used"];
            invite_keys[key_str] = key;
        }
    }

public:
    Database() {}

    void init() {
        loadInviteKeys();
        loadUserData();
    }


    void addUser(const std::string& username, const std::string& password, const std::string& hwid,
        const std::string& role, const std::string& inviter, time_t sub_duration) {
        int uid = users.size() + 1;
        users[username] = { username, password, "", hwid, UserRole::fromString(role), uid, 0, generateToken(), inviter, std::time(nullptr) + sub_duration};
        saveUserData();
    }

    bool validateToken(const std::string& username, const std::string& token) {
        if (users.find(username) == users.end()) return false;
        if (users[username].token != token) return false;
        saveUserData();
        return true;
    }

    bool startSession(const std::string& username) {
        if (users.find(username) == users.end() || users[username].subscription_expiry < std::time(nullptr)) return false;
        users[username].active_session = true;
        saveUserData();
        return true;
    }

    bool userExists(const std::string& username) {
        return users.find(username) != users.end();
    }

    InviteKey* getKey(const std::string& key) {
        if (invite_keys.find(key) == invite_keys.end()) {
            return nullptr;
        }
        return &invite_keys[key];
    }

    User* getUser(const std::string& username) {
        if (users.find(username) == users.end()) {
            return nullptr;
        }
        return &users[username];
    }

    User* getUserByHash(const std::string& hash) {
        auto it = usersByHash.find(hash);
        return (it != usersByHash.end()) ? it->second : nullptr;
    }

    std::string genUrlWL() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        const char* hex_chars = "0123456789abcdef";
        std::string token;
        for (int i = 0; i < 64; ++i) token += hex_chars[dis(gen)];
        return token;
    }

    bool buyProduct(const std::string& username, const std::string& product_id) {
        if (users.find(username) == users.end()) {
            return false;
        }

        User* user = &users[username];

        Product product;
        if (product_id == "rust_1_day") {
            product = { "Rust 1 Day", 1, 5.00 };
        }
        else if (product_id == "rust_7_days") {
            product = { "Rust 7 Days", 7, 20.00 };
        }
        else if (product_id == "rust_14_days") {
            product = { "Rust 14 Days", 14, 35.00 };
        }
        else if (product_id == "rust_30_days") {
            product = { "Rust 30 Days", 30, 60.00 };
        }
        else {
            return false; // Неверный ID продукта
        }

        if (user->balance < product.price) {
            return false; // Недостаточно средств
        }

        user->balance -= product.price;

        time_t current_time = std::time(nullptr);
        if (user->subscription_expiry > current_time) {
            user->subscription_expiry += product.days * 86400; 
        }
        else {
            user->subscription_expiry = current_time + product.days * 86400;
        }

        saveUserData();

        return true; 
    }

    bool registerUser(const std::string& username, const std::string& password, const std::string& hwid,
        const std::string& invite_key, const std::string& role, const std::string& inviter, time_t sub_duration) {
        if (invite_keys.find(invite_key) == invite_keys.end()) {
            return false;
        }

        if (users.find(username) != users.end()) {
            return false;
        }

        addUser(username, password, hwid, role, inviter, sub_duration);
        invite_keys.erase(invite_key);
        saveInviteKeys();
        return true;
    }

    void remove_invite(const std::string& invite_key) {
        invite_keys.erase(invite_key);
    }

    void addInviteKey(const std::string& invite_key, const std::string& inviter) {
        InviteKey key = { invite_key, UserRole::Type::USER, false };
        invite_keys[invite_key] = key;
        saveInviteKeys();
    }

    bool validateUser(const std::string& username, const std::string& password) {
        if (users.find(username) == users.end()) return false;
        return users[username].password == password;
    }

    bool validateUserInLoader(const std::string& username, const std::string& password, const std::string& hwid) {
        if (users.find(username) == users.end()) return false;
        return users[username].password == password && users[username].hwid == hwid;
    }

    std::string generateTokenForUser(const std::string& username) {
        std::string token = generateToken();
        users[username].token = token;
        saveUserData();
        return token;
    }

    void force_save_data() {
        saveUserData();
        saveInviteKeys();
    }

    void sethash(const std::string& username, const std::string& hash) {
        if (users.find(username) != users.end()) {
            usersByHash.erase(users[username].hash);
            users[username].hash = hash;
            usersByHash[hash] = &users[username];
            saveUserData();
        }
    }

    void sethwid(const std::string& username, const std::string& hwid) {
        users[username].hwid = hwid;
        saveUserData();
    }

    void endSession(const std::string& username) {
        if (users.find(username) != users.end()) users[username].active_session = false;
        saveUserData();
    }

    bool isSubscriptionActive(const std::string& username) {
        return users.find(username) != users.end() && users[username].subscription_expiry > std::time(nullptr);
    }

    bool isInviteKeyValid(const std::string& invite_key) {
        return invite_keys.find(invite_key) != invite_keys.end() && !invite_keys[invite_key].used;
    }

    bool addKey(const std::string& role) {
        std::string key = generateRandomKey();

        while (invite_keys.find(key) != invite_keys.end()) {
            key = generateRandomKey();
        }

        InviteKey newKey = { key, UserRole::fromString(role), false };

        invite_keys[key] = newKey;
        saveInviteKeys();
        return true;
    }


    std::string getInviter(const std::string& username) {
        if (users.find(username) != users.end()) {
            return users[username].inviter;
        }
        return "";
    }

    void updateSubscription(const std::string& username, time_t new_expiry) {
        if (users.find(username) != users.end()) {
            users[username].subscription_expiry = new_expiry;
            saveUserData();
        }
    }

    void promoteToRole(const std::string& username, UserRole::Type new_role) {
        if (users.find(username) != users.end()) {
            users[username].role = new_role;
            saveUserData();
        }
    }

    bool isUserActive(const std::string& username) {
        return users.find(username) != users.end() && users[username].active_session;
    }

    int getActiveUsersCount() {
        int count = 0;
        for (const auto& [username, user] : users) {
            if (user.active_session) {
                count++;
            }
        }
        return count;
    }

    int getInviteKeysCount() {
        int count = 0;
        for (const auto& [key, InviteKey] : invite_keys) {
            count++;
        }
        return count;
    }

    void updateInviteKeyRole(const std::string& key, UserRole::Type newRole) {
        if (invite_keys.find(key) != invite_keys.end()) {
            invite_keys[key].role = newRole;
            saveInviteKeys();
        }
    }
};

Database db;