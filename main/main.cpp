#include "../main/main.h"
#include "../library/minhook/DirectX11.h"
#include "../library/ui/ui.hpp"
#include "../library/ui/fonts.hpp"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void BuildMenuTabs();
void RenderMenu();

static int g_tabESP = -1;
static int g_tabIslands = -1;
static int g_tabAimBot = -1;


ImColor GetRainbowColor()
{
	static float time = 0.0f;
	time += 0.005f;
	ImVec4 color;
	color.x = sinf(0.3f * time + 0.0f) * 0.5f + 0.5f;
	color.y = sinf(0.3f * time + 2.0f) * 0.5f + 0.5f;
	color.z = sinf(0.3f * time + 4.0f) * 0.5f + 0.5f;
	color.w = 1.0f;
	return ImColor(color);
}


void setConsoleColor(int color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}


void printWelcomeMessage()
{
	setConsoleColor(10);
	std::cout <<
		"                ████████████              \n"
		"           █████████████████████          \n"
		"         ██████████████████ ██   █        \n"
		"       ██████████████████     █████       \n"
		"      ██ ████████████ █   ██████████      \n"
		"      ████████ ████  ████████████████     \n"
		"    ████████   ██  ███████████████████    \n"
		"   ███████████   ██████████████████████   \n"
		"    ██████     █████████████████  █████   \n"
		"    █████ ██      ████████████    █████   \n"
		"     █   ██       ██████████  ██  █████   \n"
		"      ██ ███   █ ██ ███████████   ████    \n"
		"     █            ████████      ██████    \n"
		"  █████          ████  ████████████████   \n"
		"   ████████████████     ████████████████  \n"
		"     ██████████████  ██  █████████████    \n"
		"       ████████████████████████ ████      \n"
		"           ████████████████████  ███      \n"
		"            ██████████████████            \n"
		"            ███ █ █████ ██████            \n"
		"            ███   █████ █████             \n"
		"            ███    ███   ████             \n"
		"                   ███  ██ █               \n";

	setConsoleColor(12);
	std::cout << "               Sea OF Gods\n\n";

	setConsoleColor(7);
}

static IDirectInput8* g_dinput = nullptr;
static IDirectInputDevice8* g_mouseDevice = nullptr;
static bool g_wheelInitFailed = false;

static float PollMouseWheelDelta()
{
	if (g_wheelInitFailed)
		return 0.0f;

	if (!g_dinput)
	{

		if (FAILED(DirectInput8Create(GetModuleHandleA(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8, reinterpret_cast<void**>(&g_dinput), nullptr)))
		{
			g_wheelInitFailed = true;
			return 0.0f;
		}
	}

	if (!g_mouseDevice)
	{
		if (FAILED(g_dinput->CreateDevice(GUID_SysMouse, &g_mouseDevice, nullptr)) ||
			FAILED(g_mouseDevice->SetDataFormat(&c_dfDIMouse)) ||
			FAILED(g_mouseDevice->SetCooperativeLevel(WindowHwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
		{
			if (g_mouseDevice) { g_mouseDevice->Release(); g_mouseDevice = nullptr; }
			g_wheelInitFailed = true;
			return 0.0f;
		}
	}

	static bool wasAnyButtonDown = false;
	bool isAnyButtonDown = (SafeGetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0 || (SafeGetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	bool clickEdge = isAnyButtonDown && !wasAnyButtonDown;
	wasAnyButtonDown = isAnyButtonDown;

	static ULONGLONG lastReacquireTick = 0;
	ULONGLONG nowTick = GetTickCount64();
	bool timerDue = (nowTick - lastReacquireTick) >= 1000;

	if (clickEdge || timerDue)
	{
		g_mouseDevice->Unacquire();
		lastReacquireTick = nowTick;
	}
	g_mouseDevice->Acquire();

	DIMOUSESTATE state = {};
	HRESULT hr = g_mouseDevice->GetDeviceState(sizeof(DIMOUSESTATE), &state);
	if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
	{
		g_mouseDevice->Acquire();
		hr = g_mouseDevice->GetDeviceState(sizeof(DIMOUSESTATE), &state);
	}
	if (FAILED(hr))
		return 0.0f;

	return static_cast<float>(state.lZ) / static_cast<float>(WHEEL_DELTA);
}

static void CleanupWheelInput()
{
	if (g_mouseDevice)
	{
		g_mouseDevice->Unacquire();
		g_mouseDevice->Release();
		g_mouseDevice = nullptr;
	}
	if (g_dinput)
	{
		g_dinput->Release();
		g_dinput = nullptr;
	}
	g_wheelInitFailed = false;
}

void PollMenuInput()
{
	if (!g_ShowMenu || ImGui::GetCurrentContext() == NULL || GetForegroundWindow() != WindowHwnd)
		return;

	ImGuiIO& io = ImGui::GetIO();

	io.MouseDown[0] = (SafeGetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	io.MouseDown[1] = (SafeGetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	io.MouseDown[2] = (SafeGetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
	io.MouseDown[3] = (SafeGetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;
	io.MouseDown[4] = (SafeGetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0;
	io.MouseWheel = PollMouseWheelDelta();
	static BYTE prevKeyState[256] = {};
	BYTE curKeyState[256] = {};
	for (int vk = 0; vk < 256; ++vk)
	{
		bool down = (SafeGetAsyncKeyState(vk) & 0x8000) != 0;
		curKeyState[vk] = down ? 0x80 : 0;
		io.KeysDown[vk] = down;
	}
	curKeyState[VK_CAPITAL] = (GetKeyState(VK_CAPITAL) & 0x0001) ? 1 : 0;
	curKeyState[VK_NUMLOCK] = (GetKeyState(VK_NUMLOCK) & 0x0001) ? 1 : 0;

	for (int vk = 0; vk < 256; ++vk)
	{
		bool wasDown = (prevKeyState[vk] & 0x80) != 0;
		bool isDown = (curKeyState[vk] & 0x80) != 0;
		if (isDown && !wasDown)
		{
			UINT scanCode = MapVirtualKeyW((UINT)vk, MAPVK_VK_TO_VSC);
			WCHAR chars[4] = {};
			int result = ToUnicode((UINT)vk, scanCode, curKeyState, chars, ARRAYSIZE(chars), 0);
			if (result > 0)
			{
				for (int i = 0; i < result; ++i)
					io.AddInputCharacterUTF16(chars[i]);
			}
		}
	}
	memcpy(prevKeyState, curKeyState, sizeof(curKeyState));

	if (io.KeysDown[VK_NEXT])   // Page Down
		io.MouseWheel -= 6.0f * io.DeltaTime;
	if (io.KeysDown[VK_PRIOR])  // Page Up
		io.MouseWheel += 6.0f * io.DeltaTime;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		g_hInstance = hModule;
		MainThread(lpReserved);
	}

	return TRUE;
}

DWORD WINAPI MainThread(LPVOID lpParameter)
{
	tslog::init(LOG_LEVEL_TARGET, SHOULD_ALLOC_CONSOLE);
	printWelcomeMessage();
	setConsoleColor(13);
	tslog::log("Injected at: %s", __TIME__);
	setConsoleColor(7);
	setConsoleColor(10);
	tslog::log("Module Device: 0x%x", reinterpret_cast<void*>(g_hInstance));
	setConsoleColor(7);

	if (MH_Initialize() != MH_OK) {
		setConsoleColor(12);
		tslog::critical("MinHook initialization failed.");
		setConsoleColor(7);
		end();
		return false;
	}

	if (!getBaseModule())
	{
		setConsoleColor(12);
		tslog::critical("Module Base Address not found.");
		setConsoleColor(7);
		end();
		return false;
	}
	else
	{

		setConsoleColor(10);
		tslog::log("Module Base Address: 0x%x", (void*)g_BaseModule.lpBaseOfDll);
		setConsoleColor(7);
	}

	setConsoleColor(14);
	tslog::verbose("Starting main initialization..");
	getModule();
	bool WindowFocus = false;
	while (WindowFocus == false) {
		if (GetCurrentProcessId()) {

			ID = GetCurrentProcessId();
			Handle = GetCurrentProcess();
			WindowHwnd = GetForegroundWindow();

			RECT TempRect;
			GetWindowRect(WindowHwnd, &TempRect);
			WindowWidth = TempRect.right - TempRect.left;
			WindowHeight = TempRect.bottom - TempRect.top;

			char TempTitle[MAX_PATH];
			GetWindowText(WindowHwnd, TempTitle, sizeof(TempTitle));
			Title = TempTitle;

			char TempClassName[MAX_PATH];
			GetClassName(WindowHwnd, TempClassName, sizeof(TempClassName));
			ClassName = TempClassName;

			char TempPath[MAX_PATH];
			GetModuleFileNameEx(Handle, NULL, TempPath, sizeof(TempPath));
			Path = TempPath;

			WindowFocus = true;
		}
	}
	bool InitHook = false;
	while (InitHook == false) {
		if (DirectX11::Init() == true) {
			CreateHook(8, (void**)&oDX11Present, presentHook);
			setConsoleColor(10);
			tslog::log("Present address: 0x%x", (void*)&presentHook);
			CreateHook(13, (void**)&oDX11ResizeBuffer, resizeBufferHook);
			setConsoleColor(10);
			tslog::log("ResizeBuffers address: 0x%x", (void*)&presentHook);
			InitHook = true;
		}
	}
	if (dxgi == NULL)
	{
		setConsoleColor(12);
		tslog::critical("dxgi.dll not found.");
		end();
		return false;
	}


	uintptr_t world = findGWorld();
	if (!world)
	{
		setConsoleColor(12);
		tslog::critical("GWorld not found.");
		end();
		return false;
	}

	uintptr_t objects = findGObjects();
	if (!objects)
	{
		setConsoleColor(12);
		tslog::critical("GObjects not found.");
		end();
		return false;
	}

	uintptr_t names = findGNames();
	if (!names)
	{
		setConsoleColor(12);
		tslog::critical("GNames not found.");
		end();
		return false;
	}

	if (!initUE4(world, objects, names))
	{
		setConsoleColor(12);
		tslog::critical("UE4 vars can not be initialized.");
		end();
		return false;
	}

	HookPostRender();
	tslog::log("PostRender Address: 0x%x", (void*)&hooked_postrender);
	tslog::log("ProcessEvent Address: 0x%x", (void*)&processhk);
	g_Running = true;

	if (lpParameter)
	{
		PMM_SYNC_DATA pSync = reinterpret_cast<PMM_SYNC_DATA>(lpParameter);
		InterlockedExchange(&pSync->initComplete, 1);
		tslog::log("initComplete signaled to injector.");
	}

	return true;
}


bool end()
{
	setConsoleColor(13);
	tslog::log("Exit..");
	tslog::log("Unload Main Menu..");

	SetGameInputBlocked(false);
	OnMenuVisibilityChanged(false);
	CleanupWheelInput();

	if (oDX11Present) {
		DisableHook(8);
		RemoveHook(8);
	}

	if (oDX11ResizeBuffer) {
		DisableHook(13);
		RemoveHook(13);
	}

	if (oSetCursor)
	{
		MH_DisableHook(oSetCursor);
		MH_RemoveHook(oSetCursor);
	}

	if (oSetCursorPos)
	{
		MH_DisableHook(oSetCursorPos);
		MH_RemoveHook(oSetCursorPos);
	}

	if (oShowCursor)
	{
		MH_DisableHook(oShowCursor);
		MH_RemoveHook(oShowCursor);
	}

	if (oGetAsyncKeyState)
	{
		MH_DisableHook(oGetAsyncKeyState);
		MH_RemoveHook(oGetAsyncKeyState);
	}

	if (oGetKeyboardState)
	{
		MH_DisableHook(oGetKeyboardState);
		MH_RemoveHook(oGetKeyboardState);
	}

	DisableAll();
	EngineShutdown();
	clearDX11Objects();
	tslog::shutdown();
	MH_Uninitialize();
	UnHookPostRender();
	FreeLibraryAndExitThread(g_hInstance, 0);
	return true;
}


void BuildMenuTabs()
{
	// ---------------------------------------------------------------- Client (no subtabs)
	int tabClient = ui::add_tab(ICON_FA_USER, "Client");
	ui::add_page(tabClient, []() {
		ui::begin_child("Client", { -1, 0 });
		{
			static const char* crosshairTypes[] = { "None", "Circle", "Cross" };
			int crosshairIndex = static_cast<int>(Config::cfg.client.crosshairType);

			ImGui::Checkbox("Enable##client.enable", &Config::cfg.client.enable);
			ImGui::Separator();

			ImGui::Checkbox("Player FOV##client.fovEnable", &Config::cfg.client.fovEnable);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable Field of View (FOV).");
			ImGui::SliderFloat("FOV Value##client.fov", &Config::cfg.client.fov, 60.f, 150.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Field of View (FOV) value for players.");
			ImGui::Checkbox("Spyglass FOV (Right Click)##client.spyRClickMode", &Config::cfg.client.spyRClickMode);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Zoom in without the lens.");
			ImGui::SliderFloat("Spyglass##client.spyglassFovMul", &Config::cfg.client.spyglassFovMul, 1.f, 25.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Field of View (FOV) value for spyglass.");
			ImGui::SliderFloat("Sniper##client.sniperFovMul", &Config::cfg.client.sniperFovMul, 1.f, 25.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Field of View (FOV) value for guns.");

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImGui::Checkbox("Compass##client.bCompass", &Config::cfg.client.bCompass);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the Compass.");
			ImGui::Checkbox("Oxygen Level##client.oxygen", &Config::cfg.client.oxygen);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the percentage of oxygen depletion.");

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImGui::Checkbox("CrossHair##client.crosshair", &Config::cfg.client.crosshair);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable crosshair.");
			ImGui::SliderFloat("CrossHair Size##client.crosshairSize", &Config::cfg.client.crosshairSize, 1.f, 50.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Size value for crosshair.");
			ImGui::SliderFloat("CrossHair Thickness##client.crosshairThickness", &Config::cfg.client.crosshairThickness, 1.f, 50.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Thickness value for crosshair.");
			ui::color("CrossHair Color", &Config::cfg.client.crosshairColor.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Add color for crosshair.");
			ImGui::SliderInt("Crosshair Type##client.crosshairType", &crosshairIndex, 0, IM_ARRAYSIZE(crosshairTypes) - 1, crosshairTypes[crosshairIndex]);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Select the type of crosshair.");
			Config::cfg.client.crosshairType = static_cast<decltype(Config::cfg.client.crosshairType)>(crosshairIndex);

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImGui::Checkbox("Custom Time of Day##client.bCustomTOD", &Config::cfg.client.bCustomTOD);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable custom time of day.");
			ImGui::SliderFloat("Time of Day##client.customTOD", &Config::cfg.client.customTOD, 1.f, 24.f, "%.0f:00");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Change time of day (Only client).");
		}
		ui::end_child();
	});

	// ---------------------------------------------------------------- ESP (Players / Skeletons / Ships)
	int tabESP = g_tabESP = ui::add_tab(ICON_FA_PLUS_CIRCLE, "ESP", { "Players", "Skeletons", "Ships" });
	ui::add_page(tabESP, []() { // Players
		ui::begin_child("Players ESP", { -1, 0 });
		{
			static const char* bars[] = { "None", "Left", "Right", "Bottom", "Top" };
			static const char* boxes[] = { "None", "2D Box", "3D Box" };

			ImGui::Checkbox("Enable##esp.players.enable", &Config::cfg.esp.players.enable);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable Player ESP.");

			ImGui::Separator();

			ImGui::Checkbox("Players Name##esp.players.bName", &Config::cfg.esp.players.bName);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Draw name for players.");
			ImGui::Checkbox("Player Bones##esp.players.bSkeleton", &Config::cfg.esp.players.bSkeleton);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Draw bones for player body");
			ImGui::Checkbox("Teammate##esp.players.team", &Config::cfg.esp.players.team);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show ESP your teammates.");

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImGui::Combo("Player ESP Box", reinterpret_cast<int*>(&Config::cfg.esp.players.boxType), boxes, IM_ARRAYSIZE(boxes));
			ImGui::Combo("Player Health Bar", reinterpret_cast<int*>(&Config::cfg.esp.players.barType), bars, IM_ARRAYSIZE(bars));

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImGui::SliderFloat("Players Distance##esp.players.renderDistance", &Config::cfg.esp.players.renderDistance, 1.f, 5000.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for a player to appear.");
			ui::color("Visible Color", &Config::cfg.esp.players.colorVisible.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the player that is visible to you.");
			ui::color("Invisible Color", &Config::cfg.esp.players.colorInvisible.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the player that is invisible to you.");

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImGui::Checkbox("Tracer Line##esp.players.tracers", &Config::cfg.esp.players.tracers);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show a line to the player.");
			ImGui::SliderFloat("Thickness Line##esp.players.tracersThickness", &Config::cfg.esp.players.tracersThickness, 1.f, 10.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Thickness of the line.");
		}
		ui::end_child();
	});
	ui::add_page(tabESP, []() { // Skeletons
		ui::begin_child("Skeletons ESP", { -1, 0 });
		{
			static const char* bars[] = { "None", "Left", "Right", "Bottom", "Top" };
			static const char* boxes[] = { "None", "2D Box", "3D Box" };

			ImGui::Checkbox("Enable##esp.skeletons.enable", &Config::cfg.esp.skeletons.enable);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable skeletons ESP.");

			ImGui::Separator();

			ImGui::Checkbox("Skeletons Bones##esp.skeletons.bSkeleton", &Config::cfg.esp.skeletons.bSkeleton);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Draw bones for skeletons body");

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImGui::Combo("Skele ESP Box", reinterpret_cast<int*>(&Config::cfg.esp.skeletons.boxType), boxes, IM_ARRAYSIZE(boxes));
			ImGui::Combo("Skele Health Bar", reinterpret_cast<int*>(&Config::cfg.esp.skeletons.barType), bars, IM_ARRAYSIZE(bars));

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImGui::SliderFloat("Skeletons Distance##esp.skeletons.renderDistance", &Config::cfg.esp.skeletons.renderDistance, 1.f, 1000.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for a skeleton to appear.");
			ui::color("Skeletons Color", &Config::cfg.esp.skeletons.color.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color ESP of the skeleton.");
			ui::color("Visible Color", &Config::cfg.esp.skeletons.colorVis.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the skeletons that is visible to you.");
			ui::color("Invisible Color", &Config::cfg.esp.skeletons.colorInv.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the skeletons that is invisible to you.");
		}
		ui::end_child();
	});
	ui::add_page(tabESP, []() { // Ships
		ui::begin_child("Ships ESP", { -1, 0 });
		{
			ImGui::Checkbox("Enable##esp.ships.enable", &Config::cfg.esp.ships.enable);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable ships ESP.");

			ImGui::Separator();

			ImGui::SliderFloat("Ships Distance##esp.ships.renderDistance", &Config::cfg.esp.ships.renderDistance, 1.f, 5000.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for a ship to appear.");
			ui::color("Ships Color", &Config::cfg.esp.ships.color.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color ESP of the ship.");
			ImGui::Checkbox("Show Holes##esp.ships.holes", &Config::cfg.esp.ships.holes);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show holes in the ship.");
			ImGui::Checkbox("Skeletons##esp.ships.skeletons", &Config::cfg.esp.ships.skeletons);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("ESP skeletons ship.");
			ImGui::Checkbox("Ghost Ships##esp.ships.ghosts", &Config::cfg.esp.ships.ghosts);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("ESP ghost ships.");
			ImGui::Checkbox("Ladders Position##esp.ships.showLadders", &Config::cfg.esp.ships.showLadders);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the position of the ladders to any ship.");

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImGui::Checkbox("Ship Trajectory [may crash]##esp.ships.shipTray", &Config::cfg.esp.ships.shipTray);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the trajectory of the ship.");
			ImGui::SliderFloat("Thickness##esp.ships.shipTrayThickness", &Config::cfg.esp.ships.shipTrayThickness, 0.f, 1000.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Thickness of the trajectory.");
			ImGui::SliderFloat("Height##esp.ships.shipTrayHeight", &Config::cfg.esp.ships.shipTrayHeight, -10.f, 20.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Height of the trajectory.");
			ui::color("Trajectory Color", &Config::cfg.esp.ships.shipTrayCol.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Color of the trajectory.");
		}
		ui::end_child();
	});

	// ---------------------------------------------------------------- Islands (Islands / Items / World)
	int tabIslands = g_tabIslands = ui::add_tab(ICON_FA_EYE, "Islands", { "Islands", "Items", "World" });
	ui::add_page(tabIslands, []() { // Islands
		ui::begin_child("Islands Items", { -1, 0 });
		{
			ImGui::Checkbox("Enable##esp.islands.enable", &Config::cfg.esp.islands.enable);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable island ESP.");

			ImGui::Separator();

			ImGui::SliderFloat("Size##esp.islands.size", &Config::cfg.esp.islands.size, 1.f, 10.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Name size of the island.");
			ImGui::SliderFloat("Islands Distance##esp.islands.renderDistance", &Config::cfg.esp.islands.renderDistance, 1.f, 10000.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for an islands names to appear.");
			ui::color("Islands Names Color", &Config::cfg.esp.islands.color.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the islands names.");

			ImGui::Separator();

			ImGui::Checkbox("Map Marks##esp.islands.marks", &Config::cfg.esp.islands.marks);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show marks location on the island.");
			ImGui::SliderFloat("Marks Distance##esp.islands.marksRenderDistance", &Config::cfg.esp.islands.marksRenderDistance, 1.f, 10000.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for a marks location to appear.");
			ui::color("Marks Color", &Config::cfg.esp.islands.marksColor.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the marks location.");

			ImGui::Separator();

			ImGui::Checkbox("Vaults##esp.islands.vaults", &Config::cfg.esp.islands.vaults);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the vaults location on the island when open.");
			ImGui::SliderFloat("Vaults Distance##esp.islands.vaultsRenderDistance", &Config::cfg.esp.islands.vaultsRenderDistance, 1.f, 10000.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for a vaults location to appear.");
			ui::color("Vaults Color", &Config::cfg.esp.islands.vaultsColor.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the vaults location.");

			ImGui::Separator();

			ImGui::Checkbox("Barrels##esp.islands.barrels", &Config::cfg.esp.islands.barrels);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the barrels location on the island.");
			ImGui::Checkbox("Barrels Items##esp.islands.barrelspeek", &Config::cfg.esp.islands.barrelspeek);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the items in the barrels.");
			ImGui::Checkbox("(R) Key Toggle##esp.islands.barrelstoggle", &Config::cfg.esp.islands.barrelstoggle);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle the barrels items with the (R) key.");
			ui::color("Barrels Color", &Config::cfg.esp.islands.barrelsColor.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the barrels location.");

			ImGui::Separator();

			ImGui::Checkbox("Ammo Chests##esp.islands.ammoChest", &Config::cfg.esp.islands.ammoChest);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the ammo chests location.");
			ImGui::SliderFloat("Ammo Distance##esp.islands.ammoChestRenderDistance", &Config::cfg.esp.islands.ammoChestRenderDistance, 1.f, 10000.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for a ammo chests location to appear.");
			ui::color("Ammo Color", &Config::cfg.esp.islands.ammoChestColor.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the ammo chests location.");
		}
		ui::end_child();
	});
	ui::add_page(tabIslands, []() { // Items
		ui::begin_child("Items", { -1, 0 });
		{
			static const char* boxes[] = { "None", "2D Box", "3D Box" };

			ImGui::Checkbox("Enable Items##esp.items.enable", &Config::cfg.esp.items.enable);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable items ESP.");
			ImGui::SliderFloat("Items Distance##esp.items.renderDistance", &Config::cfg.esp.items.renderDistance, 1.f, 3000.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for an items to appear.");
			ui::color("Items Color", &Config::cfg.esp.items.color.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the items.");
			ImGui::Checkbox("(R) key Toggle Names##esp.items.nameToggle", &Config::cfg.esp.items.nameToggle);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle the items names with the (R) key.");

			ImGui::Separator();

			ImGui::Checkbox("Enable Animals##esp.items.animals", &Config::cfg.esp.items.animals);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the animals location on the island.");
			ImGui::Checkbox("Animals Names##esp.items.bName", &Config::cfg.esp.items.bName);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the animals names on the island.");
			ImGui::SliderFloat("Animals Distance##esp.items.animalsRenderDistance", &Config::cfg.esp.items.animalsRenderDistance, 1.f, 500.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for a animals location to appear.");
			ui::color("Animals Color", &Config::cfg.esp.items.animalsColor.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the animals location.");
			ImGui::Combo("Animals ESP Box ", reinterpret_cast<int*>(&Config::cfg.esp.items.boxType), boxes, IM_ARRAYSIZE(boxes));

			ImGui::Separator();

			ImGui::Checkbox("Enable Sharks##esp.items.sharks", &Config::cfg.esp.items.sharks);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable sharks ESP include (megalodon).");
			ImGui::Checkbox("Sharks bones##esp.items.bSkeleton", &Config::cfg.esp.items.bSkeleton);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable sharks bones include (megalodon).");
			ImGui::SliderFloat("Sharks Distance##esp.items.sharksRenderDistance", &Config::cfg.esp.items.sharksRenderDistance, 1.f, 500.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for a sharks ESP to appear.");
			ui::color("Sharks Color", &Config::cfg.esp.items.sharksColor.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the sharks ESP and bones.");

			ImGui::Separator();

			ImGui::Checkbox("Lost Cargo##esp.items.lostCargo", &Config::cfg.esp.items.lostCargo);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the lost cargo location.");
			ui::color("Cargo Color", &Config::cfg.esp.items.cluesColor.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the lost cargo location.");
		}
		ui::end_child();
	});
	ui::add_page(tabIslands, []() { // World
		ui::begin_child("World", { -1, 0 });
		{
			ImGui::Checkbox("Enable##esp.others.enable", &Config::cfg.esp.others.enable);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable others ESP.");

			ImGui::Separator();

			ImGui::Checkbox("Shipwrecks##esp.others.shipwrecks", &Config::cfg.esp.others.shipwrecks);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the shipwrecks location in the sea.");
			ImGui::SliderFloat("Shipwrecks Distance##esp.others.shipwrecksRenderDistance", &Config::cfg.esp.others.shipwrecksRenderDistance, 1.f, 10000.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for a shipwrecks location to appear.");
			ui::color("Shipwrecks Color", &Config::cfg.esp.others.shipwrecksColor.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the shipwrecks location.");

			ImGui::Separator();

			ImGui::Checkbox("Mermaids##esp.others.mermaids", &Config::cfg.esp.others.mermaids);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the mermaids location in the sea.");
			ImGui::SliderFloat("Mermaids Distance##esp.others.mermaidsRenderDistance", &Config::cfg.esp.others.mermaidsRenderDistance, 1.f, 1000.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for a mermaids location to appear.");
			ui::color("Mermaids Color", &Config::cfg.esp.others.mermaidsColor.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the mermaids location.");

			ImGui::Separator();

			ImGui::Checkbox("Rowboats##esp.others.rowboats", &Config::cfg.esp.others.rowboats);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the rowboats location.");
			ImGui::SliderFloat("Rowboats Distance##esp.others.rowboatsRenderDistance", &Config::cfg.esp.others.rowboatsRenderDistance, 1.f, 3500.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for a rowboats location to appear.");
			ui::color("Rowboats Color", &Config::cfg.esp.others.rowboatsColor.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the rowboats location.");

			ImGui::Separator();

			ImGui::Checkbox("World Events##esp.others.events", &Config::cfg.esp.others.events);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the world events.");
			ImGui::SliderFloat("Events Distance##esp.others.eventsRenderDistance", &Config::cfg.esp.others.eventsRenderDistance, 1.f, 10000.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for a world events to appear.");
			ui::color("Events Color", &Config::cfg.esp.others.eventsColor.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the world events.");
		}
		ui::end_child();
	});

	// ---------------------------------------------------------------- AimBot (Weapon / Cannon / Harpoon)
	int tabAimBot = g_tabAimBot = ui::add_tab(ICON_FA_CROSSHAIRS, "AimBot", { "Weapon", "Cannon", "Harpoon" });
	ui::add_page(tabAimBot, []() { // Weapon
		ui::begin_child("Weapons AimBot", { -1, 0 });
		{
			ImGui::Checkbox("Enable##aim.weapon.enable", &Config::cfg.aim.weapon.enable);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable weapons aimbot.");

			ImGui::Separator();

			ImGui::SliderFloat("Pitch##aim.weapon.fPitch", &Config::cfg.aim.weapon.fPitch, 1.f, 100.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The ejection angle percentage, I recommend setting it to the maximum.");
			ImGui::SliderFloat("Yaw##aim.weapon.fYaw", &Config::cfg.aim.weapon.fYaw, 1.f, 100.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The sensitivity of vision, I recommend setting it to the maximum.");
			ImGui::SliderFloat("Smoothness##aim.weapon.smooth", &Config::cfg.aim.weapon.smooth, 1.f, 10.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Value smoothness capture of the enemy.");
			ImGui::SliderFloat("Height##aim.weapon.height", &Config::cfg.aim.weapon.height, 1.f, 100.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The height of the aimbot (Head | Chest | Body).");

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImGui::Checkbox("Players##aim.weapon.players", &Config::cfg.aim.weapon.players);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable aimbot to players.");
			ImGui::Checkbox("Skeletons##aim.weapon.skeletons", &Config::cfg.aim.weapon.skeletons);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable aimbot to skeletons.");
			ImGui::Checkbox("Gunpowder##aim.weapon.kegs", &Config::cfg.aim.weapon.kegs);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable aimbot to gunpowder.");
			ImGui::Checkbox("Instant Shot##aim.weapon.trigger", &Config::cfg.aim.weapon.trigger);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Automatic throw while aiming at the target.");
			ImGui::Checkbox("Visible Only##aim.weapon.visibleOnly", &Config::cfg.aim.weapon.visibleOnly);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable Aimbot if the enemy is visible.");
			ImGui::Checkbox("Beta Aim (F1)##aim.others.rage", &Config::cfg.aim.others.rage);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("New mathematical aiming accuracy, (beta version) it will be developed in the future, may stuck in last location you aimed (it works when you use a weapon).");
			ImGui::Checkbox("Fast Reload##aim.weapon.fastweapon", &Config::cfg.aim.weapon.fastweapon);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Reducing delay while reload and scoping");
		}
		ui::end_child();
	});
	ui::add_page(tabAimBot, []() { // Cannon
		ui::begin_child("Cannon AimBot", { -1, 0 });
		{
			ImGui::Checkbox("Enable##aim.cannon.enable", &Config::cfg.aim.cannon.enable);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable cannon aimbot.");

			ImGui::Separator();

			ImGui::SliderFloat("Pitch##aim.cannon.fPitch", &Config::cfg.aim.cannon.fPitch, 1.f, 100.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The ejection angle percentage, I recommend setting it to the maximum.");
			ImGui::SliderFloat("Yaw##aim.cannon.fYaw", &Config::cfg.aim.cannon.fYaw, 1.f, 100.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The sensitivity of vision, I recommend setting it to the maximum.");

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImGui::Checkbox("Instant Shot##aim.cannon.instant", &Config::cfg.aim.cannon.instant);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Automatic throw while aiming at the target.");
			ImGui::Checkbox("Chain Shots (F1)##aim.cannon.chains", &Config::cfg.aim.cannon.chains);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable aimbot to chain shots.");
			ImGui::Checkbox("Players##aim.cannon.players", &Config::cfg.aim.cannon.players);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Aim at the player who is on the ship or island (Inaccurate if the player is moving).");
			ImGui::Checkbox("Skeletons##aim.cannon.skeletons", &Config::cfg.aim.cannon.skeletons);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Aim at the skeletons who is on the ship or island.");
			ImGui::Checkbox("Ghost Ships##aim.cannon.ghostShips", &Config::cfg.aim.cannon.ghostShips);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Aim at the ghost ships.");
			ImGui::Checkbox("Cannon Pre-Fire (F4)##aim.cannon.drawPred", &Config::cfg.aim.cannon.drawPred);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Draw trajectory only for cannon ball to know where to aim.");
			ImGui::Checkbox("Damage Zone (F3)##aim.cannon.lowAim", &Config::cfg.aim.cannon.lowAim);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Aiming on damage zone and making holes sequentially (it doesn't aim at the same place).");
			ImGui::Checkbox("Player Deck (F2)##aim.cannon.deckshots", &Config::cfg.aim.cannon.deckshots);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The player is thrown onto the enemy ship.");
			ImGui::Checkbox("Visible Only##aim.cannon.visibleOnly", &Config::cfg.aim.cannon.visibleOnly);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable Aimbot if the ship is visible.");
			ImGui::Checkbox("Skeleton Throne##aim.cannon.skeletonthrone", &Config::cfg.aim.cannon.skeletonthrone);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Throw player and yourself to skeleton throne.");
			ImGui::Checkbox("Silent Aim Cannon (F5)##aim.cannon.silent2", &Config::cfg.aim.cannon.silent2);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The cannon targets the ship wherever it goes (you don't need to aim at it)");
		}
		ui::end_child();
	});
	ui::add_page(tabAimBot, []() { // Harpoon
		ui::begin_child("Harpoon AimBot", { -1, 0 });
		{
			ImGui::Checkbox("Enable##aim.harpoon.enable", &Config::cfg.aim.harpoon.enable);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable harpoon aimbot.");

			ImGui::Separator();

			ImGui::Checkbox("Visible only##aim.harpoon.visibleOnly", &Config::cfg.aim.harpoon.visibleOnly);
		}
		ui::end_child();
	});

	// ---------------------------------------------------------------- Global (no subtabs)
	int tabGlobal = ui::add_tab(ICON_FA_GLOBE, "Global");
	ui::add_page(tabGlobal, []() {
		ui::begin_child("Global", { -1, 0 });
		{
			ImGui::Checkbox("Enable##game.enable", &Config::cfg.game.enable);
			ImGui::Separator();

			ImGui::SliderFloat("Render Text Size##dev.renderTextSizeFactor", &Config::cfg.dev.renderTextSizeFactor, 0.1f, 3.0f, "%.2f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Size of the render appearing in the overlay.");
			ImGui::Separator();
			ImGui::Checkbox("Map Pins##game.mapPins", &Config::cfg.game.mapPins);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the location of the mark that was placed on the table.");
			ImGui::Checkbox("Players List##game.playerList", &Config::cfg.game.playerList);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the list of players in the server.");
			ImGui::Checkbox("Cook Tracker##game.cooking", &Config::cfg.game.cooking);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the cooking time of the food.");
			ImGui::Checkbox("Anti AFK##game.noIdleKick", &Config::cfg.game.noIdleKick);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Prevent the game from kicking you for being idle.");
			ImGui::Checkbox("Walk Underwate (C) key ##game.walkUnderwater", &Config::cfg.game.walkUnderwater);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable walk underwater.");
			ImGui::Checkbox("Sunk Location##game.showSunk", &Config::cfg.game.showSunk);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the location of the sunken ship.");
			ImGui::Checkbox("Ship Info##game.shipInfo", &Config::cfg.game.shipInfo);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show status about ships.");
			ImGui::Checkbox("Bunny Hop##game.b_bunnyhop", &Config::cfg.game.b_bunnyhop);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Bunny Hop, spam jumps.");
			ImGui::Checkbox("Interactive Distance##game.id", &Config::cfg.game.id);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Increase interactive distance by 5 meters.");
			ImGui::Checkbox("Instant Ladder##game.il", &Config::cfg.game.il);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Instant Ladder.");
			ui::color("Sunk Location Color", &Config::cfg.game.sunkColor.x, false);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("The color of the sunken ship location.");
			if (ImGui::Button("Clear Sunk List"))
			{
				ClearSunkList();
			}

			ImGui::Separator();
			ImGui::Text("Pirate Customizer");
			if (ImGui::Button("Open Pirate Generator"))
			{
				if (loadPirateGenerator())
				{
					engine::bInPirateGenerator = true;
				}
				else
				{
					ImGui::OpenPopup("##PirateGeneratorLoaded");
				}
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Modify the look of your pirate, but you must first delete the character or purchase a skin change from the store.");
			if (ImGui::BeginPopupModal("##PirateGeneratorLoaded", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::Text("\nSomething went wrong.\nThis must be used in the character creator.\n\n");
				ImGui::Separator();
				if (ImGui::Button("OK", { 170.f , 0.f })) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}
		}
		ui::end_child();
	});

	// ---------------------------------------------------------------- Debug (no subtabs)
	int tabDebug = ui::add_tab(ICON_FA_WRENCH, "Debug");
	ui::add_page(tabDebug, []() {
		ui::begin_child("Debug", { -1, 0 });
		{
			ImGui::Checkbox("Print errors codes in console##dev.printErrorCodes", &Config::cfg.dev.printErrorCodes);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Print errors codes in console like crash or feature doesn't work.");
			ImGui::Checkbox("Intercept Process Event##dev.interceptProcessEvent", &Config::cfg.dev.interceptProcessEvent);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Don't use it if you don't know what it does.");
			ImGui::Checkbox("Print RPC calls in console##dev.printRPCCalls", &Config::cfg.dev.printRPCCalls);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Don't use it if you don't know what it does.");
			ImGui::Separator();
			ImGui::Checkbox("Show Debug Names##dev.debugNames", &Config::cfg.dev.debugNames);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show the names of the objects in the game.");
			ImGui::Text("Filter Keyword");
			ImGui::InputText("Keyword", Config::cfg.dev.debugNamesFilter, IM_ARRAYSIZE(Config::cfg.dev.debugNamesFilter));
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Filter the keyword for the debug names.");
			ImGui::SliderInt("Debug Names Text Size##dev.debugNamesTextSize", &Config::cfg.dev.debugNamesTextSize, 1, 50);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Size of the debug names.");
			ImGui::SliderFloat("Debug Names Render Distance##dev.debugNamesRenderDistance", &Config::cfg.dev.debugNamesRenderDistance, 1.f, 1000.f, "%.0f");
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Distance for a debug names to appear.");
			ImGui::Checkbox("Dummy Boolean (Debugging purposes)##dev.bDummy", &Config::cfg.dev.bDummy);
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("Don't use it if you don't know what it does.");
		}
		ui::end_child();
	});

	// ---------------------------------------------------------------- Save (no subtabs)
	int tabSave = ui::add_tab(ICON_FA_SAVE, "Save");
	ui::add_page(tabSave, []() {
		ui::begin_child("Save", { -1, 0 });
		{
			if (ImGui::Button("Save Settings", ImVec2(120, 50)))
			{
				do {
					wchar_t buf[MAX_PATH];
					GetModuleFileNameW(g_hInstance, buf, MAX_PATH);
					fs::path path = fs::path(buf).remove_filename() / ".settings";
					auto file = CreateFileW(path.wstring().c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					if (file == INVALID_HANDLE_VALUE) break;
					DWORD written;
					if (WriteFile(file, &Config::cfg, sizeof(Config::cfg), &written, 0)) ImGui::OpenPopup("##SettingsSaved");
					CloseHandle(file);
				} while (false);
			}
			ImGui::SameLine();
			if (ImGui::Button("Load Settings", ImVec2(120, 50)))
			{
				do {
					wchar_t buf[MAX_PATH];
					GetModuleFileNameW(g_hInstance, buf, MAX_PATH);
					fs::path path = fs::path(buf).remove_filename() / ".settings";
					auto file = CreateFileW(path.wstring().c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if (file == INVALID_HANDLE_VALUE) break;
					DWORD readed;
					if (ReadFile(file, &Config::cfg, sizeof(Config::cfg), &readed, 0))  ImGui::OpenPopup("##SettingsLoaded");
					CloseHandle(file);
				} while (false);
			}

			ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("##SettingsSaved", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::Text("\nSettings Saved!\n\n");
				ImGui::Separator();
				if (ImGui::Button("OK", { 170.f , 0.f })) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			if (ImGui::BeginPopupModal("##SettingsLoaded", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::Text("\nSettings Loaded!\n\n");
				ImGui::Separator();
				if (ImGui::Button("OK", { 170.f , 0.f })) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}
		}
		ui::end_child();
	});
}

void RenderMenu()
{
	ImGui::SetNextWindowSize(ui::size);
	ImGui::Begin("Sea Of Gods Menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
	{
		ImGui::BeginChild("nav", { 220, ImGui::GetWindowHeight() }, 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
		{
			ImGui::BeginChild("brand", { ImGui::GetWindowWidth(), 42 }, 0, ImGuiWindowFlags_NoBackground);
			{
				ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetWindowPos(), { ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - 2 }, ImGui::GetColorU32(ImGuiCol_ChildBg), GImGui->Style.WindowRounding, ImDrawFlags_RoundCornersTop);
				ImGui::GetWindowDrawList()->AddRectFilled({ ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - 2 }, { ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y + ImGui::GetWindowHeight() }, ImGui::GetColorU32(ImGuiCol_Scheme));
			}
			ImGui::EndChild();

			ImGui::SetCursorPosY(42);
			ImGui::BeginChild("tabs", { ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - 42 }, 0, ImGuiWindowFlags_NoBackground);
			{
				imgui_blur::create_blur(ImGui::GetWindowDrawList(), ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), ImColor(1.f, 1.f, 1.f, GImGui->Style.Alpha), GImGui->Style.WindowRounding, ImDrawFlags_RoundCornersBottom);
				ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetWindowPos(), ImGui::GetWindowPos() + ImGui::GetWindowSize(), ImGui::GetColorU32(ImGuiCol_WindowBg, 0.875f), GImGui->Style.WindowRounding, ImDrawFlags_RoundCornersBottom);

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 1, 1 });
				ImGui::BeginGroup();
				{
					for (int i = 0; i < (int)ui::tabs.size(); ++i)
						ui::tab(i);
				}
				ImGui::EndGroup();
				ImGui::PopStyleVar();
			}
			ImGui::EndChild();

			ImGui::SetCursorPos({ 0, ImGui::GetWindowHeight() - 42 });
			ImGui::BeginChild("profile", { ImGui::GetWindowWidth(), 42 }, 0, ImGuiWindowFlags_NoBackground);
			{
				ImGui::GetWindowDrawList()->AddRectFilled({ ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + 1 }, ImGui::GetWindowPos() + ImGui::GetWindowSize(), ImGui::GetColorU32(ImGuiCol_Header), GImGui->Style.WindowRounding, ImDrawFlags_RoundCornersBottom);
				ImGui::GetWindowDrawList()->AddLine(ImGui::GetWindowPos(), { ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y }, ImGui::GetColorU32(ImGuiCol_BorderShadow));
				ImGui::GetWindowDrawList()->AddText(font(1), font(1)->FontSize, { ImGui::GetWindowPos().x + 15, ImGui::GetWindowPos().y + ImGui::GetWindowHeight() / 2 - font(1)->CalcTextSizeA(font(1)->FontSize, FLT_MAX, 0.f, "Sea Of Gods by - Daived").y / 2 }, ImGui::GetColorU32(ImGuiCol_Text), "Sea Of Gods by - Daived");
				ImGui::GetWindowDrawList()->AddText(font(1), font(1)->FontSize, { ImGui::GetWindowPos().x + ImGui::GetWindowWidth() - font(1)->CalcTextSizeA(font(1)->FontSize, FLT_MAX, 0.f, "v4").x - 15, ImGui::GetWindowPos().y + ImGui::GetWindowHeight() / 2 - font(1)->CalcTextSizeA(font(1)->FontSize, FLT_MAX, 0.f, "v3").y / 2 }, ImGui::GetColorU32(ImGuiCol_TextDisabled), "v3");
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();

		ImGui::SetCursorPos({ 229, 0 });
		ImGui::BeginChild("main", ImGui::GetContentRegionAvail(), 0, ImGuiWindowFlags_NoBackground);
		{
			ImGui::BeginChild("main header", { ImGui::GetWindowWidth(), 42 }, 0, ImGuiWindowFlags_NoBackground);
			{
				ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetWindowPos(), { ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - 2 }, ImGui::GetColorU32(ImGuiCol_ChildBg), GImGui->Style.WindowRounding, ImDrawFlags_RoundCornersTop);
				ImGui::GetWindowDrawList()->AddRectFilled({ ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetWindowHeight() - 2 }, { ImGui::GetWindowPos().x + ImGui::GetWindowWidth(), ImGui::GetWindowPos().y + ImGui::GetWindowHeight() }, ImGui::GetColorU32(ImGuiCol_Scheme));

				if (!ui::tabs.empty())
				{
					auto& cur = ui::tabs[ui::cur_tab];
					ImGui::GetWindowDrawList()->AddText({ ImGui::GetWindowPos().x + 15, ImGui::GetWindowPos().y + ImGui::GetWindowHeight() / 2 - ImGui::CalcTextSize(cur.m_icon, 0, 1).y / 2 }, ImGui::GetColorU32(ImGuiCol_Text, ui::anim), cur.m_icon, ImGui::FindRenderedTextEnd(cur.m_icon));
					ImGui::GetWindowDrawList()->AddText(font(2), font(2)->FontSize, { ImGui::GetWindowPos().x + 40, ImGui::GetWindowPos().y + ImGui::GetWindowHeight() / 2 - font(2)->CalcTextSizeA(font(2)->FontSize, FLT_MAX, 0.f, cur.m_name, ImGui::FindRenderedTextEnd(cur.m_name)).y / 2 }, ImGui::GetColorU32(cur.m_subtabs.size() > 0 ? ImGuiCol_TextDisabled : ImGuiCol_Text, ui::anim), cur.m_name, ImGui::FindRenderedTextEnd(cur.m_name));
					if (cur.m_subtabs.size() > 0)
					{
						float name_w = font(2)->CalcTextSizeA(font(2)->FontSize, FLT_MAX, 0.f, cur.m_name, ImGui::FindRenderedTextEnd(cur.m_name)).x;
						ImGui::RenderArrow(ImGui::GetWindowDrawList(), { ImGui::GetWindowPos().x + 47 + name_w, ImGui::GetWindowPos().y + ImGui::GetWindowHeight() / 2 - 10 }, ImGui::GetColorU32(ImGuiCol_Text, ui::anim * ui::content_anim), ImGuiDir_Right, 12);
						const char* sub_name = cur.m_subtabs[cur.cur_subtab];
						ImGui::GetWindowDrawList()->AddText(font(2), font(2)->FontSize, { ImGui::GetWindowPos().x + 60 + name_w, ImGui::GetWindowPos().y + ImGui::GetWindowHeight() / 2 - font(2)->CalcTextSizeA(font(2)->FontSize, FLT_MAX, 0.f, sub_name, ImGui::FindRenderedTextEnd(sub_name)).y / 2 }, ImGui::GetColorU32(ImGuiCol_Scheme, ui::anim * ui::content_anim), sub_name, ImGui::FindRenderedTextEnd(sub_name));
					}
				}
			}
			ImGui::EndChild();

			ImGui::GetWindowDrawList()->AddRectFilled({ ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + 42 }, ImGui::GetWindowPos() + ImGui::GetWindowSize(), ImGui::GetColorU32(ImGuiCol_WindowBg), GImGui->Style.WindowRounding, ImDrawFlags_RoundCornersBottom);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 10, 10 });
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 15, 15 });
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ui::anim * ui::content_anim);
			ImGui::SetCursorPosY(42);
			ImGui::BeginChild("content", ImGui::GetContentRegionAvail(), 0, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysUseWindowPadding);
			{

				if (ui::cur_tab == g_tabESP || ui::cur_tab == g_tabIslands)
				{
					ImGui::Checkbox("Enable Global ESP##esp.enable", &Config::cfg.esp.enable);
					if (ImGui::IsItemHovered()) ImGui::SetTooltip("Master switch for all ESP (ESP tab and Islands tab).");
					ImGui::Separator();
				}
				else if (ui::cur_tab == g_tabAimBot)
				{
					ImGui::Checkbox("Enable Global AimBot##aim.enable", &Config::cfg.aim.enable);
					if (ImGui::IsItemHovered()) ImGui::SetTooltip("Master switch for all AimBot tabs (Weapon/Cannon/Harpoon).");
					ImGui::Separator();
				}

				ui::render_page();
			}
			ImGui::EndChild();
			ImGui::PopStyleVar(3);
		}
		ImGui::EndChild();
	}
	ImGui::End();

	ui::handle_anims();
}

HRESULT presentHook(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
{
	if (!g_Running) return oDX11Present(swapChain, syncInterval, flags);

	if (!device)
	{
		ID3D11Texture2D* buffer;

		swapChain->GetBuffer(0, IID_PPV_ARGS(&buffer));
		swapChain->GetDevice(IID_PPV_ARGS(&device));
		device->CreateRenderTargetView(buffer, nullptr, &rtv);
		device->GetImmediateContext(&context);

		safe_release(buffer);

		DXGI_SWAP_CHAIN_DESC desc;
		swapChain->GetDesc(&desc);
		auto& window = desc.OutputWindow;
		WindowHwnd = desc.OutputWindow;

		if (IsWindow(WindowHwnd))
		{
			char winTitle[256] = {};
			char winClass[256] = {};
			GetWindowTextA(WindowHwnd, winTitle, sizeof(winTitle));
			GetClassNameA(WindowHwnd, winClass, sizeof(winClass));
			setConsoleColor(10);
			tslog::log("Window connected: hwnd=%p title=\"%s\" class=\"%s\" foreground=%d",
				WindowHwnd, winTitle, winClass, (GetForegroundWindow() == WindowHwnd) ? 1 : 0);
			setConsoleColor(7);
		}
		else
		{
			setConsoleColor(12);
			tslog::critical("Window connection FAILED: hwnd=%p from swap chain is not a valid window.", WindowHwnd);
			setConsoleColor(7);
		}

		static bool hooksCreated = false;
		if (!hooksCreated)
		{
			hooksCreated = true;
			bool cursorOk = true;
			MH_STATUS cursorStatus = MH_CreateHook(SetCursor, hkSetCursor, reinterpret_cast<LPVOID*>(&oSetCursor));
			if (cursorStatus != MH_OK)
			{
				tslog::critical("SetCursor hook creation failed: %s (%d)", MH_StatusToString(cursorStatus), (int)cursorStatus);
				cursorOk = false;
			}
			cursorStatus = MH_CreateHook(SetCursorPos, hkSetCursorPos, reinterpret_cast<LPVOID*>(&oSetCursorPos));
			if (cursorStatus != MH_OK)
			{
				tslog::critical("SetCursorPos hook creation failed: %s (%d)", MH_StatusToString(cursorStatus), (int)cursorStatus);
				cursorOk = false;
			}
			cursorStatus = MH_CreateHook(ShowCursor, hkShowCursor, reinterpret_cast<LPVOID*>(&oShowCursor));
			if (cursorStatus != MH_OK)
			{
				tslog::critical("ShowCursor hook creation failed: %s (%d)", MH_StatusToString(cursorStatus), (int)cursorStatus);
				cursorOk = false;
			}

			bool inputBlockOk = true;
			MH_STATUS inputStatus = MH_CreateHook(GetAsyncKeyState, hkGetAsyncKeyState, reinterpret_cast<LPVOID*>(&oGetAsyncKeyState));
			if (inputStatus != MH_OK)
			{
				tslog::critical("GetAsyncKeyState hook creation failed: %s (%d)", MH_StatusToString(inputStatus), (int)inputStatus);
				inputBlockOk = false;
			}
			inputStatus = MH_CreateHook(GetKeyboardState, hkGetKeyboardState, reinterpret_cast<LPVOID*>(&oGetKeyboardState));
			if (inputStatus != MH_OK)
			{
				tslog::critical("GetKeyboardState hook creation failed: %s (%d)", MH_StatusToString(inputStatus), (int)inputStatus);
				inputBlockOk = false;
			}

			if (MH_EnableHook(MH_ALL_HOOKS) == MH_OK)
			{
				if (cursorOk)
					tslog::log("Cursor hooks enabled.");
				if (inputBlockOk)
					tslog::log("Game input-blocking hooks enabled.");
			}
			else
			{
				tslog::critical("Failed to enable cursor/input-blocking hooks.");
			}

			setConsoleColor(10);
			tslog::log("Initialization and hooking successful!\n");
			setConsoleColor(9);
			tslog::log("Menu Keys: [Ins] or [Home] -> Open Menu - [End] -> Unload Menu\n");
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui::GetStyle().Colors[ImGuiCol_Scheme] = ImColor(255, 128, 0);

		ImGuiIO& io = ImGui::GetIO();

		ImFontConfig textConfig;
		textConfig.FontDataOwnedByAtlas = false;

		static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
		ImFontConfig iconsConfig;
		iconsConfig.MergeMode = true;
		iconsConfig.PixelSnapH = true;
		iconsConfig.OversampleH = 3;
		iconsConfig.OversampleV = 3;
		iconsConfig.FontDataOwnedByAtlas = false;

		io.Fonts->AddFontFromMemoryTTF(font_medium_bin, sizeof(font_medium_bin), 14, &textConfig);
		io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 14.0f, &iconsConfig, icons_ranges);
		io.Fonts->AddFontFromMemoryTTF(font_bold_bin, sizeof(font_bold_bin), 14, &textConfig);
		io.Fonts->AddFontFromMemoryTTF(font_medium_bin, sizeof(font_medium_bin), 17, &textConfig);
		io.Fonts->AddFontFromMemoryTTF(font_bold_bin, sizeof(font_bold_bin), 17, &textConfig);
		io.Fonts->Build();

		ImGui_ImplWin32_Init(WindowHwnd);
		ImGui_ImplDX11_Init(device, context);
		ImGui_ImplDX11_CreateDeviceObjects();

		imgui_blur::set_device(device, context);

		static bool tabsBuilt = false;
		if (!tabsBuilt)
		{
			tabsBuilt = true;
			BuildMenuTabs();
		}
	}


	if ((SafeGetAsyncKeyState(VK_INSERT) & 1) || (SafeGetAsyncKeyState(VK_HOME) & 1))
	{
		g_ShowMenu = !g_ShowMenu;
		SetGameInputBlocked(g_ShowMenu);
		OnMenuVisibilityChanged(g_ShowMenu);
	}
	if (SafeGetAsyncKeyState(VK_END) & 1)
	{
		end();
	}

	PollMenuInput();
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	imgui_blur::new_frame(rtv);

	ImGui::GetIO().MouseDrawCursor = g_ShowMenu;

	if (g_ShowMenu)
		UpdateImGuiCursor();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
	ImGui::Begin("overlay", nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar);
	auto& io = ImGui::GetIO();
	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
	ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);

	auto drawList = ImGui::GetCurrentWindow()->DrawList;

	if (updateGameVars())
	{
		render(ImGui::GetCurrentWindow()->DrawList);
	}

	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	if (g_ShowMenu)
	{
		RenderMenu();
	}

	if (engine::bInPirateGenerator)
	{
		ImVec4 rainbowTextColor = GetRainbowColor();
		ImGui::PushStyleColor(ImGuiCol_Text, rainbowTextColor);
		PirateGeneratorLineUpUI* p = (PirateGeneratorLineUpUI*)getPirateGenerator();
		auto pirateDescs = p->CarouselPirateDescs;

		ImGui::SetNextWindowSize(ImVec2(580, 320), ImGuiCond_Once);
		ImGui::Begin("Pirate Customizer - Daived - v2.3", &engine::bInPirateGenerator, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
		ImGui::Text("Open Main Menu (Insert or Home) to be able to interact with this window");
		if (ImGui::Button("Close"))
		{
			engine::bInPirateGenerator = false;
		}

		ImGui::Separator();
		ImGui::Text("Pirate Seed: %d", tPirateSeedN);
		ImGui::SameLine();
		if (ImGui::Button("Reset"))
		{
			ZeroMemory(tPirateSeed, sizeof(tPirateSeed));
			tPirateSeedN = 0;
		}
		ImGui::InputText("Pirate Seed", tPirateSeed, IM_ARRAYSIZE(tPirateSeed));
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("If Pirate Seed = 0, it will affect the character's face, it may look diferent in-game.");
		}
		ImGui::SliderInt("Pirate Gender", &tPirateGender, 0, 2);
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("Male = 1 | Female = 2");
		}
		ImGui::SliderFloat("Pirate Age", &tPirateAge, 0.0f, 1.0f, "%.1f");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("Controls the character age");
		}
		ImGui::SliderInt("Pirate Ethnicity", &tPirateEthnicity, 1, 3);
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("Asian = 1 | Black = 2 | White = 3");
		}
		ImGui::SliderFloat("Pirate BodyType", &tPirateBodyType, 0.0f, 1.0f, "%.1f");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("Every 0.1 value change the body of your character");
		}
		ImGui::SliderFloat("Pirate BodyType Modifier", &tPirateBodyTypeModifier, 0.0f, 1.0f, "%.1f");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("It's used to tune the body type");
		}
		ImGui::SliderFloat("Pirate Dirtiness", &tPirateDirtiness, 0.0f, 1.0f, "%.2f");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("If = 0 it gets ignored");
		}
		ImGui::SliderFloat("Pirate Wonkiness", &tPirateWonkiness, 0.0f, 1.0f, "%.2f");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip("If = 0 it gets ignored");
		}
		for (UINT8 i = 0; i < pirateDescs.Count; i++)
		{
			char pBuf[0x64];
			int filterSize = sprintf_s(pBuf, sizeof(pBuf), tPirateSeed);
			if (filterSize != 0)
			{
				try
				{
					tPirateSeedN = std::stoi(std::string(pBuf));
				}
				catch (...)
				{
					tPirateSeedN = 0;
					tslog::info("Seed must be a valid int32 number.");
				}
			}
			else
			{
				tPirateSeedN = 0;
			}
			if (tPirateSeedN != 0) pirateDescs[i].Seed = tPirateSeedN;
			if (tPirateGender > 0) pirateDescs[i].Gender = tPirateGender;
			if (tPirateAge >= 0.0f) pirateDescs[i].Age = tPirateAge;
			if (tPirateEthnicity > 0) pirateDescs[i].Ethnicity = tPirateEthnicity;

			if (tPirateBodyType >= 0.0f) pirateDescs[i].BodyShape.NormalizedAngle = tPirateBodyType;
			if (tPirateBodyTypeModifier >= 0.0f) pirateDescs[i].BodyShape.RadialDistance = tPirateBodyTypeModifier;
			if (tPirateDirtiness >= 0.0f) pirateDescs[i].Dirtiness = tPirateDirtiness;
			if (tPirateWonkiness >= 0.0f) pirateDescs[i].Wonkiness = tPirateWonkiness;
		}
		ImGui::End();
		ImGui::PopStyleColor();
	}


	ImGui::Render();
	context->OMSetRenderTargets(1, &rtv, nullptr);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oDX11Present(swapChain, syncInterval, flags);
}

void clearDX11Objects()
{
	safe_release(rtv);
	safe_release(context);
	safe_release(device);
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
}

void UpdateImGuiCursor()
{
	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
	LPTSTR win32_cursor = IDC_ARROW;
	switch (imgui_cursor)
	{
	case ImGuiMouseCursor_Arrow:        win32_cursor = IDC_ARROW; break;
	case ImGuiMouseCursor_TextInput:    win32_cursor = IDC_IBEAM; break;
	case ImGuiMouseCursor_ResizeAll:    win32_cursor = IDC_SIZEALL; break;
	case ImGuiMouseCursor_ResizeEW:     win32_cursor = IDC_SIZEWE; break;
	case ImGuiMouseCursor_ResizeNS:     win32_cursor = IDC_SIZENS; break;
	case ImGuiMouseCursor_ResizeNESW:   win32_cursor = IDC_SIZENESW; break;
	case ImGuiMouseCursor_ResizeNWSE:   win32_cursor = IDC_SIZENWSE; break;
	case ImGuiMouseCursor_Hand:         win32_cursor = IDC_HAND; break;
	case ImGuiMouseCursor_NotAllowed:   win32_cursor = IDC_NO; break;
	}
	oSetCursor(::LoadCursor(NULL, win32_cursor));
}

HRESULT resizeBufferHook(IDXGISwapChain* swapChain, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags)
{
	clearDX11Objects();
	return oDX11ResizeBuffer(swapChain, bufferCount, width, height, newFormat, swapChainFlags);
}

bool getBaseModule()
{
	return K32GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(nullptr), &g_BaseModule, sizeof(MODULEINFO));
}

void getModule()
{
	mSize = 0;
	dxgi = (uintptr_t)ts::GetMBA("dxgi.dll", mSize);
	if (dxgi != NULL)
		tslog::log("d3d11.dll Address: 0x%x", (void*)dxgi);
}

uintptr_t findGWorld()
{
	if (dxgi == NULL) return 0;
	char Sign[] = "\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x88\x00\x00\x00\x00\x48\x85\xC9\x74\x06\x48\x8B\x49\x70";
	char Mask[] = "xxx????xxx????xxxxxxxxx";
	uintptr_t Addr = ts::Aobs(Sign, Mask, (uintptr_t)g_BaseModule.lpBaseOfDll, (uintptr_t)g_BaseModule.lpBaseOfDll + (uintptr_t)g_BaseModule.SizeOfImage);
	if (!Addr) return 0;
	auto offset = *reinterpret_cast<uint32_t*>(Addr + 3);
	Addr = Addr + 7 + offset;
	setConsoleColor(14);
	tslog::log("GWorld Address: 0x%x", Addr);
	return Addr;
}
uintptr_t findGObjects()
{
	if (dxgi == NULL) return 0;
	char Sign[] = "\x4C\x8B\x15\x00\x00\x00\x00\x8B\x5D\x10";
	char Mask[] = "xxx????xxx";
	uintptr_t Addr = ts::Aobs(Sign, Mask, (uintptr_t)g_BaseModule.lpBaseOfDll, (uintptr_t)g_BaseModule.lpBaseOfDll + (uintptr_t)g_BaseModule.SizeOfImage);
	if (!Addr) return 0;
	auto offset = *reinterpret_cast<uint32_t*>(Addr + 3);
	Addr = Addr + 7 + offset - 0x8;
	setConsoleColor(14);
	tslog::log("GObjects Address: 0x%x", Addr);
	return Addr;
}
uintptr_t findGNames()
{
	if (dxgi == NULL) return 0;
	char Sign[] = "\x48\x89\x1D\x00\x00\x00\x00\x48\x8B\x5C\x24\x20\x48\x83\xC4\x28";
	char Mask[] = "xxx????xxxxxxxxx";
	uintptr_t Addr = ts::Aobs(Sign, Mask, (uintptr_t)g_BaseModule.lpBaseOfDll, (uintptr_t)g_BaseModule.lpBaseOfDll + (uintptr_t)g_BaseModule.SizeOfImage);
	if (!Addr) return 0;
	auto offset = *reinterpret_cast<uint32_t*>(Addr + 3);
	Addr = Addr + 7 + offset;
	setConsoleColor(14);
	tslog::log("GNames Address: 0x%x", Addr);
	return Addr;
}

HCURSOR WINAPI hkSetCursor(HCURSOR hCursor)
{
	if (g_ShowMenu)
	{
		return NULL;
	}
	return oSetCursor(hCursor);
}

BOOL WINAPI hkSetCursorPos(int x, int y)
{
	if (g_ShowMenu)
	{
		return false;
	}
	return oSetCursorPos(x, y);
}

SHORT SafeGetAsyncKeyState(int vKey)
{
	return oGetAsyncKeyState ? oGetAsyncKeyState(vKey) : GetAsyncKeyState(vKey);
}

SHORT WINAPI hkGetAsyncKeyState(int vKey)
{
	if (g_ShowMenu)
	{
		return 0;
	}
	return oGetAsyncKeyState(vKey);
}

BOOL WINAPI hkGetKeyboardState(PBYTE lpKeyState)
{
	if (g_ShowMenu)
	{
		if (lpKeyState)
			ZeroMemory(lpKeyState, 256);
		return TRUE;
	}
	return oGetKeyboardState(lpKeyState);
}

int WINAPI hkShowCursor(BOOL bShow)
{
	if (g_ShowMenu)
	{
		return 1;
	}
	return oShowCursor(bShow);
}

static int g_CursorShowNudges = 0;

void OnMenuVisibilityChanged(bool nowOpen)
{
	if (!oShowCursor)
		return;

	if (nowOpen)
	{
		g_CursorShowNudges = 0;
		int guard = 0;
		while (oShowCursor(TRUE) < 0 && guard < 64)
		{
			++g_CursorShowNudges;
			++guard;
		}
	}
	else
	{
		while (g_CursorShowNudges > 0)
		{
			oShowCursor(FALSE);
			--g_CursorShowNudges;
		}
	}
}