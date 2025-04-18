#include "../includes.h"
#include <windows.h>
#include <psapi.h>
#include <iphlpapi.h>
#include <thread>
#include <atomic>
#include <filesystem>

#pragma comment(lib, "iphlpapi.lib")
UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
WNDCLASSEXW wc;
HWND hwnd;

ImVec2 screen = { 800, 500 };
ImVec2 navpos = { (screen.x / 2) - (216 / 2), screen.y + 250 };
ImVec2 window = { (GetSystemMetrics(SM_CXSCREEN) / 2) - (screen.x / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (screen.y / 2) };

bool done = false;

// ���������� DirectX
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// ���������� ��������� �������
std::atomic<bool> serverRunning(true);
std::vector<std::string> consoleLogs;

long getMemoryUsage() {
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / 1024 / 1024;
    }
    return 0;
}

double getCpuUsage() {
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        static ULARGE_INTEGER previousIdleTime = { 0, 0 };
        static ULARGE_INTEGER previousKernelTime = { 0, 0 };
        static ULARGE_INTEGER previousUserTime = { 0, 0 };

        ULARGE_INTEGER idleTimeValue, kernelTimeValue, userTimeValue;
        idleTimeValue.LowPart = idleTime.dwLowDateTime;
        idleTimeValue.HighPart = idleTime.dwHighDateTime;
        kernelTimeValue.LowPart = kernelTime.dwLowDateTime;
        kernelTimeValue.HighPart = kernelTime.dwHighDateTime;
        userTimeValue.LowPart = userTime.dwLowDateTime;
        userTimeValue.HighPart = userTime.dwHighDateTime;

        ULARGE_INTEGER totalIdleTime = { idleTimeValue.QuadPart - previousIdleTime.QuadPart };
        ULARGE_INTEGER totalKernelTime = { kernelTimeValue.QuadPart - previousKernelTime.QuadPart };
        ULARGE_INTEGER totalUserTime = { userTimeValue.QuadPart - previousUserTime.QuadPart };

        previousIdleTime = idleTimeValue;
        previousKernelTime = kernelTimeValue;
        previousUserTime = userTimeValue;

        ULONGLONG totalTime = totalKernelTime.QuadPart + totalUserTime.QuadPart;
        if (totalTime == 0) {
            return 0;
        }

        return 100.0 - (100.0 * totalIdleTime.QuadPart) / totalTime;
    }
    return 0;
}

double getNetworkUsage() {
    MIB_IFTABLE* pIfTable = nullptr;
    DWORD dwSize = 0;

    if (GetIfTable(pIfTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
        pIfTable = (MIB_IFTABLE*)malloc(dwSize);
    }

    if (pIfTable) {
        if (GetIfTable(pIfTable, &dwSize, FALSE) == NO_ERROR) {
            static ULONGLONG previousTotalBytes = 0;
            ULONGLONG totalBytes = 0;

            for (DWORD i = 0; i < pIfTable->dwNumEntries; i++) {
                totalBytes += pIfTable->table[i].dwInOctets + pIfTable->table[i].dwOutOctets;
            }

            ULONGLONG bytesDiff = totalBytes - previousTotalBytes;
            previousTotalBytes = totalBytes;

            free(pIfTable);

            return (bytesDiff * 8) / 1024 / 1024;
        }
        free(pIfTable);
    }
    return 0;
}


void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) g_mainRenderTargetView->Release();
    g_mainRenderTargetView = nullptr;
}


void startServer(crow::SimpleApp& app) {
    consoleLogs.push_back("������ �����������...");
    app.port(443)
        .ssl_file("sert/fullchain.pem", "sert/privkey.pem")
        .multithreaded()
        .run();
}

void stopServer(crow::SimpleApp& app) {
    consoleLogs.push_back("������ ���������������...");
    app.stop();
    serverRunning = false;
    consoleLogs.push_back("������ ����������.");
}

bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

inline void StartRender() {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
        if (msg.message == WM_QUIT)
            done = true;
    }
    if (done != false) {
        exit(0);
    }
    if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
    {
        CleanupRenderTarget();
        g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
        g_ResizeWidth = g_ResizeHeight = 0;
        CreateRenderTarget();
    }
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

inline void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE: {
        if (wParam == SIZE_MINIMIZED)
            return 0;
        UINT newWidth = LOWORD(lParam);
        UINT newHeight = HIWORD(lParam);

        if (newWidth != g_ResizeWidth || newHeight != g_ResizeHeight) {
            g_ResizeWidth = newWidth;
            g_ResizeHeight = newHeight;
        }

        return 0;
    }
    case WM_SYSCOMMAND: {
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
    }
                      break;
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    }

    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

void Init_WindowClass() {

    ImGui_ImplWin32_EnableDpiAwareness();
    wc = {
        sizeof(wc),
        CS_VREDRAW | CS_HREDRAW,
        WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
        L"Classname", nullptr
    };
    wc.hbrBackground = NULL;
    ::RegisterClassExW(&wc);
}

void Init_Hwnd() {

    hwnd = ::CreateWindowExW(
        NULL,                            // �������������� �����
        wc.lpszClassName,                // ��� ������������������� ������
        L"winFLSUHIloapo97af4",          // ��������� ����
        WS_POPUP,                        // ����� ����
        window.x,                        // ������� X
        window.y,                        // ������� Y
        screen.x,                        // ������
        screen.y,                        // ������
        NULL,                            // ������������ ����
        NULL,                            // ����
        GetModuleHandle(NULL),           // ���������� ����������
        NULL                             // �������������� ���������
    );
    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        done = true;
    }


    LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, style | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY | LWA_ALPHA);


}
void Init_DirectX() {

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
    ImGui::CreateContext();
    ImGui::StyleColorsDark();


    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);


    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

}

void ImGuiCleanupScene() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

inline void StopRender() {
    const float clear_color_with_alpha[4] = { 0 };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(1, 0);
}

template <typename Func> void renderImgui(Func&& code) {

    StartRender();
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(screen);
        ImGui::Begin("@", &done, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar); {
            code();
        }
        ImGui::End();
    }
    ImGui::Render();
    StopRender();
}

void ImGuiInitializate() {
    Init_WindowClass();

    Init_Hwnd();

    Init_DirectX();

    setCustomStyle();
}

float dtime = 0.f;

namespace fs = std::filesystem;

void server_thread() {
    static crow::SimpleApp app;

    static std::string key_path = "./files/data/keys.json";
    static std::string data_path = "./files/data/users.json";

    try {
        fs::create_directories("./files/data"); // ������ �����, ���� �����������

        if (!fs::exists(key_path)) {
            std::ofstream file(key_path);
            if (!file) {
                std::cerr << "Error creating keys file: " << key_path << std::endl;
                throw std::runtime_error("Failed to create keys file: " + key_path);
            }
            file << "{}"; // �������������� ������ JSON-��������
            file.close();
        }

        if (!fs::exists(data_path)) {
            std::ofstream file(data_path);
            if (!file) {
                std::cerr << "Error creating users file: " << data_path << std::endl;
                throw std::runtime_error("Failed to create users file: " + data_path);
            }
            file << "{}"; // �������������� ������ JSON-��������
            file.close();
        }

        db.init(); // ��������������, ��� db.init() ��������� keys.json � users.json

        // API ��������
// API ��������
        CROW_ROUTE(app, "/api/v1/internal").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
            return internal_api_resp(req);
            });

        CROW_ROUTE(app, "/api/v1/site").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
            return site_api_resp(req);
            });

        CROW_ROUTE(app, "/api/v1/shop_").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
            return shop_api_resp(req);
            });

        CROW_ROUTE(app, "/api/v1/secret/admin/SuperAPI").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
            return admin_api_resp(req);
            });

        CROW_ROUTE(app, "/api/v1/status")
            .methods(crow::HTTPMethod::GET)([]() {
            nlohmann::json status;
            status["memoryUsage"] = getMemoryUsage();
            status["cpuUsage"] = getCpuUsage();
            status["networkUsage"] = getNetworkUsage();
            status["activeUsers"] = db.getActiveUsersCount();
            return status.dump();
                });

        CROW_ROUTE(app, "/reaction/<string>")
            .methods(crow::HTTPMethod::GET)([](const crow::request& req, const std::string& filename) {
            std::string filepath = "reaction/" + filename;
            if (fs::exists(filepath)) {
                crow::response res;
                res.set_static_file_info(filepath);
                return res;
            }
            else {
                crow::response res(404);
                res.body = "File not found: " + filepath;
                return res;
            }
                });

        CROW_ROUTE(app, "/api/v1/GetUnicalLoader")
            .methods(crow::HTTPMethod::GET)([]() {
            std::string filepath = "./files/loaderexe/newversion/Loader.exe";
            if (fs::exists(filepath)) {
                crow::response res;
                res.set_static_file_info(filepath);
                res.add_header("Content-Disposition", "attachment; filename=Loader.exe");
                return res;
            }
            else {
                crow::response res(404);
                res.body = "File not found: " + filepath;
                return res;
            }
                });

        // �������� ��� HTML-������� ��� ���������� .html
        CROW_ROUTE(app, "/")([]() {
            crow::response res;
            res.set_static_file_info("reaction/index.html");
            return res;
            });
        CROW_ROUTE(app, "/sites/default/private/files/backup_migrate/scheduled/test.txt")([]() {
            crow::response res;
            res.code = 302;
            res.set_header("Location", "https://www.youtube.com/watch?v=dQw4w9WgXcQ"); // URL ��� ���������������
            return res;
            });

        CROW_ROUTE(app, "/admin")([]() {
            crow::response res;
            res.set_static_file_info("reaction/admin.html");
            return res;
            });

        CROW_ROUTE(app, "/auth")([]() {
            crow::response res;
            res.set_static_file_info("reaction/auth.html");
            return res;
            });

        CROW_ROUTE(app, "/register")([]() {
            crow::response res;
            res.set_static_file_info("reaction/register.html");
            return res;
            });

        CROW_ROUTE(app, "/block")([]() {
            crow::response res;
            res.set_static_file_info("reaction/block.html");
            return res;
            });

        CROW_ROUTE(app, "/profile")([]() {
            crow::response res;
            res.set_static_file_info("reaction/profile.html");
            return res;
            });

        CROW_ROUTE(app, "/shop")([]() {
            crow::response res;
            res.set_static_file_info("reaction/shop.html");
            return res;
            });

        CROW_ROUTE(app, "/status")([]() {
            crow::response res;
            res.set_static_file_info("reaction/status.html");
            return res;
            });

        // ���������� �������� ��������
        CROW_ROUTE(app, "/scripts/<string>")
            .methods(crow::HTTPMethod::GET)([](const crow::request& req, const std::string& filename) {
            std::string filepath = "reaction/scripts/" + filename;
            if (fs::exists(filepath)) {
                crow::response res;
                res.set_static_file_info(filepath);
                return res;
            }
            else {
                crow::response res(404);
                res.body = "File not found: " + filepath;
                return res;
            }
                });

        CROW_ROUTE(app, "/styles.css")
            .methods(crow::HTTPMethod::GET)([]() {
            std::string filepath = "reaction/styles.css";
            if (fs::exists(filepath)) {
                crow::response res;
                res.set_static_file_info(filepath);
                res.add_header("Content-Type", "text/css"); // ��������� ���������� MIME-���
                return res;
            }
            else {
                crow::response res(404);
                res.body = "File not found: " + filepath;
                return res;
            }
                });

        // ��������� SSL ��� HTTPS
        app.port(443)
            .ssl_file("sert/fullchain.pem", "sert/privkey.pem")
            .multithreaded()
            .run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error occurred: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "An unknown error occurred" << std::endl;
    }
}

void menu_thread() {
    while (true) {
        renderImgui([&]() {
            dtime = ImGui::GetIO().DeltaTime;
            showMenu();
            });
    }
}

int main() {

    ImGuiInitializate();

    std::thread server(server_thread);

    menu_thread();

    server.join();

    return 0;
}