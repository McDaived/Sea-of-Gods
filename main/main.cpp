#include "../main/main.h"
#include "../DirectX11.h"
bool Client = false;
bool ESP = false;
bool Islands = false;
bool AimBot = false;
bool Global = false;
bool Save = false;
bool Debug = false;


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

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		if (ChecktDirectXVersion(DirectXVersion.D3D11) == true) {
			g_hInstance = hModule;
			CreateThread(0, 0, MainThread, 0, 0, 0);
		}
		break;
	case DLL_PROCESS_DETACH:
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	default:
		break;
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
	tslog::log("Module Device: 0x%p", reinterpret_cast<void*>(g_hInstance));
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
		tslog::log("Module Base Address: %p", (void*)g_BaseModule.lpBaseOfDll);
		setConsoleColor(7);
	}

	setConsoleColor(14);
	tslog::verbose("Starting main initialization..");
	getModule();
	bool WindowFocus = false;
	while (WindowFocus == false) {
		DWORD ForegroundWindowProcessID;
		GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);
		if (GetCurrentProcessId() == ForegroundWindowProcessID) {

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
			tslog::log("Present address: %p", (void*)&presentHook);
			CreateHook(13, (void**)&oDX11ResizeBuffer, resizeBufferHook);
			setConsoleColor(10);
			tslog::log("ResizeBuffers address: %p", (void*)&presentHook);
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

	if (MH_CreateHook(SetCursor, hkSetCursor, reinterpret_cast<LPVOID*>(&oSetCursor)) != MH_OK) {
		setConsoleColor(12);
		tslog::critical("SetCursor hook creation failed.");
		end();
		return false;
	}

	if (MH_CreateHook(SetCursorPos, hkSetCursorPos, reinterpret_cast<LPVOID*>(&oSetCursorPos)) != MH_OK) {
		setConsoleColor(12);
		tslog::critical("SetCursorPos hook creation failed.");
		end();
		return false;
	}

	if (MH_EnableHook(SetCursor) != MH_OK || MH_EnableHook(SetCursorPos) != MH_OK) {
		setConsoleColor(12);
		tslog::critical("Failed to enable hooks.");
		end();
		return false;
	}

	g_Running = true;

	run();
	return true;
}

bool end()
{
	setConsoleColor(13);
	tslog::log("Exit..");
	tslog::log("Unload Main Menu..");


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

	if (oWindowProc)
		SetWindowLongPtr(WindowHwnd, GWLP_WNDPROC, (LONG_PTR)oWindowProc);

	DisableAll();
	EngineShutdown();
	clearDX11Objects();
	tslog::shutdown();
	MH_Uninitialize();

	FreeLibraryAndExitThread(g_hInstance, 0);
	return true;
}


void run()
{
	setConsoleColor(14);
	tslog::verbose("Starting main loop..\n");
	while (g_Running)
	{
		if ((GetAsyncKeyState(VK_INSERT) & 1) || (GetAsyncKeyState(VK_HOME) & 1))
		{
			g_ShowMenu = !g_ShowMenu;
		}
		if (GetAsyncKeyState(VK_END) & 1)
		{
			g_Running = false;
		}
		Sleep(20);
	}
	end();
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
		WindowHwnd = window;

		if (oWindowProc && WindowHwnd)
		{
			SetWindowLongPtrA(WindowHwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(oWindowProc));
			oWindowProc = nullptr;
		}
		if (WindowHwnd) {
			oWindowProc = (WNDPROC)SetWindowLongPtr(WindowHwnd, GWLP_WNDPROC, (LONG_PTR)windowProcHookEx);
			setConsoleColor(14);
			tslog::log("Original WndProc function: %p", oWindowProc);
			setConsoleColor(10);
			tslog::log("Initialization and hooking successful!\n");
			setConsoleColor(9);
			tslog::log("Menu Keys: [Ins] -> Open Menu - [End] -> Unload Menu\n");
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
		ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arialbd.ttf", 14, nullptr);
		ImFontConfig config;
		config.MergeMode = true;
		config.PixelSnapH = true;
		config.OversampleH = 3;
		config.OversampleV = 3;
		io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 19.0f, &config, icons_ranges);
		io.Fonts->Build();

		ImGui_ImplWin32_Init(WindowHwnd);
		ImGui_ImplDX11_Init(device, context);
		ImGui_ImplDX11_CreateDeviceObjects();
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();


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
		ImVec4 rainbowTextColor = GetRainbowColor();
		ImGui::PushStyleColor(ImGuiCol_Text, rainbowTextColor);
		ImGuiStyle* style = &ImGui::GetStyle();
		style->WindowPadding = ImVec2(10, 10);
		style->WindowRounding = 5.0f;
		style->FramePadding = ImVec2(6, 4);
		style->FrameRounding = 4.0f;
		style->ItemSpacing = ImVec2(8, 6);
		style->ItemInnerSpacing = ImVec2(6, 4);
		style->IndentSpacing = 20.0f;
		style->ScrollbarSize = 14.0f;
		style->ScrollbarRounding = 9.0f;
		style->GrabMinSize = 5.0f;
		style->GrabRounding = 3.0f;
		style->WindowBorderSize = 1;
		style->WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style->FramePadding = ImVec2(4, 3);
		style->Colors[ImGuiCol_TitleBg] = ImColor(0, 0, 0, 255);
		style->Colors[ImGuiCol_TitleBgActive] = ImColor(255, 128, 0, 255);
		style->Colors[ImGuiCol_Button] = ImColor(30, 30, 30, 255);
		style->Colors[ImGuiCol_ButtonActive] = ImColor(255, 128, 0, 255);
		style->Colors[ImGuiCol_ButtonHovered] = ImColor(40, 40, 40, 255);
		style->Colors[ImGuiCol_Separator] = ImColor(60, 60, 60, 255);
		style->Colors[ImGuiCol_SeparatorActive] = ImColor(80, 80, 80, 255);
		style->Colors[ImGuiCol_SeparatorHovered] = ImColor(70, 70, 70, 255);
		style->Colors[ImGuiCol_Tab] = ImColor(0, 0, 0, 255);
		style->Colors[ImGuiCol_TabHovered] = ImColor(255, 128, 0, 255);
		style->Colors[ImGuiCol_TabActive] = ImColor(60, 60, 60, 255);
		style->Colors[ImGuiCol_SliderGrab] = ImColor(80, 80, 80, 255);
		style->Colors[ImGuiCol_SliderGrabActive] = ImColor(100, 100, 100, 255);
		style->Colors[ImGuiCol_MenuBarBg] = ImColor(30, 30, 30, 255);
		style->Colors[ImGuiCol_FrameBg] = ImColor(0, 0, 0, 255);
		style->Colors[ImGuiCol_FrameBgActive] = ImColor(25, 25, 25, 255);
		style->Colors[ImGuiCol_FrameBgHovered] = ImColor(22, 22, 22, 255);
		style->Colors[ImGuiCol_Header] = ImColor(0, 0, 0, 0);
		style->Colors[ImGuiCol_HeaderActive] = ImColor(0, 0, 0, 0);
		style->Colors[ImGuiCol_HeaderHovered] = ImColor(30, 30, 30, 255);
		style->Colors[ImGuiCol_ResizeGrip] = ImColor(0.00f, 0.00f, 0.00f, 0.00f);
		style->Colors[ImGuiCol_ResizeGripHovered] = ImColor(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGripActive] = ImColor(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_PlotLines] = ImColor(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotLinesHovered] = ImColor(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_PlotHistogram] = ImColor(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotHistogramHovered] = ImColor(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_TextSelectedBg] = ImColor(0.25f, 1.00f, 0.00f, 0.43f);
		style->Colors[ImGuiCol_CheckMark] = ImColor(255, 128, 0, 255);


		if (ImGui::Begin("Vertical Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
		{
			ImVec2 windowSize(170, 337);
			ImGui::SetWindowSize(windowSize);
			ImGui::SetWindowPos(ImVec2(0, 0));

			std::vector<std::string> menuItems = { ICON_FA_USER " Client", ICON_FA_PLUS_CIRCLE " ESP", ICON_FA_EYE " Islands ESP", ICON_FA_CROSSHAIRS " AimBot", ICON_FA_GLOBE " Global", ICON_FA_WRENCH " Debug", ICON_FA_SAVE " Save" };

			ImVec2 buttonSize(150, 40);

			for (const auto& item : menuItems)
			{


				if (ImGui::Button(item.c_str(), buttonSize))
				{


					if (item.find("Client") != std::string::npos)
					{
						Client = true;
						ESP = false;
						Islands = false;
						AimBot = false;
						Global = false;
						Debug = false;
						Save = false;
					}
					if (item.find("ESP") != std::string::npos)
					{
						Client = false;
						ESP = true;
						Islands = false;
						AimBot = false;
						Global = false;
						Debug = false;
						Save = false;
					}
					if (item.find("Islands ESP") != std::string::npos)
					{
						Client = false;
						ESP = false;
						Islands = true;
						AimBot = false;
						Global = false;
						Debug = false;
						Save = false;
					}
					if (item.find("AimBot") != std::string::npos)
					{
						Client = false;
						ESP = false;
						Islands = false;
						AimBot = true;
						Global = false;
						Debug = false;
						Save = false;
					}
					if (item.find("Global") != std::string::npos)
					{
						Client = false;
						ESP = false;
						Islands = false;
						AimBot = false;
						Global = true;
						Debug = false;
						Save = false;
					}
					if (item.find("Debug") != std::string::npos)
					{
						Client = false;
						ESP = false;
						Islands = false;
						AimBot = false;
						Global = false;
						Debug = true;
						Save = false;
					}
					if (item.find("Save") != std::string::npos)
					{
						Client = false;
						ESP = false;
						Islands = false;
						AimBot = false;
						Global = false;
						Debug = false;
						Save = true;
					}
				}

			}


		}
		ImGui::End();

		if (Client)
		{

			ImGui::SetNextWindowSize(ImVec2(400, 530));
			ImGui::Begin("Sea Of Gods - Daived - v2.2", &Client, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

			ImGui::Text("Client");
			if (ImGui::BeginChild("Client Activation", ImVec2(200.f, 50.f), false, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.client.enable);

			}
			ImGui::EndChild();

			if (ImGui::BeginChild("Client", ImVec2(0.f, 0.f), false, 0))
			{
				enum CrosshairType {
					None,
					Circle,
					Cross
				};
				int crosshairIndex = static_cast<int>(Config::cfg.client.crosshairType);
				const char* crosshair[] = { "None", "Circle", "Cross" };
				ImGui::Checkbox("Player FOV", &Config::cfg.client.fovEnable);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable Field of View (FOV).");
				ImGui::SliderFloat("FOV Value", &Config::cfg.client.fov, 60.f, 150.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Field of View (FOV) value for players.");
				ImGui::Checkbox("Spyglass FOV (Right Click)", &Config::cfg.client.spyRClickMode);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Zoom in without the lens.");
				ImGui::SliderFloat("Spyglass", &Config::cfg.client.spyglassFovMul, 1.f, 25.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Field of View (FOV) value for spyglass.");
				ImGui::SliderFloat("Sniper", &Config::cfg.client.sniperFovMul, 1.f, 25.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Field of View (FOV) value for guns.");

				ImGui::Dummy(ImVec2(0.0f, 10.0f));

				ImGui::Checkbox("Compass", &Config::cfg.client.bCompass);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the Compass.");
				ImGui::Checkbox("Oxygen Level", &Config::cfg.client.oxygen);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the percentage of oxygen depletion.");

				ImGui::Dummy(ImVec2(0.0f, 10.0f));

				ImGui::Checkbox("CrossHair", &Config::cfg.client.crosshair);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable crosshair.");
				ImGui::SliderFloat("CrossHair Size", &Config::cfg.client.crosshairSize, 1.f, 50.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Size value for crosshair.");
				ImGui::SliderFloat("CrossHair Thickness", &Config::cfg.client.crosshairThickness, 1.f, 50.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Thickness value for crosshair.");
				ImGui::ColorEdit3("CrossHair Color", &Config::cfg.client.crosshairColor.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Add color for crosshair.");
				ImGui::SliderInt("Crosshair Type", &crosshairIndex, 0, IM_ARRAYSIZE(crosshair) - 1, crosshair[crosshairIndex]);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Select the type of crosshair.");
				Config::cfg.client.crosshairType = static_cast<decltype(Config::cfg.client.crosshairType)>(crosshairIndex);

				ImGui::Dummy(ImVec2(0.0f, 10.0f));

				ImGui::Checkbox("Custom Time of Day", &Config::cfg.client.bCustomTOD);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable custom time of day.");
				ImGui::SliderFloat("Time of Day", &Config::cfg.client.customTOD, 1.f, 24.f, "%.0f:00");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Change time of day (Only client).");


			}
			ImGui::EndChild();

			ImGui::End();
		}


		if (ESP)
		{
			ImGui::SetNextWindowSize(ImVec2(1225, 490));
			ImGui::Begin("Sea Of Gods - Daived - v2.2", &ESP, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
			ImGui::Text("ESP");
			if (ImGui::BeginChild("ESP Activation", ImVec2(200.f, 50.f), false, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.esp.enable);


			}
			ImGui::EndChild();

			ImGui::Columns(3, "ActorEspCol", false);

			ImGui::Text("Players ESP");
			if (ImGui::BeginChild("Players", ImVec2(0.f, 0.f), true, 0))
			{
				const char* bars[] = { "None", "Left", "Right", "Bottom", "Top" };
				const char* boxes[] = { "None", "2D Box", "3D Box" };
				ImGui::Checkbox("Enable", &Config::cfg.esp.players.enable);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable Player ESP.");

				ImGui::Separator();

				ImGui::Checkbox("Players Name", &Config::cfg.esp.players.bName);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Draw name for players.");
				ImGui::Checkbox("Player Bones", &Config::cfg.esp.players.bSkeleton);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Draw bones for player body");
				ImGui::Checkbox("Teammate", &Config::cfg.esp.players.team);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show ESP your teammates.");

				ImGui::Dummy(ImVec2(0.0f, 10.0f));

				ImGui::Combo("Player ESP Box", reinterpret_cast<int*>(&Config::cfg.esp.players.boxType), boxes, IM_ARRAYSIZE(boxes));
				ImGui::Combo("Player Health Bar", reinterpret_cast<int*>(&Config::cfg.esp.players.barType), bars, IM_ARRAYSIZE(bars));
				
				ImGui::Dummy(ImVec2(0.0f, 10.0f));
				
				ImGui::SliderFloat("Players Distance", &Config::cfg.esp.players.renderDistance, 1.f, 5000.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for a player to appear.");
				ImGui::ColorEdit3("Visible Color", &Config::cfg.esp.players.colorVisible.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the player that is visible to you.");
				ImGui::ColorEdit3("Invisible Color", &Config::cfg.esp.players.colorInvisible.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the player that is invisible to you.");

				ImGui::Dummy(ImVec2(0.0f, 10.0f));

				ImGui::Checkbox("Tracer Line", &Config::cfg.esp.players.tracers);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show a line to the player.");
				ImGui::SliderFloat("Thickness Line", &Config::cfg.esp.players.tracersThickness, 1.f, 10.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Thickness of the line.");

			}
			ImGui::EndChild();

			ImGui::NextColumn();

			ImGui::Text("Skeletons ESP");
			if (ImGui::BeginChild("Skeletons ESP", ImVec2(0.f, 0.f), true, 0))
			{
				const char* bars[] = { "None", "Left", "Right", "Bottom", "Top" };
				const char* boxes[] = { "None", "2D Box", "3D Box" };
				ImGui::Checkbox("Enable", &Config::cfg.esp.skeletons.enable);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable skeletons ESP.");

				ImGui::Separator();

				ImGui::Checkbox("Skeletons Bones", &Config::cfg.esp.skeletons.bSkeleton);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Draw bones for skeletons body");

				ImGui::Dummy(ImVec2(0.0f, 10.0f));

				ImGui::Combo("Skele ESP Box", reinterpret_cast<int*>(&Config::cfg.esp.skeletons.boxType), boxes, IM_ARRAYSIZE(boxes));
				ImGui::Combo("Skele Health Bar", reinterpret_cast<int*>(&Config::cfg.esp.skeletons.barType), bars, IM_ARRAYSIZE(bars));

				ImGui::Dummy(ImVec2(0.0f, 10.0f));

				ImGui::SliderFloat("Skeletons Distance", &Config::cfg.esp.skeletons.renderDistance, 1.f, 1000.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for a skeleton to appear.");
				ImGui::ColorEdit3("Skeletons Color", &Config::cfg.esp.skeletons.color.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color ESP of the skeleton.");
				ImGui::ColorEdit3("Visible Color", &Config::cfg.esp.skeletons.colorVis.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the skeletons that is visible to you.");
				ImGui::ColorEdit3("Invisible Color", &Config::cfg.esp.skeletons.colorInv.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the skeletons that is invisible to you.");
			}
			ImGui::EndChild();


			ImGui::NextColumn();

			ImGui::Text("Ships ESP");
			if (ImGui::BeginChild("Ships ESP", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.esp.ships.enable);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable ships ESP.");

				ImGui::Separator();

				ImGui::SliderFloat("Ships Distance", &Config::cfg.esp.ships.renderDistance, 1.f, 5000.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for a ship to appear.");
				ImGui::ColorEdit3("Ships Color", &Config::cfg.esp.ships.color.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color ESP of the ship.");
				ImGui::Checkbox("Show Holes", &Config::cfg.esp.ships.holes);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show holes in the ship.");
				ImGui::Checkbox("Skeletons", &Config::cfg.esp.ships.skeletons);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("ESP skeletons ship.");
				ImGui::Checkbox("Ghost Ships", &Config::cfg.esp.ships.ghosts);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("ESP ghost ships.");
				ImGui::Checkbox("Ladders Position", &Config::cfg.esp.ships.showLadders);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the position of the ladders to any ship.");

				ImGui::Dummy(ImVec2(0.0f, 10.0f));

				ImGui::Checkbox("Ship Trajectory [may crash]", &Config::cfg.esp.ships.shipTray);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the trajectory of the ship.");
				ImGui::SliderFloat("Thickness", &Config::cfg.esp.ships.shipTrayThickness, 0.f, 1000.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Thickness of the trajectory.");
				ImGui::SliderFloat("Height", &Config::cfg.esp.ships.shipTrayHeight, -10.f, 20.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Height of the trajectory.");
				ImGui::ColorEdit3("Trajectory Color", &Config::cfg.esp.ships.shipTrayCol.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Color of the trajectory.");


			}
			ImGui::EndChild();

			ImGui::Columns();

			ImGui::End();
		}


		if (Islands)
		{
			ImGui::SetNextWindowSize(ImVec2(1300, 624));
			ImGui::Begin("Sea Of Gods - Daived - v2.2", &Islands, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
			ImGui::Text("Islands ESP");
			if (ImGui::BeginChild("Islands Activation", ImVec2(200.f, 50.f), false, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.esp.enable);


			}
			ImGui::EndChild();

			ImGui::Columns(3, "ObfEspCol", false);

			ImGui::Text("Islands Items");
			if (ImGui::BeginChild("Island ESP", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.esp.islands.enable);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable island ESP.");

				ImGui::Separator();

				ImGui::SliderFloat("Size", &Config::cfg.esp.islands.size, 1.f, 10.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Name size of the island.");
				ImGui::SliderFloat("Islands Distance", &Config::cfg.esp.islands.renderDistance, 1.f, 10000.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for an islands names to appear.");
				ImGui::ColorEdit3("Islands Names Color", &Config::cfg.esp.islands.color.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the islands names.");

				ImGui::Separator();

				ImGui::Checkbox("Map Marks", &Config::cfg.esp.islands.marks);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show marks location on the island.");
				ImGui::SliderFloat("Marks Distance", &Config::cfg.esp.islands.marksRenderDistance, 1.f, 10000.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for a marks location to appear.");
				ImGui::ColorEdit3("Marks Color", &Config::cfg.esp.islands.marksColor.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the marks location.");

				ImGui::Separator();

				ImGui::Checkbox("Vaults", &Config::cfg.esp.islands.vaults);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the vaults location on the island when open.");
				ImGui::SliderFloat("Vaults Distance", &Config::cfg.esp.islands.vaultsRenderDistance, 1.f, 10000.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for a vaults location to appear.");
				ImGui::ColorEdit3("Vaults Color", &Config::cfg.esp.islands.vaultsColor.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the vaults location.");

				ImGui::Separator();

				ImGui::Checkbox("Barrels", &Config::cfg.esp.islands.barrels);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the barrels location on the island.");
				ImGui::Checkbox("Barrels Items", &Config::cfg.esp.islands.barrelspeek);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the items in the barrels.");
				ImGui::Checkbox("(R) Key Toggle", &Config::cfg.esp.islands.barrelstoggle);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Toggle the barrels items with the (R) key.");
				ImGui::ColorEdit3("Barrels Color", &Config::cfg.esp.islands.barrelsColor.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the barrels location.");

				ImGui::Separator();

				ImGui::Checkbox("Ammo Chests", &Config::cfg.esp.islands.ammoChest);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the ammo chests location.");
				ImGui::SliderFloat("Ammo Distance", &Config::cfg.esp.islands.ammoChestRenderDistance, 1.f, 10000.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for a ammo chests location to appear.");
				ImGui::ColorEdit3("Ammo Color", &Config::cfg.esp.islands.ammoChestColor.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the ammo chests location.");


			}
			ImGui::EndChild();

			ImGui::NextColumn();

			ImGui::Text("Items");
			if (ImGui::BeginChild("Items ESP", ImVec2(0.f, 0.f), true, 0))
			{
				const char* boxes[] = { "None", "2D Box", "3D Box" };
				ImGui::Checkbox("Enable Items", &Config::cfg.esp.items.enable);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable items ESP.");
				ImGui::SliderFloat("Items Distance", &Config::cfg.esp.items.renderDistance, 1.f, 3000.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for an items to appear.");
				ImGui::ColorEdit3("Items Color", &Config::cfg.esp.items.color.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the items.");
				ImGui::Checkbox("(R) key Toggle Names", &Config::cfg.esp.items.nameToggle);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Toggle the items names with the (R) key.");

				ImGui::Separator();

				ImGui::Checkbox("Enable Animals", &Config::cfg.esp.items.animals);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the animals location on the island.");
				ImGui::Checkbox("Animals Names", &Config::cfg.esp.items.bName);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the animals names on the island.");
				ImGui::SliderFloat("Animals Distance", &Config::cfg.esp.items.animalsRenderDistance, 1.f, 500.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for a animals location to appear.");
				ImGui::ColorEdit3("Animals Color", &Config::cfg.esp.items.animalsColor.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the animals location.");
				ImGui::Combo("Animals ESP Box ", reinterpret_cast<int*>(&Config::cfg.esp.items.boxType), boxes, IM_ARRAYSIZE(boxes));

				ImGui::Separator();

				ImGui::Checkbox("Enable Sharks", &Config::cfg.esp.items.sharks);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable sharks ESP include (megalodon).");
				ImGui::Checkbox("Sharks bones", &Config::cfg.esp.items.bSkeleton);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable sharks bones include (megalodon).");
				ImGui::SliderFloat("Sharks Distance", &Config::cfg.esp.items.sharksRenderDistance, 1.f, 500.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for a sharks ESP to appear.");
				ImGui::ColorEdit3("Sharks Color", &Config::cfg.esp.items.sharksColor.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the sharks ESP and bones.");

				ImGui::Separator();

				ImGui::Checkbox("Lost Cargo", &Config::cfg.esp.items.lostCargo);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the lost cargo location.");
				ImGui::ColorEdit3("Cargo Color", &Config::cfg.esp.items.cluesColor.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the lost cargo location.");


			}
			ImGui::EndChild();

			ImGui::NextColumn();

			ImGui::Text("World");
			if (ImGui::BeginChild("Others ESP", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.esp.others.enable);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable others ESP.");

				ImGui::Separator();

				ImGui::Checkbox("Shipwrecks", &Config::cfg.esp.others.shipwrecks);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the shipwrecks location in the sea.");
				ImGui::SliderFloat("Shipwrecks Distance", &Config::cfg.esp.others.shipwrecksRenderDistance, 1.f, 10000.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for a shipwrecks location to appear.");
				ImGui::ColorEdit3("Shipwrecks Color", &Config::cfg.esp.others.shipwrecksColor.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the shipwrecks location.");

				ImGui::Separator();

				ImGui::Checkbox("Mermaids", &Config::cfg.esp.others.mermaids);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the mermaids location in the sea.");
				ImGui::SliderFloat("Mermaids Distance", &Config::cfg.esp.others.mermaidsRenderDistance, 1.f, 1000.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for a mermaids location to appear.");
				ImGui::ColorEdit3("Mermaids Color", &Config::cfg.esp.others.mermaidsColor.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the mermaids location.");

				ImGui::Separator();

				ImGui::Checkbox("Rowboats", &Config::cfg.esp.others.rowboats);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the rowboats location.");
				ImGui::SliderFloat("Rowboats Distance", &Config::cfg.esp.others.rowboatsRenderDistance, 1.f, 3500.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for a rowboats location to appear.");
				ImGui::ColorEdit3("Rowboats Color", &Config::cfg.esp.others.rowboatsColor.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the rowboats location.");

				ImGui::Separator();

				ImGui::Checkbox("World Events", &Config::cfg.esp.others.events);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the world events.");
				ImGui::SliderFloat("Events Distance", &Config::cfg.esp.others.eventsRenderDistance, 1.f, 10000.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for a world events to appear.");
				ImGui::ColorEdit3("Events Color", &Config::cfg.esp.others.eventsColor.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the world events.");


			}
			ImGui::EndChild();

			ImGui::Columns();

			ImGui::End();
		}


		if (AimBot)
		{
			ImGui::SetNextWindowSize(ImVec2(890, 510));
			ImGui::Begin("Sea Of Gods - Daived - v2.2", &AimBot, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
			ImGui::Text("AimBot");
			if (ImGui::BeginChild("AimBot Activation", ImVec2(200.f, 50.f), false, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.aim.enable);


			}
			ImGui::EndChild();

			ImGui::Columns(3, "AimBot", false);

			ImGui::Text("Weapons AimBot");
			if (ImGui::BeginChild("Weapons AimBot", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.aim.weapon.enable);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable weapons aimbot.");

				ImGui::Separator();

				ImGui::SliderFloat("Pitch", &Config::cfg.aim.weapon.fPitch, 1.f, 100.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The ejection angle percentage, I recommend setting it to the maximum.");
				ImGui::SliderFloat("Yaw", &Config::cfg.aim.weapon.fYaw, 1.f, 100.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The sensitivity of vision, I recommend setting it to the maximum.");
				ImGui::SliderFloat("Smoothness", &Config::cfg.aim.weapon.smooth, 1.f, 10.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Value smoothness capture of the enemy.");
				ImGui::SliderFloat("Height", &Config::cfg.aim.weapon.height, 1.f, 100.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The height of the aimbot (Head | Chest | Body).");

				ImGui::Dummy(ImVec2(0.0f, 10.0f));

				ImGui::Checkbox("Players", &Config::cfg.aim.weapon.players);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable aimbot to players.");
				ImGui::Checkbox("Skeletons", &Config::cfg.aim.weapon.skeletons);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable aimbot to skeletons.");
				ImGui::Checkbox("Gunpowder", &Config::cfg.aim.weapon.kegs);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable aimbot to gunpowder.");
				ImGui::Checkbox("Instant Shot", &Config::cfg.aim.weapon.trigger);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Automatic throw while aiming at the target.");
				ImGui::Checkbox("Visible Only", &Config::cfg.aim.weapon.visibleOnly);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable Aimbot if the enemy is visible.");
				ImGui::Checkbox("Beta Aim (F1)", &Config::cfg.aim.others.rage);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("New mathematical aiming accuracy, (beta version) it will be developed in the future, may stuck in last location you aimed (it works when you use a weapon).");
				ImGui::Checkbox("Fast Reload", &Config::cfg.aim.weapon.fastweapon);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Reducing delay while reload and scoping");


			}
			ImGui::EndChild();

			ImGui::NextColumn();

			ImGui::Text("Cannon AimBot");
			if (ImGui::BeginChild("Cannon AimBot", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.aim.cannon.enable);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable cannon aimbot.");

				ImGui::Separator();

				ImGui::SliderFloat("Pitch", &Config::cfg.aim.cannon.fPitch, 1.f, 100.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The ejection angle percentage, I recommend setting it to the maximum.");
				ImGui::SliderFloat("Yaw", &Config::cfg.aim.cannon.fYaw, 1.f, 100.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The sensitivity of vision, I recommend setting it to the maximum.");

				ImGui::Dummy(ImVec2(0.0f, 10.0f));

				ImGui::Checkbox("Instant Shot", &Config::cfg.aim.cannon.instant);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Automatic throw while aiming at the target.");
				ImGui::Checkbox("Chain Shots (F1)", &Config::cfg.aim.cannon.chains);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable aimbot to chain shots.");
				ImGui::Checkbox("Players", &Config::cfg.aim.cannon.players);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Aim at the player who is on the ship or island (Inaccurate if the player is moving).");
				ImGui::Checkbox("Skeletons", &Config::cfg.aim.cannon.skeletons);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Aim at the skeletons who is on the ship or island.");
				ImGui::Checkbox("Ghost Ships", &Config::cfg.aim.cannon.ghostShips);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Aim at the ghost ships.");
				ImGui::Checkbox("Cannon Pre-Fire (F4)", &Config::cfg.aim.cannon.drawPred);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Draw trajectory only for cannon ball to know where to aim.");
				ImGui::Checkbox("Damage Zone (F3)", &Config::cfg.aim.cannon.lowAim);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Aiming on damage zone and making holes sequentially (it doesn't aim at the same place).");
				ImGui::Checkbox("Player Deck (F2)", &Config::cfg.aim.cannon.deckshots);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The player is thrown onto the enemy ship.");
				ImGui::Checkbox("Visible Only", &Config::cfg.aim.cannon.visibleOnly);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable Aimbot if the ship is visible.");
				ImGui::Checkbox("AimBot V2", &Config::cfg.aim.cannon.improvedVersion);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Improved version of the aimbot (It is not recommended to use it).");
				


			}
			ImGui::EndChild();

			ImGui::NextColumn();

			ImGui::Text("Harpoon AimBot");
			if (ImGui::BeginChild("Harpoon AimBot", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.aim.harpoon.enable);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable harpoon aimbot.");

				ImGui::Separator();

				ImGui::Checkbox("Visible only", &Config::cfg.aim.harpoon.visibleOnly);
			}
			ImGui::EndChild();

			ImGui::Columns();

			ImGui::End();
		}
		if (Global)
		{
			ImGui::SetNextWindowSize(ImVec2(450, 480));
			ImGui::Begin("Sea Of Gods - Daived - v2.2", &Global, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
			ImGui::Text("Global");
			if (ImGui::BeginChild("Global Activation", ImVec2(200.f, 50.f), false, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.game.enable);


			}
			ImGui::EndChild();

			if (ImGui::BeginChild("Global", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::SliderFloat("Render Text Size", &Config::cfg.dev.renderTextSizeFactor, 0.1f, 3.0f, "%.2f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Size of the render appearing in the overlay.");
				ImGui::Separator();
				ImGui::Checkbox("Map Pins", &Config::cfg.game.mapPins);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the location of the mark that was placed on the table.");
				ImGui::Checkbox("Players List", &Config::cfg.game.playerList);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the list of players in the server.");
				ImGui::Checkbox("Cook Tracker", &Config::cfg.game.cooking);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the cooking time of the food.");
				ImGui::Checkbox("Anti AFK", &Config::cfg.game.noIdleKick);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Prevent the game from kicking you for being idle.");
				ImGui::Checkbox("Walk Underwate (C) key ", &Config::cfg.game.walkUnderwater);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enable walk underwater.");
				ImGui::Checkbox("Sunk Location", &Config::cfg.game.showSunk);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the location of the sunken ship.");
				ImGui::Checkbox("Ship Info", &Config::cfg.game.shipInfo);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show status about ships.");
				ImGui::Checkbox("Bunny Hop", &Config::cfg.game.b_bunnyhop);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Bunny Hop, spam jumps.");
				ImGui::ColorEdit3("Sunk Location Color", &Config::cfg.game.sunkColor.x, 0);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("The color of the sunken ship location.");
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
			ImGui::EndChild();

			ImGui::End();
		}
		if (Debug)
		{
			ImGui::SetNextWindowSize(ImVec2(640, 320));
			ImGui::Begin("Sea Of Gods - Daived - v2.2", &Global, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
			ImGui::Text("Debug");
			if (ImGui::BeginChild("Debug Activation", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::Checkbox("Print errors codes in console", &Config::cfg.dev.printErrorCodes);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Print errors codes in console like crash or feature doesn't work.");
				ImGui::Checkbox("Intercept Process Event", &Config::cfg.dev.interceptProcessEvent);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Don't use it if you don't know what it does.");
				ImGui::Checkbox("Print RPC calls in console", &Config::cfg.dev.printRPCCalls);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Don't use it if you don't know what it does.");
				ImGui::Separator();
				ImGui::Checkbox("Show Debug Names", &Config::cfg.dev.debugNames);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Show the names of the objects in the game.");
				ImGui::Text("Filter Keyword");
				ImGui::InputText("Keyword", Config::cfg.dev.debugNamesFilter, IM_ARRAYSIZE(Config::cfg.dev.debugNamesFilter));
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Filter the keyword for the debug names.");
				ImGui::SliderInt("Debug Names Text Size", &Config::cfg.dev.debugNamesTextSize, 1, 50);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Size of the debug names.");
				ImGui::SliderFloat("Debug Names Render Distance", &Config::cfg.dev.debugNamesRenderDistance, 1.f, 1000.f, "%.0f");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Distance for a debug names to appear.");
				ImGui::Checkbox("Dummy Boolean (Debugging purposes)", &Config::cfg.dev.bDummy);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Don't use it if you don't know what it does.");

			}
			ImGui::EndChild();

			ImGui::End();
		}
		if (Save)
		{
			ImGui::SetNextWindowSize(ImVec2(270, 95));
			ImGui::Begin("Sea Of Gods - Daived - v2.2", &Save, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
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
			ImGui::SameLine();

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
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			ImGui::End();
		}

		ImGui::PopStyleColor();
	}

	if (engine::bInPirateGenerator)
	{
		ImVec4 rainbowTextColor = GetRainbowColor();
		ImGui::PushStyleColor(ImGuiCol_Text, rainbowTextColor);
		PirateGeneratorLineUpUI* p = (PirateGeneratorLineUpUI*)getPirateGenerator();
		auto pirateDescs = p->CarouselPirateDescs;

		ImGui::SetNextWindowSize(ImVec2(580, 320), ImGuiCond_Once);
		ImGui::Begin("Pirate Customizer - Daived - v2.2", &engine::bInPirateGenerator, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
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

LRESULT CALLBACK windowProcHookEx(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (g_ShowMenu)
	{
		if (ImGui::GetCurrentContext() != NULL)
		{
			ImGuiIO& io = ImGui::GetIO();
			switch (msg)
			{
			case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
			case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
			{
				int button = 0;
				if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = 0; }
				if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = 1; }
				if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = 2; }
				if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
				if (!ImGui::IsAnyMouseDown() && GetCapture() == NULL)
					SetCapture(hWnd);
				io.MouseDown[button] = true;
				break;
			}
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
			case WM_XBUTTONUP:
			{
				int button = 0;
				if (msg == WM_LBUTTONUP) { button = 0; }
				if (msg == WM_RBUTTONUP) { button = 1; }
				if (msg == WM_MBUTTONUP) { button = 2; }
				if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1) ? 3 : 4; }
				io.MouseDown[button] = false;
				if (!ImGui::IsAnyMouseDown() && GetCapture() == hWnd)
					ReleaseCapture();
				break;
			}
			case WM_MOUSEWHEEL:
				io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
				break;
			case WM_MOUSEHWHEEL:
				io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
				break;
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				if (wParam < 256)
					io.KeysDown[wParam] = 1;
				break;
			case WM_KEYUP:
			case WM_SYSKEYUP:
				if (wParam < 256)
					io.KeysDown[wParam] = 0;
				break;
			case WM_CHAR:
				if (wParam > 0 && wParam < 0x10000)
					io.AddInputCharacterUTF16((unsigned short)wParam);
				break;
			}

		}
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
		if (msg == WM_KEYUP) return CallWindowProc(oWindowProc, hWnd, msg, wParam, lParam);
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return CallWindowProc(oWindowProc, hWnd, msg, wParam, lParam);
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
		tslog::log("d3d11.dll Address: %p", (void*)dxgi);
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
	tslog::log("GWorld Address: %p", Addr);
	return Addr;
}
uintptr_t findGObjects()
{
	if (dxgi == NULL) return 0;
	char Sign[] = "\x48\x8B\x0D\x00\x00\x00\x00\x81\x4C\xD1\x00\x00\x00\x00\x00\x48\x8D\x4D\xD8";
	char Mask[] = "xxx????xxx?????xxxx";
	uintptr_t Addr = ts::Aobs(Sign, Mask, (uintptr_t)g_BaseModule.lpBaseOfDll, (uintptr_t)g_BaseModule.lpBaseOfDll + (uintptr_t)g_BaseModule.SizeOfImage);
	if (!Addr) return 0;
	auto offset = *reinterpret_cast<uint32_t*>(Addr + 3);
	Addr = Addr + 7 + offset;
	setConsoleColor(14);
	tslog::log("GObjects Address: %p", Addr);
	return Addr;
}
uintptr_t findGNames()
{
	if (dxgi == NULL) return 0;
	char Sign[] = "\x48\x89\x3D\x00\x00\x00\x00\x41\x8B\x75\x00";
	char Mask[] = "xxx????xxx?";
	uintptr_t Addr = ts::Aobs(Sign, Mask, (uintptr_t)g_BaseModule.lpBaseOfDll, (uintptr_t)g_BaseModule.lpBaseOfDll + (uintptr_t)g_BaseModule.SizeOfImage);
	if (!Addr) return 0;
	auto offset = *reinterpret_cast<uint32_t*>(Addr + 3);
	Addr = Addr + 7 + offset;
	setConsoleColor(14);
	tslog::log("GNames Address: %p", Addr);
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