#define CROW_ENABLE_SSL
#pragma warning(disable : 4996)

#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")

#include "crow.h"

#include "json.hpp"
#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#include "src/sdk/DataBase.h"
#include "src/sdk/client/client_r.h"
#include "src/sdk/site/site_r.h"

#include <d3d11.h>
#include <d3dx11.h>

#pragma comment(lib, "d3d11.lib")

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_internal.h"

#include "src/Menu/menu.h"