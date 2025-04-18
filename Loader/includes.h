#include <windows.h>
#include <winnt.h>
#include <winbase.h>
#include <errhandlingapi.h>
#include <interlockedapi.h>
#include <processthreadsapi.h>
#include <synchapi.h>
#include <libloaderapi.h>
#include <minwindef.h>
#include <basetsd.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <array>
#include <string>
#include <iostream>
#include <istream>
#include <ostream>
#include <thread>

#include <json.hpp>
#include <curl/curl.h>
#include <d3d11.h>
#include <d3dx11.h>

#pragma comment(lib, "zlib.lib")
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Wldap32.lib")
#pragma comment(lib, "Secur32.lib")
#pragma comment(lib, "d3d11.lib")


char usname[128] = "";
char pass[128] = "";
char hashe[128] = "";
bool rememberMe = false;



#include "Server/Server.h"

#include "Utils/Injector/injector.h"

#include "Utils/func.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_internal.h"


RECT rc;
HWND hwnd;
WNDCLASSEXW wc;

float g_DpiScale = 1.0f;

ID3D11Device* g_pd3dDevice = nullptr;
ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
D3DX11_IMAGE_LOAD_INFO Iinfo;
ID3DX11ThreadPump* pump{ nullptr };

bool done = false;
float dtime;

ImVec2 screen = { 480, 320 };
ImVec2 navpos = { (screen.x / 2) - (216 / 2), screen.y + 250 };
ImVec2 window = { (GetSystemMetrics(SM_CXSCREEN) / 2) - (screen.x / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (screen.y / 2) };

bool pages[2] = { true, false };

#include "Main/bytes.h"
#include "Main/fonts.h"
#include "Main/init.h"

#include "Menu/Custom/Particles/Particles.h"

#include "Menu/Custom/Button/CustomButtons.h"
#include "Menu/Custom/CheckBox/CustomCheckBox.h"
#include "Menu/Custom/InputText/CustomInputText.h"

#include "Menu/Pages/Authorization.h"

#include "Menu/Pages/mainmenu.h"