#include "main.hpp"

static MODULEINFO g_BaseModule;
static Present oPresent = nullptr;
static HWND window = nullptr;
static WNDPROC oWndProc = nullptr;
static ID3D11Device* pDevice = nullptr;
static ID3D11DeviceContext* pContext = nullptr;
static ID3D11RenderTargetView* mainRenderTargetView = nullptr;

static DWORD mSize = 0;
static uintptr_t dxgi = NULL;

bool Client = false;
bool ESP = false;
bool Islands = false;
bool AimBot = false;
bool Global = false;
bool Save = false;

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

int __stdcall DllMain(HMODULE hModule, DWORD dwReason, void*)
{
	Globals::DllInstance = hModule;

	if (dwReason == DLL_PROCESS_ATTACH)
		CloseHandle(CreateThread(0, 0, reinterpret_cast<PTHREAD_START_ROUTINE>(MainThread), 0, 0, 0));

	return TRUE;
}

int MainThread()
{
#if CONSOLE_LOGGING
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
#endif
	printWelcomeMessage();
	setConsoleColor(13);
	Log("Injected at: %s", __TIME__);
	setConsoleColor(7);
	setConsoleColor(10);
	Log("Module Device: 0x%p", reinterpret_cast<void*>(Globals::DllInstance));
	setConsoleColor(7);
	tslog::init(LOG_LEVEL_TARGET, SHOULD_ALLOC_CONSOLE);
	
	

	if (!getBaseModule())
	{
		setConsoleColor(10);
		Log("Module Base Address not found.");
		setConsoleColor(7);
		return false;
	}
	else
	{
		setConsoleColor(10);
		Log("Module Base Address: %p", (void*)g_BaseModule.lpBaseOfDll);
		setConsoleColor(7);
	}
	setConsoleColor(14);
	Log("Starting main initialization..");
	
	getModule();

	if (dxgi == NULL)
	{
		setConsoleColor(10);
		Log("dxgi.dll not found.");

		return false;
	}

	uintptr_t world = findGWorld();
	if (!world)
	{
		setConsoleColor(10);
		Log("GWorld not found.");

		return false;
	}
	uintptr_t objects = findGObjects();
	if (!objects)
	{
		setConsoleColor(10);
		Log("GObjects not found.");

		return false;
	}
	uintptr_t names = findGNames();
	if (!names)
	{
		setConsoleColor(10);
		Log("GNames not found.");

		return false;
	}
	if (!initUE4(world, objects, names))
	{
		setConsoleColor(10);
		Log("UE4 vars can not be initialized.");

		return false;
	}
	static bool Done = false;
	
	
	while(!Done)
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)&oPresent, hkPresent);
			Done = true;
			setConsoleColor(10);
			Log("Initialization and hooking successful!");
			setConsoleColor(9);
			Log("Menu Keys: [Ins] -> Open Menu - [End] -> Unload Menu\n");
		}
		else
		{
			setConsoleColor(10);
			Log("Initialization and hooking failed!");
		}
	}

	while (true)
	{
		if ((GetAsyncKeyState(VK_END) & 1) || Globals::IsClosing)
			break;

		Sleep(1000);
	}
	setConsoleColor(13);
	Log("Unhooked at: %s", __TIME__);

	Globals::IsClosing = false;
	Globals::Open = false;

	kiero::unbind(8);
	EngineShutdown();
	clearDX11Objects();
	kiero::shutdown();

	FreeLibrary((HMODULE)Globals::DllInstance);
	return TRUE;
}

LRESULT __stdcall WndProc( const HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam ) {

	if ( uMsg == WM_KEYUP && wParam == VK_INSERT )
		Globals::Open ^= 1;

	if ( Globals::Open )
	{
		ImGui_ImplWin32_WndProcHandler( hWnd , uMsg , wParam , lParam );
		return true;
	}	

	return CallWindowProc( oWndProc , hWnd , uMsg , wParam , lParam );
}

HRESULT __stdcall hkPresent( IDXGISwapChain* pSwapChain , UINT SyncInterval , UINT Flags )
{
	static bool Init = false;
	if ( !Init )
	{
		if ( SUCCEEDED( pSwapChain->GetDevice( __uuidof( ID3D11Device ) , (void**)&pDevice ) ) )
		{
			pDevice->GetImmediateContext( &pContext );
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc( &sd );
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer( 0 , __uuidof( ID3D11Texture2D ) , (LPVOID*)&pBackBuffer );
			pDevice->CreateRenderTargetView( pBackBuffer , NULL , &mainRenderTargetView );
			pBackBuffer->Release( );
			oWndProc = (WNDPROC)SetWindowLongPtr( window , GWLP_WNDPROC , (LONG_PTR)WndProc );
			InitImGui( );
			Init = true;

		}

		else
			return oPresent( pSwapChain , SyncInterval , Flags );
	}

	ImGui_ImplDX11_NewFrame( );
	ImGui_ImplWin32_NewFrame( );
	ImGui::NewFrame( );
	overlay();
	ImGui::GetIO( ).MouseDrawCursor = Globals::Open;
	OnDraw();
	
	ImGui::Render( );

	pContext->OMSetRenderTargets( 1 , &mainRenderTargetView , NULL );
	ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );
	return oPresent( pSwapChain , SyncInterval , Flags );
}

void InitImGui()
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();


	auto io = ImGui::GetIO();

	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;
	static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
	ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 17);
	ImFontConfig config;
	config.MergeMode = true;
	io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 19.0f, &config, icons_ranges);
	io.Fonts->Build();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

void overlay() {

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

}

void OnDraw()
{

	if (Globals::Open)
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
			ImVec2 windowSize(170, 291);
			ImGui::SetWindowSize(windowSize);
			ImGui::SetWindowPos(ImVec2(0, 0));

			std::vector<std::string> menuItems = { ICON_FA_USER " Client", ICON_FA_PLUS_CIRCLE " ESP", ICON_FA_EYE " Islands ESP", ICON_FA_CROSSHAIRS " AimBot", ICON_FA_GLOBE " Global", ICON_FA_SAVE " Save" };

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
						Save = false;
					}
					if (item.find("ESP") != std::string::npos)
					{
						Client = false;
						ESP = true;
						Islands = false;
						AimBot = false;
						Global = false;
						Save = false;
					}
					if (item.find("Islands ESP") != std::string::npos)
					{
						Client = false;
						ESP = false;
						Islands = true;
						AimBot = false;
						Global = false;
						Save = false;
					}
					if (item.find("AimBot") != std::string::npos)
					{
						Client = false;
						ESP = false;
						Islands = false;
						AimBot = true;
						Global = false;
						Save = false;
					}
					if (item.find("Global") != std::string::npos)
					{
						Client = false;
						ESP = false;
						Islands = false;
						AimBot = false;
						Global = true;
						Save = false;
					}
					if (item.find("Save") != std::string::npos)
					{
						Client = false;
						ESP = false;
						Islands = false;
						AimBot = false;
						Global = false;
						Save = true;
					}
				}

			}

			
		}
		ImGui::End();

		if (Client)
		{

			ImGui::SetNextWindowSize(ImVec2(500, 500));
			ImGui::Begin("Sea Of Gods - Daived - V2", &Client, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

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
				ImGui::SliderFloat("FOV Value", &Config::cfg.client.fov, 60.f, 150.f, "%.0f");
				ImGui::Checkbox("Spyglass FOV (Right Click)", &Config::cfg.client.spyRClickMode);
				ImGui::SliderFloat("Spyglass", &Config::cfg.client.spyglassFovMul, 1.f, 25.f, "%.0f");
				ImGui::SliderFloat("Eye of Reach", &Config::cfg.client.sniperFovMul, 1.f, 25.f, "%.0f");
				ImGui::Checkbox("Oxygen Level", &Config::cfg.client.oxygen);
				ImGui::Checkbox("CrossHair", &Config::cfg.client.crosshair);
				ImGui::SliderFloat("CrossHair Size", &Config::cfg.client.crosshairSize, 1.f, 50.f, "%.0f");
				ImGui::SliderFloat("CrossHair Thickness", &Config::cfg.client.crosshairThickness, 1.f, 50.f, "%.0f");
				ImGui::ColorEdit3("CrossHair Color", &Config::cfg.client.crosshairColor.x, 0);
				ImGui::SliderInt("Crosshair Type", &crosshairIndex, 0, IM_ARRAYSIZE(crosshair) - 1, crosshair[crosshairIndex]);
				Config::cfg.client.crosshairType = static_cast<decltype(Config::cfg.client.crosshairType)>(crosshairIndex);
				ImGui::Checkbox("Custom Time of Day", &Config::cfg.client.bCustomTOD);
				ImGui::SliderFloat("Time of Day", &Config::cfg.client.customTOD, 1.f, 24.f, "%.0f:00");

				
			}
			ImGui::EndChild();

			ImGui::End();
		}
		

		if (ESP)
		{
			ImGui::SetNextWindowSize(ImVec2(1350, 500));
			ImGui::Begin("Sea Of Gods - Daived - V2", &ESP, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
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
				ImGui::Checkbox("Enable", &Config::cfg.esp.players.enable);
				ImGui::SliderFloat("Players Distance", &Config::cfg.esp.players.renderDistance, 1.f, 2000.f, "%.0f");
				ImGui::ColorEdit3("Visible Color", &Config::cfg.esp.players.colorVisible.x, 0);
				ImGui::ColorEdit3("Invisible Color", &Config::cfg.esp.players.colorInvisible.x, 0);
				ImGui::Checkbox("Teammate", &Config::cfg.esp.players.team);
				ImGui::Checkbox("Tracer Line", &Config::cfg.esp.players.tracers);
				ImGui::SliderFloat("Thickness Line", &Config::cfg.esp.players.tracersThickness, 1.f, 10.f, "%.0f");

				
			}
			ImGui::EndChild();

			ImGui::NextColumn();

			ImGui::Text("Skeletons ESP");
			if (ImGui::BeginChild("Skeletons ESP", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.esp.skeletons.enable);
				ImGui::SliderFloat("Skeletons Distance", &Config::cfg.esp.skeletons.renderDistance, 1.f, 500.f, "%.0f");
				ImGui::ColorEdit3("Skeletons Color", &Config::cfg.esp.skeletons.color.x, 0);

				
			}
			ImGui::EndChild();


			ImGui::NextColumn();

			ImGui::Text("Ships ESP");
			if (ImGui::BeginChild("Ships ESP", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.esp.ships.enable);
				ImGui::SliderFloat("Ships Distance", &Config::cfg.esp.ships.renderDistance, 1.f, 5000.f, "%.0f");
				ImGui::ColorEdit3("Ships Color", &Config::cfg.esp.ships.color.x, 0);
				ImGui::Checkbox("Show Holes", &Config::cfg.esp.ships.holes);
				ImGui::Checkbox("Skeletons", &Config::cfg.esp.ships.skeletons);
				ImGui::Checkbox("Ghost Ships", &Config::cfg.esp.ships.ghosts);
				ImGui::Checkbox("Ladders Position", &Config::cfg.esp.ships.showLadders);
				ImGui::Checkbox("Ship Trajectory", &Config::cfg.esp.ships.shipTray);
				ImGui::SliderFloat("Thickness", &Config::cfg.esp.ships.shipTrayThickness, 0.f, 1000.f, "%.0f");
				ImGui::SliderFloat("Height", &Config::cfg.esp.ships.shipTrayHeight, -10.f, 20.f, "%.0f");
				ImGui::ColorEdit3("Trajectory Color", &Config::cfg.esp.ships.shipTrayCol.x, 0);

				
			}
			ImGui::EndChild();

			ImGui::End();
		}
		

		if (Islands)
		{
			ImGui::SetNextWindowSize(ImVec2(1480, 820));
			ImGui::Begin("Sea Of Gods - Daived - V2", &Islands, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
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
				ImGui::SliderFloat("Size", &Config::cfg.esp.islands.size, 1.f, 10.f, "%.0f");
				ImGui::SliderFloat("Islands Distance", &Config::cfg.esp.islands.renderDistance, 1.f, 10000.f, "%.0f");
				ImGui::ColorEdit3("Islands Names Color", &Config::cfg.esp.islands.color.x, 0);
				ImGui::Checkbox("Map Marks", &Config::cfg.esp.islands.marks);
				ImGui::SliderFloat("Marks Distance", &Config::cfg.esp.islands.marksRenderDistance, 1.f, 10000.f, "%.0f");
				ImGui::ColorEdit3("Marks Color", &Config::cfg.esp.islands.marksColor.x, 0);
				ImGui::Checkbox("Rare Spots", &Config::cfg.esp.islands.rareNames);
				ImGui::Text("Filter Keyword");
				ImGui::InputText("Keyword", Config::cfg.esp.islands.rareNamesFilter, IM_ARRAYSIZE(Config::cfg.esp.islands.rareNamesFilter));
				ImGui::SliderFloat("Rare Spots Distance", &Config::cfg.esp.islands.decalsRenderDistance, 1.f, 1000.f, "%.0f");
				ImGui::ColorEdit3("Rare Spots Color", &Config::cfg.esp.islands.decalsColor.x, 0);
				ImGui::Checkbox("Vaults", &Config::cfg.esp.islands.vaults);
				ImGui::SliderFloat("Vaults Distance", &Config::cfg.esp.islands.vaultsRenderDistance, 1.f, 10000.f, "%.0f");
				ImGui::ColorEdit3("Vaults Color", &Config::cfg.esp.islands.vaultsColor.x, 0);
				ImGui::Checkbox("Barrels", &Config::cfg.esp.islands.barrels);
				ImGui::Checkbox("Barrels Items", &Config::cfg.esp.islands.barrelspeek);
				ImGui::Checkbox("(R) Key Toggle", &Config::cfg.esp.islands.barrelstoggle);
				ImGui::SliderFloat("Barrels Distance", &Config::cfg.esp.islands.barrelsRenderDistance, 1.f, 10000.f, "%.0f");
				ImGui::ColorEdit3("Barrels Color", &Config::cfg.esp.islands.barrelsColor.x, 0);
				ImGui::Checkbox("Ammo Chests", &Config::cfg.esp.islands.ammoChest);
				ImGui::SliderFloat("Ammo Distance", &Config::cfg.esp.islands.ammoChestRenderDistance, 1.f, 10000.f, "%.0f");
				ImGui::ColorEdit3("Ammo Color", &Config::cfg.esp.islands.ammoChestColor.x, 0);

				
			}
			ImGui::EndChild();

			ImGui::NextColumn();

			ImGui::Text("Items");
			if (ImGui::BeginChild("Items ESP", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.esp.items.enable);
				ImGui::SliderFloat("Items Distance", &Config::cfg.esp.items.renderDistance, 1.f, 500.f, "%.0f");
				ImGui::ColorEdit3("Items Color", &Config::cfg.esp.items.color.x, 0);
				ImGui::Checkbox("(R) key Toggle Names", &Config::cfg.esp.items.nameToggle);
				ImGui::Checkbox("Animals", &Config::cfg.esp.items.animals);
				ImGui::SliderFloat("Animals Distance", &Config::cfg.esp.items.animalsRenderDistance, 1.f, 500.f, "%.0f");
				ImGui::ColorEdit3("Animals Color", &Config::cfg.esp.items.animalsColor.x, 0);
				ImGui::Checkbox("Sharks", &Config::cfg.esp.others.sharks);
				ImGui::SliderFloat("Sharks Distance", &Config::cfg.esp.others.sharksRenderDistance, 1.f, 500.f, "%.0f");
				ImGui::ColorEdit3("Sharks Color", &Config::cfg.esp.others.sharksColor.x, 0);
				ImGui::Checkbox("Lost Cargo", &Config::cfg.esp.items.lostCargo);
				ImGui::ColorEdit3("Cargo Color", &Config::cfg.esp.items.cluesColor.x, 0);
				ImGui::Checkbox("GhostShips Rewards", &Config::cfg.esp.items.gsRewards);
				ImGui::ColorEdit3("Rewards Color", &Config::cfg.esp.items.gsRewardsColor.x, 0);

				
			}
			ImGui::EndChild();

			ImGui::NextColumn();

			ImGui::Text("World");
			if (ImGui::BeginChild("Others ESP", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.esp.others.enable);
				ImGui::Checkbox("Shipwrecks", &Config::cfg.esp.others.shipwrecks);
				ImGui::SliderFloat("Shipwrecks Distance", &Config::cfg.esp.others.shipwrecksRenderDistance, 1.f, 5000.f, "%.0f");
				ImGui::ColorEdit3("Shipwrecks Color", &Config::cfg.esp.others.shipwrecksColor.x, 0);
				ImGui::Checkbox("Mermaids", &Config::cfg.esp.others.mermaids);
				ImGui::SliderFloat("Mermaids Distance", &Config::cfg.esp.others.mermaidsRenderDistance, 1.f, 1000.f, "%.0f");
				ImGui::ColorEdit3("Mermaids Color", &Config::cfg.esp.others.mermaidsColor.x, 0);
				ImGui::Checkbox("Rowboats", &Config::cfg.esp.others.rowboats);
				ImGui::SliderFloat("Rowboats Distance", &Config::cfg.esp.others.rowboatsRenderDistance, 1.f, 3500.f, "%.0f");
				ImGui::ColorEdit3("Rowboats Color", &Config::cfg.esp.others.rowboatsColor.x, 0);
				ImGui::Checkbox("World Events", &Config::cfg.esp.others.events);
				ImGui::SliderFloat("Events Distance", &Config::cfg.esp.others.eventsRenderDistance, 1.f, 10000.f, "%.0f");
				ImGui::ColorEdit3("Events Color", &Config::cfg.esp.others.eventsColor.x, 0);

				
			}
			ImGui::EndChild();

			ImGui::End();
		}
		

		if (AimBot)
		{
			ImGui::SetNextWindowSize(ImVec2(450, 550));
			ImGui::Begin("Sea Of Gods - Daived - V2", &AimBot, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
			ImGui::Text("AimBot");
			if (ImGui::BeginChild("AimBot Activation", ImVec2(200.f, 50.f), false, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.aim.enable);

				
			}
			ImGui::EndChild();

			ImGui::Columns(2, "AimBot", false);

			ImGui::Text("Weapons AimBot");
			if (ImGui::BeginChild("Weapons AimBot", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.aim.weapon.enable);
				ImGui::SliderFloat("Pitch", &Config::cfg.aim.weapon.fPitch, 1.f, 100.f, "%.0f");
				ImGui::SliderFloat("Yaw", &Config::cfg.aim.weapon.fYaw, 1.f, 100.f, "%.0f");
				ImGui::SliderFloat("Smoothness", &Config::cfg.aim.weapon.smooth, 1.f, 10.f, "%.0f");
				ImGui::SliderFloat("Height", &Config::cfg.aim.weapon.height, 1.f, 100.f, "%.0f");
				ImGui::Checkbox("Players", &Config::cfg.aim.weapon.players);
				ImGui::Checkbox("Skeletons", &Config::cfg.aim.weapon.skeletons);
				ImGui::Checkbox("Gunpowder", &Config::cfg.aim.weapon.kegs);
				ImGui::Checkbox("Instant Shot", &Config::cfg.aim.weapon.trigger);
				ImGui::Checkbox("Visible Only", &Config::cfg.aim.weapon.visibleOnly);

				
			}
			ImGui::EndChild();

			ImGui::NextColumn();

			ImGui::Text("Cannon AimBot");
			if (ImGui::BeginChild("Cannon AimBot", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.aim.cannon.enable);
				ImGui::SliderFloat("Pitch", &Config::cfg.aim.cannon.fPitch, 1.f, 100.f, "%.0f");
				ImGui::SliderFloat("Yaw", &Config::cfg.aim.cannon.fYaw, 1.f, 100.f, "%.0f");
				ImGui::Checkbox("Instant Shot", &Config::cfg.aim.cannon.instant);
				ImGui::Checkbox("Chain Shots (F1)", &Config::cfg.aim.cannon.chains);
				ImGui::Checkbox("Players", &Config::cfg.aim.cannon.players);
				ImGui::Checkbox("Skeletons", &Config::cfg.aim.cannon.skeletons);
				ImGui::Checkbox("Ghost Ships", &Config::cfg.aim.cannon.ghostShips);
				ImGui::Checkbox("miss aim (F3)", &Config::cfg.aim.cannon.lowAim);
				ImGui::Checkbox("Player Deck (F2)", &Config::cfg.aim.cannon.deckshots);
				ImGui::Checkbox("Visible Only", &Config::cfg.aim.cannon.visibleOnly);
				ImGui::Checkbox("AimBot V2", &Config::cfg.aim.cannon.improvedVersion);

				
			}
			ImGui::EndChild();

			ImGui::End();
		}
		if (Global)
		{
			ImGui::SetNextWindowSize(ImVec2(550, 400));
			ImGui::Begin("Sea Of Gods - Daived - V2", &Global, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
			ImGui::Text("Global");
			if (ImGui::BeginChild("Global Activation", ImVec2(200.f, 50.f), false, 0))
			{
				ImGui::Checkbox("Enable", &Config::cfg.game.enable);

				
			}
			ImGui::EndChild();

			if (ImGui::BeginChild("Global", ImVec2(0.f, 0.f), true, 0))
			{
				ImGui::Checkbox("Map Pins", &Config::cfg.game.mapPins);
				ImGui::Checkbox("Players List", &Config::cfg.game.playerList);
				ImGui::Checkbox("Cook Tracker", &Config::cfg.game.cooking);
				ImGui::Checkbox("Anti AFK", &Config::cfg.game.noIdleKick);
				ImGui::Checkbox("Walk Underwate (C) key ", &Config::cfg.game.walkUnderwater);
				ImGui::Checkbox("Sunk Location", &Config::cfg.game.showSunk);
				ImGui::ColorEdit3("Sunk Location Color", &Config::cfg.game.sunkColor.x, 0);
				if (ImGui::Button("Clear Sunk List"))
				{
					ClearSunkList();
				}

				
			}
			ImGui::EndChild();

			ImGui::End();
		}
		if (Save)
		{
			ImGui::SetNextWindowSize(ImVec2(270, 95));
			ImGui::Begin("Sea Of Gods - Daived - V2", &Save, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
			if (ImGui::Button("Save Settings", ImVec2(120, 50)))
			{
				do {
					wchar_t buf[MAX_PATH];
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
}

void clearDX11Objects()
{
	safe_release(mainRenderTargetView);
	safe_release(pContext);
	safe_release(pDevice);
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();
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
		setConsoleColor(14);
		Log("d3d11.dll Address: %p", (void*)dxgi);
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
	Log("GWorld Address: %p", Addr);
	return Addr;
}
uintptr_t findGObjects()
{
	if (dxgi == NULL) return 0;
	char Sign[] = "\x89\x0D\x00\x00\x00\x00\x48\x8B\xDF\x48\x89\x5C\x24";
	char Mask[] = "xx????xxxxxxx";
	uintptr_t Addr = ts::Aobs(Sign, Mask, (uintptr_t)g_BaseModule.lpBaseOfDll, (uintptr_t)g_BaseModule.lpBaseOfDll + (uintptr_t)g_BaseModule.SizeOfImage);
	if (!Addr) return 0;
	auto offset = *reinterpret_cast<uint32_t*>(Addr + 2);
	Addr = Addr + 6 + offset + 16;
	setConsoleColor(14);
	Log("GObjects Address: %p", Addr);
	return Addr;
}
uintptr_t findGNames()
{
	if (dxgi == NULL) return 0;
	char Sign[] = "\x48\x8B\x1D\x00\x00\x00\x00\x48\x85\xDB\x75\x00\xB9\x08\x04\x00\x00";
	char Mask[] = "xxx????xxxx?xxxxx";
	uintptr_t Addr = ts::Aobs(Sign, Mask, (uintptr_t)g_BaseModule.lpBaseOfDll, (uintptr_t)g_BaseModule.lpBaseOfDll + (uintptr_t)g_BaseModule.SizeOfImage);
	if (!Addr) return 0;
	auto offset = *reinterpret_cast<uint32_t*>(Addr + 3);
	Addr = Addr + 7 + offset;
	setConsoleColor(14);
	Log("GNames Address: %p", Addr);
	return Addr;
}