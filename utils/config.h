#pragma once

#include "../library/ImGui/imgui.h"

class Config
{
public:
	static inline struct Configuration {
		enum class ECrosshairs : int {
			ENone,
			ECircle,
			ECross
		};
		enum class EBox : int {
			ENone,
			E2DBoxes,
			E3DBoxes,
			EDebugBoxes
		};
		enum class EBar : int {
			ENone,
			ELeft,
			ERight,
			EBottom,
			ETop,
			ETriangle
		};
		struct
		{
			bool enable = false;
			bool fovEnable = false;
			float fov = 90.f;
			float spyglassFovMul = 1.0;
			float sniperFovMul = 1.0;
			bool spyRClickMode = false;
			bool oxygen = false;
			bool crosshair = false;
			float crosshairSize = 3.f;
			float crosshairThickness = 2.f;
			ImVec4 crosshairColor = { 1.f, 1.f, 1.f, 1.f };
			ECrosshairs crosshairType = ECrosshairs::ECircle;
			bool bCustomTOD = false;
			float customTOD = 1.f;
			bool bCompass = false;
		}client;
		struct
		{
			bool enable = false;
			struct
			{
				bool enable = false;
				float renderDistance = 100.f;
				ImVec4 colorVisible = { 1.f, 1.f, 1.f, 1.f };
				ImVec4 colorInvisible = { 1.f, 1.f, 1.f, 1.f };
				bool team = false;
				bool tracers = false;
				float tracersThickness = 1.f;
				EBox boxType = EBox::ENone;
				EBar barType = EBar::ENone;
				bool bName = false;
				bool bSkeleton = false;

			}players;
			struct
			{
				bool enable = false;
				float renderDistance = 100.f;
				ImVec4 color = { 1.f, 1.f, 1.f, 1.f };
				bool bSkeleton = false;
				ImVec4 colorVis = { 1.f, 1.f, 1.f, 1.f };
				ImVec4 colorInv = { 1.f, 1.f, 1.f, 1.f };
				EBox boxType = EBox::ENone;
				EBar barType = EBar::ENone;
			}skeletons;
			struct
			{
				bool enable = false;
				float renderDistance = 100.f;
				ImVec4 color = { 1.f, 1.f, 1.f, 1.f };
				bool holes = false;
				ImVec4 holesColor = { 1.f, 1.f, 1.f, 1.f };
				bool skeletons = false;
				bool ghosts = false;
				bool shipTray = false;
				float shipTrayThickness = 0.f;
				float shipTrayHeight = 0.f;
				ImVec4 shipTrayCol = { 1.f, 1.f, 1.f, 1.f };
				bool showLadders = false;
			}ships;
			struct
			{
				bool enable = false;
				float renderDistance = 1000.f;
				float size = 10.f;
				ImVec4 color = { 1.f, 1.f, 1.f, 1.f };
				bool marks = false;
				float marksRenderDistance = 100.f;
				ImVec4 marksColor = { 1.f, 1.f, 1.f, 1.f };
				bool vaults = false;
				float vaultsRenderDistance = 100.f;
				ImVec4 vaultsColor = { 1.f, 1.f, 1.f, 1.f };
				bool barrels = false;
				ImVec4 barrelsColor = { 1.f, 1.f, 1.f, 1.f };
				bool barrelspeek = false;
				bool barrelstoggle = false;
				bool ammoChest = false;
				float ammoChestRenderDistance = 100.f;
				ImVec4 ammoChestColor = { 1.f, 1.f, 1.f, 1.f };
			}islands;
			struct
			{
				bool enable = false;
				float renderDistance = 100.f;
				ImVec4 color = { 1.f, 1.f, 1.f, 1.f };
				bool nameToggle = false;
				bool mermaids = false;
				bool animals = false;
				float animalsRenderDistance = 100.f;
				ImVec4 animalsColor = { 1.f, 1.f, 1.f, 1.f };
				bool bName = false;
				EBox boxType = EBox::ENone;
				bool sharks = false;
				float sharksRenderDistance = 100.f;
				ImVec4 sharksColor = { 1.f, 1.f, 1.f, 1.f };
				bool bSkeleton = false;
				bool lostCargo = false;
				ImVec4 cluesColor = { 1.f, 1.f, 1.f, 1.f };
			}items;
			struct
			{
				bool enable = false;
				bool mermaids = false;
				float mermaidsRenderDistance = 100.f;
				ImVec4 mermaidsColor = { 1.f, 1.f, 1.f, 1.f };
				bool shipwrecks = false;
				float shipwrecksRenderDistance = 100.f;
				ImVec4 shipwrecksColor = { 1.f, 1.f, 1.f, 1.f };
				bool rowboats = false;
				float rowboatsRenderDistance = 100.f;
				ImVec4 rowboatsColor = { 1.f, 1.f, 1.f, 1.f };
				bool events = false;
				float eventsRenderDistance = 100.f;
				ImVec4 eventsColor = { 1.f, 1.f, 1.f, 1.f };
			}others;
		}esp;
		struct
		{
			bool enable = false;
			struct
			{
				bool enable = false;
				float fPitch = 1.f;
				float fYaw = 1.f;
				float smooth = 1.f;
				float height = 1.f;
				bool players = false;
				bool skeletons = false;
				bool kegs = false;
				bool trigger = false;
				bool visibleOnly = false;
				bool calcShipVel = true;
				bool fastweapon = false;
			}weapon;
			struct
			{
				bool enable = false;
				float fPitch = 1.f;
				float fYaw = 1.f;
				float smooth = 1.f;
				bool instant = false;
				bool chains = false;
				bool players = false;
				bool skeletons = false;
				bool deckshots = false;
				bool ghostShips = false;
				bool lowAim = false;
				bool visibleOnly = false;
				bool autoDetect = false;
				bool drawPred = false;
				bool improvedVersion = false;
			}cannon;
			struct {
				bool enable = false;
				bool visibleOnly = false;
				bool bAutoshoot = false;
				float fYaw = 100.f;
				float fPitch = 102.f;
			} harpoon;
			struct
			{
				bool rage = false;
			}others;
		}aim;
		struct
		{
			bool enable = false;
			bool mapPins = false;
			bool walkUnderwater = false;
			bool showSunk = false;
			ImVec4 sunkColor = { 1.f, 1.f, 1.f, 1.f };
			bool playerList = false;
			bool cooking = false;
			bool noIdleKick = false;
			bool shipInfo = false;
			bool b_bunnyhop = false;
		}game;
		struct
		{
			bool process = false;
			int cFont = 0;
			int lFont = 0;
			float renderTextSizeFactor = 1.0f;
			float nameTextRenderDistanceMax = 5000.f;
			float nameTextRenderDistanceMin = 45.f;
			float pinRenderDistanceMax = 50.f;
			float pinRenderDistanceMin = 0.f;

			bool printErrorCodes = false;
			bool interceptProcessEvent = false;
			bool printRPCCalls = false;
			bool debugNames = false;
			int debugNamesTextSize = 20;
			char debugNamesFilter[0x64] = { 0 };
			float debugNamesRenderDistance = 0.f;
			bool bDummy = false;
		}dev;
	}cfg;
};