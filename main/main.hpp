#include "../utils/engine.h"
#include <filesystem>
#include "Encryption.hpp"
#include <d3d11.h>
#include <dxgi.h>
#include <cstdio>
#include "../library/kiero/kiero.h"
#include "../library/imgui/imgui.h"
#include "../library/imgui/imgui_impl_win32.h"
#include "../library/imgui/imgui_impl_dx11.h"
#include "../library/tslib/tslib.h"


constexpr tslog::level LOG_LEVEL_TARGET = tslog::level::LOG;
constexpr bool SHOULD_ALLOC_CONSOLE = true;

#define safe_release(x) if(x) { x->Release(); x = nullptr; }
namespace fs = std::filesystem;

int MainThread();
void InitImGui();
void overlay();
void OnDraw();
void clearDX11Objects();
bool getBaseModule();
void getModule();
uintptr_t findGWorld();
uintptr_t findGObjects();
uintptr_t findGNames();

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT __stdcall hkPresent(IDXGISwapChain * pSwapChain, UINT SyncInterval, UINT Flags);
typedef HRESULT( __stdcall* Present ) ( IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags );
typedef LRESULT( CALLBACK* WNDPROC )( HWND, UINT, WPARAM, LPARAM );

extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

#define CONSOLE_LOGGING true
 #if CONSOLE_LOGGING
  #define Log(Arg, ...) (printf("[>] " Arg "\n", __VA_ARGS__))
 #else
	#define Log()
#endif

namespace Globals
{
	static void* DllInstance = nullptr;
	static bool IsClosing = false;
	static bool Open = false;
}

static int tPirateSeedN = -1;
static char tPirateSeed[0x64] = { 0 };
static int tPirateGender = -1;
static int tPirateEthnicity = -1;
static float tPirateAge = -1.f;
static float tPirateBodyType = -1.f;
static float tPirateBodyTypeModifier = -1.f;
static float tPirateDirtiness = -1.f;
static float tPirateWonkiness = -1.f;