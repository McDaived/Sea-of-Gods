#pragma once

#include <vector>
#include <Windows.h>
#include "../sdk/SDK.h"
#include "logger.h"
#include "config.h"
#include "cIcons.h"
#include <intrin.h>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")
#include "../library/ImGui/imgui_impl_win32.h"
#include "../library/ImGui/imgui_impl_dx11.h"
#include "../library/ImGui/imgui_internal.h"
#include "../library/ImGui/imgui.h"
#include "../library/minhook/include/MinHook.h"

void processhk(void* Object, UFunction* Function, void* Params);
typedef void(__stdcall* fnProcessEvent)(void* Object, UFunction* Function, void* Params);

namespace engine
{
	static UAthenaGameViewportClient* AthenaGameViewportClient = nullptr;
	static ULocalPlayer* localPlayer = nullptr;
	static APlayerController* playerController = nullptr;
	static AAthenaPlayerCharacter* localPlayerActor = nullptr;
	static ULevel* persistentLevel = nullptr;
	static Config::Configuration* cfg = &Config::cfg;
	static bool bClearSunkList = false;
	static fnProcessEvent oProcessEvent = nullptr;
	static ACharacter* aimTarget = nullptr;
	static bool bInPirateGenerator = false;
	static PirateGeneratorLineUpUI* carousel = nullptr;
}

struct FunctionIndex {
	static inline int32_t BoxedRpcDispatcherComponent = -1;
};

void render(ImDrawList* drawList);
bool initUE4(uintptr_t world, uintptr_t objects, uintptr_t names);
bool setGameVars();
bool checkGameVars();
bool updateGameVars();
void RenderText(ImDrawList* drawList, const char* text, const FVector2D& pos, const ImVec4& color, const float dist, const bool outlined = false, const bool centered = true);
void RenderText(ImDrawList* drawList, const char* text, const FVector2D& pos, const ImVec4& color, const bool outlined = false, const bool centered = true);
void RenderText(ImDrawList* drawList, const char* text, const FVector2D& pos, const ImVec4& color, const int fontSize = 10, const bool centered = true);
void RenderText(ImDrawList* drawList, const char* text, const ImVec2& screen, const ImVec4& color, const float size, const bool outlined = false, const bool centered = true);
void renderPin(ImDrawList* drawList, const ImVec2& pos, const ImVec4& color, const float radius);
void Render2DBox(ImDrawList* drawList, const FVector2D& top, const FVector2D& bottom, const float height, const float width, const ImVec4& color);
bool Render3DBox(AController* const controller, const FVector& origin, const FVector& extent, const FRotator& rotation, const ImVec4& color);
bool RenderSkeleton(ImDrawList* drawList, AController* const controller, USkeletalMeshComponent* const mesh, const FMatrix& comp2world, const std::pair<const BYTE*, const BYTE>* skeleton, int size, const ImVec4& color);
float fClamp(float v, const float min, const float max);
bool checkSDKObjects();
bool WorldToScreen(Vector3 origin, Vector2* out, const FVector& cameraLocation, const FRotator& cameraRotation, const float fov);
uintptr_t milliseconds_now();
Vector2 RotatePoint(Vector2 pointToRotate, Vector2 centerPoint, float angle, bool angleInRadians = false);
bool raytrace(UWorld* world, const struct FVector& start, const struct FVector& end, struct FHitResult* hit);
FVector pickHoleToAim(AHullDamage* damage, const FVector& localLoc);
void ClearSunkList();
void hookProcessEvent();
void unhookProcessEvent();
void EngineShutdown();
bool loadPirateGenerator();
uintptr_t getPirateGenerator();

int getMapNameCode(char* name);
std::string getIslandNameByCode(int code);
std::string getShortName(std::string name);