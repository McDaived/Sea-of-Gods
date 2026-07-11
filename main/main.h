#pragma once

#include "../library/tslib/tslib.h"
#include "../utils/engine.h"
#include <filesystem>

constexpr tslog::level LOG_LEVEL_TARGET = tslog::level::LOG;
constexpr bool SHOULD_ALLOC_CONSOLE = true;

#define safe_release(x) if(x) { x->Release(); x = nullptr; }
namespace fs = std::filesystem;

typedef HINSTANCE(WINAPI* f_LLA_t)(const char*);
typedef FARPROC(WINAPI* f_GPA_t)(HMODULE, LPCSTR);
typedef BOOL(WINAPIV* f_RAFT_t)(PRUNTIME_FUNCTION, DWORD, DWORD64);
typedef struct _MM_SYNC_DATA {
    f_LLA_t          pLoadLibraryA;
    f_GPA_t          pGetProcAddress;
    f_RAFT_t         pRtlAddFunctionTable;  
    PBYTE            pbase;
    HINSTANCE        hMod;
    DWORD            fdwReasonParam;
    LPVOID           reservedParam;
    BOOL             SEHSupport;
    PVOID* pHkFnLocAddress;
    PVOID            pHkFnAddress;
    volatile LONG    initComplete;          // set to 1 when DLL init is complete
} MM_SYNC_DATA, * PMM_SYNC_DATA;

static DWORD mSize = 0;
static uintptr_t dxgi = NULL;
static HINSTANCE g_hInstance = NULL;
static bool g_Running = false;
static bool g_ShowMenu = false;
static MODULEINFO g_BaseModule;
HANDLE Handle;
DWORD ID;
int WindowWidth;
int WindowHeight;
LPCSTR Title;
LPCSTR ClassName;
LPCSTR Path;

static ID3D11Device* device = nullptr;
static ID3D11DeviceContext* context = nullptr;
static ID3D11RenderTargetView* rtv = nullptr;

static int tPirateSeedN = -1;
static char tPirateSeed[0x64] = { 0 };
static int tPirateGender = -1;
static int tPirateEthnicity = -1;
static float tPirateAge = -1.f;
static float tPirateBodyType = -1.f;
static float tPirateBodyTypeModifier = -1.f;
static float tPirateDirtiness = -1.f;
static float tPirateWonkiness = -1.f;

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD reason, LPVOID reserved);
DWORD WINAPI MainThread(LPVOID lpParameter);
bool end();
bool getBaseModule();
void getModule();
void clearDX11Objects();
uintptr_t findGWorld();
uintptr_t findGObjects();
uintptr_t findGNames();


HRESULT presentHook(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
typedef HRESULT(*DX11Present)(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags);
DX11Present oDX11Present = nullptr;

HRESULT resizeBufferHook(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags);
typedef HRESULT(*DX11ResizeBuffer)(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags);
DX11ResizeBuffer oDX11ResizeBuffer = nullptr;

void UpdateImGuiCursor();
void OnMenuVisibilityChanged(bool nowOpen);

HCURSOR WINAPI hkSetCursor(HCURSOR hCursor);
BOOL WINAPI hkSetCursorPos(int x, int y);
typedef HCURSOR(WINAPI* fnSetCursor)(HCURSOR hCursor);
typedef BOOL(WINAPI* fnSetCursorPos)(int x, int y);
fnSetCursor oSetCursor = nullptr;
fnSetCursorPos oSetCursorPos = nullptr;

SHORT WINAPI hkGetAsyncKeyState(int vKey);
BOOL WINAPI hkGetKeyboardState(PBYTE lpKeyState);
typedef SHORT(WINAPI* fnGetAsyncKeyState)(int vKey);
typedef BOOL(WINAPI* fnGetKeyboardState)(PBYTE lpKeyState);
fnGetAsyncKeyState oGetAsyncKeyState = nullptr;
fnGetKeyboardState oGetKeyboardState = nullptr;
SHORT SafeGetAsyncKeyState(int vKey);

int WINAPI hkShowCursor(BOOL bShow);
typedef int(WINAPI* fnShowCursor)(BOOL bShow);
fnShowCursor oShowCursor = nullptr;
