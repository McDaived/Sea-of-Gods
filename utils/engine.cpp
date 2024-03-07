#include "engine.h"

bool GetProjectilePath(std::vector<FVector>& v, FVector& Vel, FVector& Pos, float Gravity, int count, UWorld* world)
{
	float interval = 0.033f;
	for (unsigned int i = 0; i < count; ++i)
	{
		v.push_back(Pos);
		FVector move;
		move.X = (Vel.X) * interval;
		move.Y = (Vel.Y) * interval;
		float newZ = Vel.Z - (Gravity * interval);
		move.Z = ((Vel.Z + newZ) * 0.5f) * interval;
		Vel.Z = newZ;
		FVector nextPos = Pos + move;
		bool res = true;

		FHitResult hit_result;
		res = raytrace(world, Pos, nextPos, &hit_result);


		Pos = nextPos;
		if (res && hit_result.Distance > 1.f) return true;
	}
	return false;
}

#include <complex>
void SolveQuartic(const std::complex<float> coefficients[5], std::complex<float> roots[4]) {
	const std::complex<float> a = coefficients[4];
	const std::complex<float> b = coefficients[3] / a;
	const std::complex<float> c = coefficients[2] / a;
	const std::complex<float> d = coefficients[1] / a;
	const std::complex<float> e = coefficients[0] / a;
	const std::complex<float> Q1 = c * c - 3.f * b * d + 12.f * e;
	const std::complex<float> Q2 = 2.f * c * c * c - 9.f * b * c * d + 27.f * d * d + 27.f * b * b * e - 72.f * c * e;
	const std::complex<float> Q3 = 8.f * b * c - 16.f * d - 2.f * b * b * b;
	const std::complex<float> Q4 = 3.f * b * b - 8.f * c;
	const std::complex<float> Q5 = std::pow(Q2 / 2.f + std::sqrt(Q2 * Q2 / 4.f - Q1 * Q1 * Q1), 1.f / 3.f);
	const std::complex<float> Q6 = (Q1 / Q5 + Q5) / 3.f;
	const std::complex<float> Q7 = 2.f * std::sqrt(Q4 / 12.f + Q6);
	roots[0] = (-b - Q7 - std::sqrt(4.f * Q4 / 6.f - 4.f * Q6 - Q3 / Q7)) / 4.f;
	roots[1] = (-b - Q7 + std::sqrt(4.f * Q4 / 6.f - 4.f * Q6 - Q3 / Q7)) / 4.f;
	roots[2] = (-b + Q7 - std::sqrt(4.f * Q4 / 6.f - 4.f * Q6 + Q3 / Q7)) / 4.f;
	roots[3] = (-b + Q7 + std::sqrt(4.f * Q4 / 6.f - 4.f * Q6 + Q3 / Q7)) / 4.f;
}

#define _USE_MATH_DEFINES
#include <math.h>

FRotator ToFRotator(FVector vec)
{
	FRotator rot;
	float RADPI = (float)(180 / M_PI);
	rot.Yaw = (float)(atan2f(vec.Y, vec.X) * RADPI);
	rot.Pitch = (float)atan2f(vec.Z, sqrt((vec.X * vec.X) + (vec.Y * vec.Y))) * RADPI;
	rot.Roll = 0;
	return rot;
}

int AimAtStaticTarget(const FVector& oTargetPos, float fProjectileSpeed, float fProjectileGravityScalar, const FVector& oSourcePos, FRotator& oOutLow, FRotator& oOutHigh) {
	const float gravity = 981.f * fProjectileGravityScalar;
	const FVector diff(oTargetPos - oSourcePos);
	const FVector oDiffXY(diff.X, diff.Y, 0.0f);
	const float fGroundDist = oDiffXY.Size();
	const float s2 = fProjectileSpeed * fProjectileSpeed;
	const float s4 = s2 * s2;
	const float y = diff.Z;
	const float x = fGroundDist;
	const float gx = gravity * x;
	float root = s4 - (gravity * ((gx * x) + (2 * y * s2)));
	if (root < 0)
		return 0;
	root = std::sqrtf(root);
	const float fLowAngle = std::atan2f((s2 - root), gx);
	const float fHighAngle = std::atan2f((s2 + root), gx);
	int nSolutions = fLowAngle != fHighAngle ? 2 : 1;
	const FVector oGroundDir(oDiffXY.unit());
	oOutLow = ToFRotator(oGroundDir * std::cosf(fLowAngle) * fProjectileSpeed + FVector(0.f, 0.f, 1.f) * std::sinf(fLowAngle) * fProjectileSpeed);
	if (nSolutions == 2)
		oOutHigh = ToFRotator(oGroundDir * std::cosf(fHighAngle) * fProjectileSpeed + FVector(0.f, 0.f, 1.f) * std::sinf(fHighAngle) * fProjectileSpeed);
	return nSolutions;
}

#include <limits>
int AimAtMovingTarget(const FVector& oTargetPos, const FVector& oTargetVelocity, float fProjectileSpeed, float fProjectileGravityScalar, const FVector& oSourcePos, const FVector& oSourceVelocity, FRotator& oOutLow, FRotator& oOutHigh) {
	const FVector v(oTargetVelocity - oSourceVelocity);
	const FVector g(0.f, 0.f, -981.f * fProjectileGravityScalar);
	const FVector p(oTargetPos - oSourcePos);
	const float c4 = g | g * 0.25f;
	const float c3 = v | g;
	const float c2 = (p | g) + (v | v) - (fProjectileSpeed * fProjectileSpeed);
	const float c1 = 2.f * (p | v);
	const float c0 = p | p;
	std::complex<float> pOutRoots[4];
	const std::complex<float> pInCoeffs[5] = { c0, c1, c2, c3, c4 };
	SolveQuartic(pInCoeffs, pOutRoots);
	float fBestRoot = FLT_MAX;
	for (int i = 0; i < 4; i++) {
		if (pOutRoots[i].real() > 0.f && std::abs(pOutRoots[i].imag()) < 0.0001f && pOutRoots[i].real() < fBestRoot) {
			fBestRoot = pOutRoots[i].real();
		}
	}
	if (fBestRoot == FLT_MAX)
		return 0;
	const FVector oAimAt = oTargetPos + (v * fBestRoot);
	return AimAtStaticTarget(oAimAt, fProjectileSpeed, fProjectileGravityScalar, oSourcePos, oOutLow, oOutHigh);
}

float time_func(float t, float K, float L, float M, float N, float r, float w, float theta, float S2)
{
	const float K2 = K * K;
	const float L2 = L * L;
	const float M2 = M * M;
	const float N2 = N * N;
	const float r2 = r * r;
	return N2 * t * t * t * t + ((2 * M * N) - S2) * t * t + 2 * r * (K * cos(theta + (w * t)) + L * sin(theta + (w * t))) + K2 + L2 + M2 + r2;
}

float time_derivFunc(float t, float K, float L, float M, float N, float r, float w, float theta, float S2)
{
	const float N2 = N * N;
	return 4 * N2 * t * t * t * t + 2 * ((2 * M * N) - S2) * t + 2 * r * w * (L * cos(theta + (w * t)) - K * sin(theta + (w + t)));
}

float newtonRaphson(float t, float K, float L, float M, float N, float r, float w, float theta, float S2)
{
	float h = time_func(t, K, L, M, N, r, w, theta, S2) / time_derivFunc(t, K, L, M, N, r, w, theta, S2);
	int counter = 0;
	while (abs(h) >= 0.01)
	{
		if (counter > 200)
		{
			break;
		}
		h = time_func(t, K, L, M, N, r, w, theta, S2) / time_derivFunc(t, K, L, M, N, r, w, theta, S2);
		t = t - h;
		counter++;
	}
	return t;
}

int AimAtShip(const FVector& oTargetPos, const FVector& oTargetVelocity, const FVector& oTargetAngularVelocity, const FVector& oSourcePos, const FVector& oSourceVelocity, float fProjectileSpeed, float fProjectileGravityScalar, FRotator& oOutLow, FRotator& oOutHigh)
{
	const FVector pPos = oSourcePos;
	const float w = oTargetAngularVelocity.Z;
	if (w > -1.0 && w < 1.0)
	{
		int n = AimAtMovingTarget(oTargetPos, oTargetVelocity, fProjectileSpeed, fProjectileGravityScalar, pPos, oSourceVelocity, oOutLow, oOutHigh);
		return n;
	}

	const FVector diff(oTargetPos - pPos);
	auto w_pos = FVector(0, 0, w).Size();
	auto w_rad = (w_pos * M_PI) / 180;


	const float v_target_boat = FVector(oTargetVelocity.X, oTargetVelocity.Y, 0.0f).Size();
	const float r = (v_target_boat / w_rad) * 0.98;

	float theta_rad;
	if (w < 0.f)
	{
		if (oTargetVelocity.X < 0.f)
		{
			theta_rad = (std::atan2f((-1 * oTargetVelocity.X), oTargetVelocity.Y));
		}
		else if (oTargetVelocity.X > 0.f)
		{
			theta_rad = (std::atan2f((-1 * oTargetVelocity.X), oTargetVelocity.Y)) + 2 * M_PI;
		}
	}
	else if (w > 0.f)
	{
		theta_rad = (std::atan2f((-1 * oTargetVelocity.X), oTargetVelocity.Y)) + M_PI;
	}

	const float K = diff.X - (r * cosf(theta_rad));
	const float L = diff.Y - (r * sinf(theta_rad));
	const float M = diff.Z;
	const float N = (981.f * fProjectileGravityScalar) / 2;
	const float S2 = fProjectileSpeed * fProjectileSpeed;

	const FVector oDiffXY(diff.X, diff.Y, 0.0f);
	const float fGroundDist = oDiffXY.Size();

	float t_init;
	if (fGroundDist < 10000)
	{
		t_init = 4;
	}
	else if (fGroundDist < 25000)
	{
		t_init = 10;
	}
	else if (fGroundDist < 40000)
	{
		t_init = 15;
	}
	else
	{
		t_init = 20;
	}

	float t_best = newtonRaphson(t_init, K, L, M, N, r, w_rad, theta_rad, S2);

	if (t_best < 0)
	{
		return 0;
	}

	const float At = oTargetPos.X - r * cos(theta_rad) + r * cos(theta_rad + (w_rad * t_best));
	const float Bt = oTargetPos.Y - r * sin(theta_rad) + r * sin(theta_rad + (w_rad * t_best));
	const FVector oAimAt = FVector(At, Bt, oTargetPos.Z);
	return AimAtStaticTarget(oAimAt, fProjectileSpeed, fProjectileGravityScalar, oSourcePos, oOutLow, oOutHigh);
}

using namespace engine;

void render(ImDrawList* drawList)
{
	static int error_code = 0;
	try
	{
		error_code = 1;
		static struct {
			ACharacter* target = nullptr;
			FVector location = FVector(0.f, 0.f, 0.f);
			FRotator delta = FRotator(0.f, 0.f, 0.f);
			float best = FLT_MAX;
			float smoothness = 1.f;
		} aimBest;

		aimBest.target = nullptr;
		aimBest.best = FLT_MAX;


		error_code = 2;
		auto& io = ImGui::GetIO();
		auto const world = *UWorld::GWorld;
		auto const gameState = world->GameState;
		const auto myLocation = localPlayerActor->K2_GetActorLocation();
		const auto camera = ((AController*)playerController)->PlayerCameraManager;
		const auto cameraLocation = camera->GetCameraLocation();
		const auto cameraRotation = camera->GetCameraRotation();
		auto item = localPlayerActor->GetWieldedItem();

		bool isWieldedWeapon = false;
		bool isWieldedSpyglass = false;
		if (item) isWieldedWeapon = item->isWeapon();
		if (item) isWieldedSpyglass = item->isSpyglass();

		const auto localSword = *reinterpret_cast<AMeleeWeapon**>(&item);
		const auto localWeapon = *reinterpret_cast<AProjectileWeapon**>(&item);
		ACharacter* attachObject = localPlayerActor->GetAttachParentActor();


		int XMarksMapCount = 1;
		static std::vector<ACharacter*> cookingPots = std::vector<ACharacter*>();
		cookingPots.clear();
		static std::vector<FVector> trackedSinkLocs = std::vector<FVector>();

		if (engine::bClearSunkList)
		{
			engine::bClearSunkList = false;
			trackedSinkLocs.clear();
		}

		bool isHarpoon = false;
		if (attachObject)
		{
			if (cfg->aim.harpoon.enable && attachObject->IsA(AHarpoonLauncher::StaticClass())) { isHarpoon = true; }
		}

		TArray<ULevel*> levels = AthenaGameViewportClient->World->Levels;

		error_code = 3;

		if (cfg->client.enable)
		{
			if (cfg->client.fovEnable)
			{
				error_code = 4;
				static std::uintptr_t desiredTimeFOV = 0;
				if (milliseconds_now() >= desiredTimeFOV)
				{
					static byte pass = 0;
					const auto spyglass = *reinterpret_cast<ASpyglass**>(&item);
					float fov = localPlayerActor->GetTargetFOV(localPlayerActor);
					playerController->FOV(cfg->client.fov);
					if (fov == 17.f)
					{
						playerController->FOV(cfg->client.fov * 0.2f);
					}
					if (isWieldedSpyglass && GetAsyncKeyState(VK_LBUTTON) && cfg->client.spyRClickMode == false && spyglass->BlurTime > 0.51f)
					{
						playerController->FOV(cfg->client.fov * (1 / cfg->client.spyglassFovMul));
					}
					else if (isWieldedSpyglass && GetAsyncKeyState(VK_RBUTTON) && cfg->client.spyRClickMode == true)
					{
						playerController->FOV(cfg->client.fov * (1 / cfg->client.spyglassFovMul));
					}
					else if (isWieldedWeapon && GetAsyncKeyState(VK_RBUTTON))
					{
						pass++;
						if (item->compareName("sniper_rifle_") && pass >= 2)
							playerController->FOV(cfg->client.fov * (1 / cfg->client.sniperFovMul));

					}
					else
					{
						pass = 0;
					}
					desiredTimeFOV = milliseconds_now() + 100;
				}
			}

			if (cfg->client.oxygen && localPlayerActor->IsInWater())
			{
				error_code = 5;
				auto drownComp = localPlayerActor->DrowningComponent;
				if (drownComp)
				{
					float level = drownComp->GetOxygenLevel();
					auto posX = io.DisplaySize.x * 0.5f;
					auto posY = io.DisplaySize.y * 0.85f;
					auto radius = io.DisplaySize.x * 0.01f;

					drawList->AddCircleFilled({ posX, posY }, radius, ImGui::GetColorU32(IM_COL32(0, 0, 0, 128)), 32);
					drawList->AddCircleFilled({ posX, posY }, radius * level, ImGui::GetColorU32(IM_COL32(0, 200, 255, 200)), 32);

					ImVec2 iconPos = { posX - 8, posY - 7 };
					ImGui::GetWindowDrawList()->AddText(iconPos, ImGui::GetColorU32(IM_COL32_WHITE), ICON_FA_HEART);

					char buf[0x64];
					float pLevel = level * 100.f;
					sprintf_s(buf, sizeof(buf), "[ %.0f%% ]", pLevel);
					auto oxColor = level > 0.25f ? ImVec4(0.f, 1.f, 0.f, 1.f) : ImVec4(1.f, 0.f, 0.f, 1.f);
					RenderText(drawList, buf, FVector2D(posX, posY + radius + 10.f), oxColor, 20);
				}


			}

			if (cfg->client.crosshair)
			{
				error_code = 6;
				switch (cfg->client.crosshairType)
				{
				case Config::Configuration::ECrosshairs::ENone:
					break;
				case Config::Configuration::ECrosshairs::ECircle:
					drawList->AddCircle({ io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f }, cfg->client.crosshairSize, ImGui::GetColorU32(cfg->client.crosshairColor), 0, cfg->client.crosshairThickness);
					break;
				case Config::Configuration::ECrosshairs::ECross:
					drawList->AddLine({ io.DisplaySize.x * 0.5f - cfg->client.crosshairSize, io.DisplaySize.y * 0.5f }, { io.DisplaySize.x * 0.5f + cfg->client.crosshairSize, io.DisplaySize.y * 0.5f }, ImGui::GetColorU32(cfg->client.crosshairColor), cfg->client.crosshairThickness);
					drawList->AddLine({ io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f - cfg->client.crosshairSize }, { io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f + cfg->client.crosshairSize }, ImGui::GetColorU32(cfg->client.crosshairColor), cfg->client.crosshairThickness);
					break;
				default:
					break;
				}
			}
			if (cfg->client.bCompass)
			{
				const char* directions[] = { "E", "NE", "N", "NW", "W", "SW", "S", "SE" };
				int yaw = ((int)cameraRotation.Yaw + 450) % 360;
				float angleStep = 45.0f * (3.14159f / 180.0f); 

				ImVec2 centerPos = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.05f); 
				float radius = 45.0f;
				auto col = ImVec4(1.f, 1.f, 1.f, 1.f);

				drawList->AddCircleFilled(centerPos, radius, IM_COL32(100, 100, 100, 50));

				float arrowLength = 25.0f; 
				ImVec2 arrowPos = ImVec2(centerPos.x, centerPos.y - arrowLength);
				ImVec2 p1 = ImVec2(arrowPos.x - 5.0f, arrowPos.y + 10.0f);
				ImVec2 p2 = ImVec2(arrowPos.x + 5.0f, arrowPos.y + 10.0f);
				drawList->AddTriangleFilled(arrowPos, p1, p2, IM_COL32(255, 255, 255, 255));

				for (int i = 0; i < 8; ++i)
				{
					float angle = -i * angleStep - (yaw * (3.14159f / 180.0f));
					ImVec2 numPos = ImVec2(centerPos.x + (radius - 5.0f) * cos(angle), centerPos.y + (radius - 5.0f) * sin(angle));
					ImVec2 textSize = ImGui::CalcTextSize(directions[i]);

					if (i == 1 || i == 3 || i == 5 || i == 7) 
						RenderText(drawList, directions[i], ImVec2(numPos.x - textSize.x * 0.4f, numPos.y - textSize.y * 0.4f), col, 12, true, true);
					else
						RenderText(drawList, directions[i], ImVec2(numPos.x - textSize.x * 0.5f, numPos.y - textSize.y * 0.5f), col, 15, true, true);

					float nextAngle = (-i - 1.0f) * angleStep - (yaw * (3.14159f / 180.0f));
					ImVec2 dotPos = ImVec2(centerPos.x + (radius - 5.0f) * cos((angle + nextAngle) / 2), centerPos.y + (radius - 5.0f) * sin((angle + nextAngle) / 2));

					float dist = sqrt(pow(dotPos.x - centerPos.x, 2) + pow(dotPos.y - centerPos.y, 2));
					float scaleFactor = radius / dist;

					if (scaleFactor < 1.0f)
					{
						dotPos.x = centerPos.x + (dotPos.x - centerPos.x) * scaleFactor;
						dotPos.y = centerPos.y + (dotPos.y - centerPos.y) * scaleFactor;
					}

					drawList->AddCircleFilled(dotPos, 2.0f, IM_COL32(255, 255, 255, 255));
				}
			}
		}
		if (cfg->esp.enable)
		{
			if (cfg->esp.islands.enable)
			{
				error_code = 7;
				const auto entries = AthenaGameViewportClient->World->GameState->IslandService->IslandDataAsset->IslandDataEntries;
				for (auto i = 0u; i < entries.Count; i++)
				{
					error_code = 8;
					auto const island = entries[i];
					auto const WorldMapData = island->WorldMapData;
					if (!WorldMapData) continue;
					const FVector islandLoc = WorldMapData->CaptureParams.WorldSpaceCameraPosition;
					const float dist = myLocation.DistTo(islandLoc) * 0.01f;
					if ((islandLoc.DistTo(FVector(0.f, 0.f, 0.f)) * 0.01) < 50.f) continue;
					if (dist > cfg->esp.islands.renderDistance) continue;
					FVector2D screen;
					if (playerController->ProjectWorldLocationToScreen(islandLoc, screen))
					{
						char buf[0x64];
						auto len = island->LocalisedName->multi(buf, 0x50);
						sprintf_s(buf + len, sizeof(buf) - len, " [%.0fm]", dist);
						RenderText(drawList, buf, screen, cfg->esp.islands.color, (int)cfg->esp.islands.size + 10);
					}
				}
			}
		}
		if (cfg->aim.enable)
		{
			if (cfg->aim.cannon.enable && attachObject && attachObject->isCannon())
			{
				auto cannonObj = reinterpret_cast<ACannon*>(attachObject);
				auto loaded_item = reinterpret_cast<ACannonLoadedItemInfo*>(attachObject);

				if (loaded_item->LoadedItemInfo)
				{
					std::wstring loaded_name = loaded_item->LoadedItemInfo->Desc->Title->wide();
					if (loaded_name.find(L"Disparo") != std::wstring::npos || loaded_name.find(L"Chain") != std::wstring::npos)
						cfg->aim.cannon.chains = true;
					else if (loaded_name.find(L"Jugador") != std::wstring::npos || loaded_name.find(L"Player") != std::wstring::npos)
						cfg->aim.cannon.deckshots = true;
					else
					{
						cfg->aim.cannon.chains = false;
						cfg->aim.cannon.deckshots = false;
					}
					char bufff[0x64];
					ZeroMemory(bufff, sizeof(bufff));
					sprintf_s(bufff, _countof(bufff), "%ws", loaded_name.c_str());
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 20, ImVec2(1656.f, 225.f), ImColor(255, 255, 255, 255), bufff, 0, 0.0f, 0);
				}

				error_code = 801;
				if (GetAsyncKeyState(VK_F1) & 1)
				{
					cfg->aim.cannon.chains = !cfg->aim.cannon.chains;
					if (cfg->aim.cannon.chains)
						cfg->aim.cannon.deckshots = false;
				}
				if (GetAsyncKeyState(VK_F2) & 1)
				{
					cfg->aim.cannon.deckshots = !cfg->aim.cannon.deckshots;
					if (cfg->aim.cannon.deckshots)
						cfg->aim.cannon.chains = false;
				}
				if (GetAsyncKeyState(VK_F3) & 1)
				{
					cfg->aim.cannon.lowAim = !cfg->aim.cannon.lowAim;
				}
				if (GetAsyncKeyState(VK_F4) & 1)
				{
					cfg->aim.cannon.drawPred = !cfg->aim.cannon.drawPred;
				}

				int cannonlocalsets = 0;

				if (cfg->aim.cannon.players)
				{
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f), ImColor(0, 0, 0, 255), "Player Cannon Aimbot", 0, 0.0f, 0);
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f), ImColor(255, 165, 0, 255), "Player Cannon Aimbot", 0, 0.0f, 0);
					cannonlocalsets++;
				}
				if (cfg->aim.cannon.skeletons)
				{
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(0, 0, 0, 255), "Skeleton Aimbot", 0, 0.0f, 0);
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(255, 165, 0, 255), "Skeleton Aimbot", 0, 0.0f, 0);
					cannonlocalsets++;
				}
				if (cfg->aim.cannon.chains)
				{
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(0, 0, 0, 255), "Chain Aimbot", 0, 0.0f, 0);
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(255, 165, 0, 255), "Chain Aimbot", 0, 0.0f, 0);
					cannonlocalsets++;
				}
				if (cfg->aim.cannon.deckshots)
				{
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(0, 0, 0, 255), "Deck Shots", 0, 0.0f, 0);
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(255, 165, 0, 255), "Deck Shots", 0, 0.0f, 0);
					cannonlocalsets++;
				}
				if (cfg->aim.cannon.visibleOnly)
				{
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(0, 0, 0, 255), "Visible Only", 0, 0.0f, 0);
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(255, 165, 0, 255), "Visible Only", 0, 0.0f, 0);
					cannonlocalsets++;
				}
				if (cfg->aim.cannon.instant)
				{
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(0, 0, 0, 255), "Instant Shoot", 0, 0.0f, 0);
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(255, 165, 0, 255), "Instant Shoot", 0, 0.0f, 0);
					cannonlocalsets++;
				}
				if (cfg->aim.cannon.ghostShips)
				{
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(0, 0, 0, 255), "Ghost Ships", 0, 0.0f, 0);
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(255, 165, 0, 255), "Ghost Ships", 0, 0.0f, 0);
					cannonlocalsets++;
				}
				if (cfg->aim.cannon.lowAim)
				{
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(0, 0, 0, 255), "Damage Zone", 0, 0.0f, 0);
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(255, 165, 0, 255), "Damage Zone", 0, 0.0f, 0);
					cannonlocalsets++;
				}
				if (cfg->aim.cannon.drawPred)
				{
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(0, 0, 0, 255), "Cannon Pre-Fire", 0, 0.0f, 0);
					ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(cannonlocalsets * 20)), ImColor(255, 165, 0, 255), "Cannon Pre-Fire", 0, 0.0f, 0);
					
				}

				error_code = 9;
				auto cannon = reinterpret_cast<ACannon*>(attachObject);
				float gravity_scale = cannon->ProjectileGravityScale;
				int localsets = 0;


				if (cfg->aim.cannon.drawPred)
				{
					error_code = 151;
					float gravity = 981.f * gravity_scale;
					float launchspeed = cannon->ProjectileSpeed;
					ACannonLoadedItemInfo* loadedItemInfo = reinterpret_cast<ACannonLoadedItemInfo*>(attachObject);
					FRotator angle = { loadedItemInfo->ServerPitch, loadedItemInfo->ServerYaw, 0.f };
					FRotator comp_angle = attachObject->K2_GetActorRotation();
					angle += comp_angle;
					FVector vForward = UKismetMathLibrary::Conv_RotatorToVector(angle);
					FVector pos = attachObject->K2_GetActorLocation();
					pos.Z += 100;
					pos = pos + (vForward * 150);
					FVector vel = vForward * launchspeed;
					if (localPlayerActor->GetCurrentShip())
						vel = vel + localPlayerActor->GetCurrentShip()->GetVelocity();
					std::vector<FVector> path;
					int count = 250;
					bool hit = GetProjectilePath(path, vel, pos, gravity, count, world);
					FVector2D screen_pos_prev;
					for (int i = 0; i < path.size(); i++)
					{
						FVector2D screen_pos;
						bool is_on_screen = playerController->ProjectWorldLocationToScreen(path[i], screen_pos);
						if (is_on_screen) {
							if (hit && i == path.size() - 1) {
								
								ImVec2 center = { screen_pos.X, screen_pos.Y };
								float radius = 7.0f;
								ImU32 color_outer = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 1.f, 0.f, 1.f));
								ImU32 color_inner = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 0.8f, 0.f, 1.f));
								drawList->AddCircleFilled(center, radius, color_outer, 32);
								drawList->AddCircleFilled(center, radius - 2, color_inner, 32);
							}
							else if (i >= 1) {
								
								ImVec2 p1 = { screen_pos_prev.X, screen_pos_prev.Y };
								ImVec2 p2 = { screen_pos.X, screen_pos.Y };
								ImU32 color_start = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 0.f, 0.f, 1.f));
								ImU32 color_end = ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 0.5f, 0.f, 1.f));

								
								const int segments = 32;
								for (int j = 0; j <= segments; ++j) {
									float t = static_cast<float>(j) / static_cast<float>(segments);
									ImVec2 point = ImVec2(
										p1.x * (1 - t) + p2.x * t,
										p1.y * (1 - t) + p2.y * t
									);
									ImU32 color = ImColor(
										color_start + static_cast<float>(j) / static_cast<float>(segments) * (color_end - color_start)
									);
									drawList->AddLine(point, point, color, 2);
								}

								
								ImVec2 circle1 = { screen_pos_prev.X, screen_pos_prev.Y };
								ImVec2 circle2 = { screen_pos.X, screen_pos.Y };
								ImU32 line_color = ImGui::ColorConvertFloat4ToU32(ImVec4(0.f, 0.f, 1.f, 1.f)); 
								drawList->AddLine(circle1, circle2, line_color, 2);
							}
							screen_pos_prev = screen_pos;
						}

					}
				}
			}
			if (isWieldedWeapon)
			{
				if (GetAsyncKeyState(VK_F1) & 1)
				{
					cfg->aim.others.rage = !cfg->aim.others.rage;
					if (!cfg->aim.others.rage)
						engine::aimTarget = nullptr;
				}
				do {
					if (attachObject && attachObject->isCannon()) break;

					int localsets = 0;
					if (cfg->aim.weapon.trigger)
					{
						ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(localsets * 20)), ImColor(0, 0, 0, 255), "Instant Shoot", 0, 0.0f, 0);
						ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(localsets * 20)), ImColor(255, 165, 0, 255), "Instant Shoot", 0, 0.0f, 0);
						localsets++;
					}
					if (cfg->aim.weapon.visibleOnly)
					{
						ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(localsets * 20)), ImColor(0, 0, 0, 255), "Visible Only", 0, 0.0f, 0);
						ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(localsets * 20)), ImColor(255, 165, 0, 255), "Visible Only", 0, 0.0f, 0);
						localsets++;
					}
					if (cfg->aim.weapon.players)
					{
						ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(localsets * 20)), ImColor(0, 0, 0, 255), "Player Aimbot", 0, 0.0f, 0);
						ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(localsets * 20)), ImColor(255, 165, 0, 255), "Player Aimbot", 0, 0.0f, 0);
						localsets++;
					}
					if (cfg->aim.weapon.skeletons)
					{
						ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(localsets * 20)), ImColor(0, 0, 0, 255), "Skeletons Aimbot", 0, 0.0f, 0);
						ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(localsets * 20)), ImColor(255, 165, 0, 255), "Skeletons Aimbot", 0, 0.0f, 0);
						localsets++;
					}
					if (cfg->aim.weapon.kegs)
					{
						ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(localsets * 20)), ImColor(0, 0, 0, 255), "Kegs Aimbot", 0, 0.0f, 0);
						ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(localsets * 20)), ImColor(255, 165, 0, 255), "Kegs Aimbot", 0, 0.0f, 0);
						localsets++;
					}
					if (cfg->aim.others.rage)
					{
						ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(localsets * 20)), ImColor(0, 0, 0, 255), "Beta Aim", 0, 0.0f, 0);
						ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(localsets * 20)), ImColor(255, 165, 0, 255), "Beta Aim", 0, 0.0f, 0);
						localsets++;
					}
					if (cfg->aim.weapon.fastweapon)
					{
						ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1656.f, 303.f + (float)(localsets * 20)), ImColor(0, 0, 0, 255), "Fast Reload", 0, 0.0f, 0);
						ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), 16, ImVec2(1655.f, 303.f + (float)(localsets * 20)), ImColor(255, 165, 0, 255), "Fast Reload", 0, 0.0f, 0);
						localsets++;
					}
				} while (false);
			}
		}
		if (cfg->game.enable)
		{

			if (cfg->game.showSunk)
			{
				error_code = 152;
				for (int j = 0; j < trackedSinkLocs.size(); j++)
				{
					FVector2D screen;
					FVector loc = trackedSinkLocs[j];
					loc.Z = 0.f;
					if (playerController->ProjectWorldLocationToScreen(loc, screen))
					{

						const int dist = myLocation.DistTo(loc) * 0.01f;
						char buf[0x64];
						snprintf(buf, sizeof(buf), "Sinking Site [%dm]", dist);
						RenderText(drawList, buf, screen, cfg->game.sunkColor, 16);
					}
				}
			}
			if (cfg->game.playerList) {
				try {
					ImGui::SetNextWindowSize(ImVec2(120.f, 300.f), ImGuiCond_Once);
					ImGui::SetNextWindowPos(ImVec2(10.f, 180.f), ImGuiCond_Once);
					ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 2.0f);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.8f);

					ImGui::Begin("PlayersList", 0,
						ImGuiWindowFlags_NoSavedSettings |
						ImGuiWindowFlags_AlwaysAutoResize | ImGuiOldColumnFlags_NoResize |
						ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
						ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoFocusOnAppearing);

					auto shipsService = gameState->ShipService;
					if (shipsService) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
						ImGui::BeginChild("Info", { 0.f, 18.f });
						ImGui::Text(" Total Ships + AI : %d", shipsService->GetNumShips());
						ImGui::EndChild();
						ImGui::PopStyleColor();
					}

					auto crewService = gameState->CrewService;
					auto crews = crewService->Crews;
					if (crews.Data) {
						ImGui::Separator();

						for (uint32_t i = 0; i < crews.Count; i++) {
							auto& crew = crews[i];
							auto players = crew.Players;

							ImVec4 shipColor;
							int crewMaxSize = crew.CrewSessionTemplate.MaxPlayers;
							switch (crewMaxSize) {
							case 2:
								shipColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
								break;
							case 3:
								shipColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
								break;
							case 4:
								shipColor = ImVec4(0.0f, 1.0f, 1.0f, 1.0f); 
								break;
							default:
								shipColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f); 
								break;
							}

							ImGui::PushStyleColor(ImGuiCol_Text, shipColor);

							ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize("Ship").x) * 0.5f);
							ImGui::Text("%s", (crewMaxSize == 2) ? "Sloop" : (crewMaxSize == 3) ? "Brig" : (crewMaxSize == 4) ? "Galle" : "Ship");
							ImGui::PopStyleColor();

							for (uint32_t k = 0; k < players.Count; k++) {
								auto& player = players[k];
								char buf[0x50];
								ZeroMemory(buf, sizeof(buf));

								int len = 0;
								char bufName[0x50];
								if (player->PlayerName.Count > 0) {
									player->PlayerName.multi(bufName, 0x50);
									len = sprintf_s(buf, sizeof(buf), "%s", bufName);
								}
								else {
									len = sprintf_s(buf, sizeof(buf), "Unknown");
								}

								
								ImGui::PushStyleColor(ImGuiCol_Text, shipColor);

								
								ImGui::Text("   [%d] ", k + 1);

								ImGui::PopStyleColor();

								
								ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));

								ImGui::SameLine(); 

								ImGui::Text(buf);

								ImGui::PopStyleColor();
							}
							ImGui::Text("");
							ImGui::Separator();
						}
					}

					ImGui::End();
					ImGui::PopStyleVar(2);
					ImGui::PopStyleColor(2);
				}
				catch (...) {
					ImGui::PopStyleColor();
					ImGui::PopStyleVar(2);

					if (cfg->dev.printErrorCodes)
						tslog::debug("Exception in Render Thread. Error Code: 111.");

					ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
					ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
					return;
				}
			}

			if (reinterpret_cast<AOnlineAthenaPlayerController*>(playerController)->IdleDisconnectEnabled && cfg->game.noIdleKick)
			{
				reinterpret_cast<AOnlineAthenaPlayerController*>(playerController)->IdleDisconnectEnabled = false;
			}

		}
		else
		{
			if (!reinterpret_cast<AOnlineAthenaPlayerController*>(playerController)->IdleDisconnectEnabled && !cfg->game.noIdleKick)
			{
				reinterpret_cast<AOnlineAthenaPlayerController*>(playerController)->IdleDisconnectEnabled = true;
			}
		}

		if (GetAsyncKeyState(VK_F8) & 1)
		{
			cfg->game.walkUnderwater = !cfg->game.walkUnderwater;
		}

		if (cfg->game.enable)
		{
			if (cfg->game.walkUnderwater)
			{
				if (GetAsyncKeyState(0x43))
				{
					localPlayerActor->CharacterMovementComponent->SwimParams.EnterSwimmingDepth = 15000.f;
					localPlayerActor->CharacterMovementComponent->SwimParams.ExitSwimmingDepth = 15000.f;
				}
				else
				{
					float playerZ = abs(localPlayerActor->K2_GetActorLocation().Z) + 170.f;

					localPlayerActor->CharacterMovementComponent->SwimParams.EnterSwimmingDepth = playerZ;
					localPlayerActor->CharacterMovementComponent->SwimParams.ExitSwimmingDepth = playerZ;
				}
			}
			else if (!cfg->game.walkUnderwater)
			{
				localPlayerActor->CharacterMovementComponent->SwimParams.EnterSwimmingDepth = 120.f;
				localPlayerActor->CharacterMovementComponent->SwimParams.ExitSwimmingDepth = 120.f;
			}
		}

		if (cfg->game.b_bunnyhop)
		{
			static bool holdToggle = false;

			if (GetAsyncKeyState(VK_SPACE) & 0x8000)
			{
				holdToggle = true;

				static std::uintptr_t desiredTime = 0;
				if (milliseconds_now() >= desiredTime)
				{
					if (localPlayerActor->IsCanJump())
					{
						localPlayerActor->IsJump();
					}
					desiredTime = milliseconds_now() + 100;
				}
			}
			else if (holdToggle)
			{
				
				for (int i = 8; i <= 255; ++i) 
				{
					if (i != VK_SPACE && GetAsyncKeyState(i) & 0x8001)
					{
						if (localPlayerActor->IsCanJump())
						{
							localPlayerActor->IsJump();
						}
						break;
					}
				}

				holdToggle = false;
			}
		}

		if (cfg->dev.interceptProcessEvent && engine::oProcessEvent == nullptr)
		{
			hookProcessEvent();
		}
		else if (!cfg->dev.interceptProcessEvent && engine::oProcessEvent != nullptr)
		{
			unhookProcessEvent();
		}
		static int rareSpotsCounter = 0;
		rareSpotsCounter = 0;
		for (UINT32 i = 0; i < levels.Count; i++)
		{
			error_code = 12;
			if (!levels[i])
				continue;
			TArray<ACharacter*> actors = levels[i]->AActors;
			for (UINT32 j = 0; j < actors.Count; j++)
			{
				ACharacter* actor = actors[j];
				if (!actor)
					continue;

				if (cfg->dev.debugNames)
				{
					error_code = 121;
					const FVector location = actor->K2_GetActorLocation();
					const float dist = myLocation.DistTo(location) * 0.01f;
					FVector2D screen;
					if (dist <= cfg->dev.debugNamesRenderDistance)
					{
						if (playerController->ProjectWorldLocationToScreen(location, screen))
						{
							std::string actorName = actor->GetName();
							char tmpBuf[0x64];
							int filterSize = sprintf_s(tmpBuf, sizeof(tmpBuf), cfg->dev.debugNamesFilter);
							if (filterSize == 0 || (filterSize > 0 && actor->compareName(tmpBuf)))
							{
								char buf[0x64];
								ZeroMemory(buf, sizeof(buf));
								sprintf_s(buf, sizeof(buf), actor->GetName().c_str());
								RenderText(drawList, buf, screen, { 1.f,1.f,1.f,1.f }, cfg->dev.debugNamesTextSize);
							}
						}
					}
				}
				if (cfg->client.enable)
				{
					if (actor->isLightningController())
					{
						auto lCon = reinterpret_cast<ALightingController*>(actor);

						if (cfg->client.bCustomTOD)
						{
							lCon->IsFixedTimeOfDay = true;
							lCon->FixedTimeOfDay = cfg->client.customTOD;
						}
						else if (lCon->IsFixedTimeOfDay)
						{
							lCon->FixedTimeOfDay = 0.f;
							lCon->IsFixedTimeOfDay = false;
						}
					}
				}

				if (cfg->esp.enable)
				{
					do
					{

						error_code = 13;
						if (actor == localPlayerActor) break;
						if (cfg->esp.players.enable)
						{
							if (actor->isPlayer() && actor != localPlayerActor && !actor->IsDead())
							{
								error_code = 14;
								const bool isTeamMate = UCrewFunctions::AreCharactersInSameCrew(actor, localPlayerActor);
								if (!cfg->esp.players.team && isTeamMate) break;
								const FVector location = actor->K2_GetActorLocation();
								const float dist = myLocation.DistTo(location) * 0.01f;
								if (dist >= cfg->esp.players.renderDistance) break;

								FVector origin, extent;
								actor->GetActorBounds(true, origin, extent);
								FVector2D headPos;
								playerController->ProjectWorldLocationToScreen({ location.X, location.Y, location.Z + extent.Z }, headPos);
								FVector2D footPos;
								playerController->ProjectWorldLocationToScreen({ location.X, location.Y, location.Z - extent.Z }, footPos);
								const float height = abs(footPos.Y - headPos.Y);
								const float width = height * 0.4f;

								ImVec4 color = (playerController->LineOfSightTo(actor, cameraLocation, false)) ? cfg->esp.players.colorVisible : cfg->esp.players.colorInvisible;
								if (!isTeamMate)
									(drawList, headPos, footPos, height, width, color);

								FVector2D screen;
								if (cfg->esp.players.bName && playerController->ProjectWorldLocationToScreen(location, screen))
								{
									auto const playerState = actor->PlayerState;
									if (!playerState) break;
									const auto playerName = playerState->PlayerName;
									if (!playerName.Data) break;

									char buf[0x64];
									ZeroMemory(buf, sizeof(buf));
									const int len = playerName.multi(buf, 0x20);
									sprintf_s(buf + len, sizeof(buf) - len, " [%.0fm]", dist);
									RenderText(drawList, buf, screen, color, dist);
								}

								switch (cfg->esp.players.boxType)
								{
								case Config::Configuration::EBox::E2DBoxes:
								{
									Render2DBox(drawList, headPos, footPos, height, width, color);
									break;
								}
								case Config::Configuration::EBox::E3DBoxes:
								{
									FRotator rotation = actor->K2_GetActorRotation();
									FVector ext = { 35.f, 35.f, extent.Z };
									if (!Render3DBox(playerController, location, ext, rotation, color)) continue;
									break;
								}
								}

								if (cfg->esp.players.tracers && !isTeamMate)
								{
									error_code = 15;
									Vector2 bScreen = Vector2();
									float fov = cfg->client.fov;
									if (WorldToScreen(Vector3(location.X, location.Y, location.Z), &bScreen, cameraLocation, cameraRotation, fov))
									{
										auto col = ImGui::GetColorU32(color);
										drawList->AddLine({ io.DisplaySize.x * 0.5f , io.DisplaySize.y * 0.5f }, { bScreen.x, bScreen.y }, col, cfg->esp.players.tracersThickness);
									}
								}
								
								if (cfg->esp.players.barType != Config::Configuration::EBar::ENone)
								{
									auto healthComp = reinterpret_cast<AHealthComponent*>(actor)->HealthComponent;
									if (!healthComp)
										continue;
									const float hp = healthComp->GetCurrentHealth() / healthComp->GetMaxHealth();
									const float width2 = width * 0.5f;
									const float adjust = height * 0.025f;
									switch (cfg->esp.players.barType)
									{
									case Config::Configuration::EBar::ELeft:
									{
										const float len = height * hp;
										drawList->AddRectFilled({ headPos.X - width2 - adjust * 2.f, headPos.Y }, { headPos.X - width2 - adjust, footPos.Y - len }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
										drawList->AddRectFilled({ headPos.X - width2 - adjust * 2.f, footPos.Y - len }, { headPos.X - width2 - adjust, footPos.Y }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
										break;
									}
									case Config::Configuration::EBar::ERight:
									{
										const float len = height * hp;
										drawList->AddRectFilled({ headPos.X + width2 + adjust, headPos.Y }, { headPos.X + width2 + adjust * 2.f, footPos.Y - len }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
										drawList->AddRectFilled({ headPos.X + width2 + adjust, footPos.Y - len }, { headPos.X + width2 + adjust * 2.f, footPos.Y }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
										break;
									}
									case Config::Configuration::EBar::EBottom:
									{
										const float len = width * hp;
										drawList->AddRectFilled({ headPos.X - width2, footPos.Y + adjust }, { headPos.X - width2 + len, footPos.Y + adjust * 2.f }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
										drawList->AddRectFilled({ headPos.X - width2 + len, footPos.Y + adjust }, { headPos.X + width2, footPos.Y + adjust * 2.f }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
										break;
									}
									case Config::Configuration::EBar::ETop:
									{
										const float len = width * hp;
										drawList->AddRectFilled({ headPos.X - width2, headPos.Y - adjust * 2.f }, { headPos.X - width2 + len, headPos.Y - adjust }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
										drawList->AddRectFilled({ headPos.X - width2 + len, headPos.Y - adjust * 2.f }, { headPos.X + width2, headPos.Y - adjust }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
										break;
									}
									}

								}
								if (cfg->esp.players.bSkeleton)
								{
									auto const mesh = reinterpret_cast<AUSkeletalMeshComponent*>(actor)->Mesh;
									if (!reinterpret_cast<AUSkeletalMeshComponent*>(actor)) continue;
									const BYTE bodyHead[] = { 4, 5, 6, 51, 7, 6, 80, 7, 8, 9 };
									const BYTE neckHandR[] = { 80, 81, 82, 83, 84 };
									const BYTE neckHandL[] = { 51, 52, 53, 54, 55 };
									const BYTE bodyFootR[] = { 4, 111, 112, 113, 114 };
									const BYTE bodyFootL[] = { 4, 106, 107, 108, 109 };
									const std::pair<const BYTE*, const BYTE> skeleton[] = { {bodyHead, 10}, {neckHandR, 5}, {neckHandL, 5}, {bodyFootR, 5}, {bodyFootL, 5} };
									const FMatrix comp2world = mesh->K2_GetComponentToWorld().ToMatrixWithScale();
									if (!RenderSkeleton(drawList, playerController, mesh, comp2world, skeleton, 5, color)) continue;
								}
							}
						}
					} while (false);
					if (cfg->esp.skeletons.enable)
					{
						do
						{
							if (actor->isSkeleton() && !actor->IsDead())
							{
								error_code = 16;
								const FVector location = actor->K2_GetActorLocation();
								const float dist = myLocation.DistTo(location) * 0.01f;
								FVector origin, extent;
								actor->GetActorBounds(true, origin, extent);

								
								FVector2D headPos;
								if (!playerController->ProjectWorldLocationToScreen({ location.X, location.Y, location.Z + extent.Z }, headPos)) continue;
								FVector2D footPos;
								if (!playerController->ProjectWorldLocationToScreen({ location.X, location.Y, location.Z - extent.Z }, footPos)) continue;

								const float height = abs(footPos.Y - headPos.Y);
								const float width = height * 0.4f;

								const bool bVisible = playerController->LineOfSightTo(actor, cameraLocation, false);
								const ImVec4 col = bVisible ? cfg->esp.skeletons.colorVis : cfg->esp.skeletons.colorInv;
								if (dist >= cfg->esp.skeletons.renderDistance) break;;
								FVector2D screen;
								if (playerController->ProjectWorldLocationToScreen(location, screen))
								{

									char buf[0x64];
									ZeroMemory(buf, sizeof(buf));
									if (dist >= 50)
									{
										sprintf_s(buf, sizeof(buf), "Skeleton [%.0fm]", dist);
									}
									RenderText(drawList, buf, screen, cfg->esp.skeletons.color, 14);
								}

								if (cfg->esp.skeletons.bSkeleton)
								{
									auto const mesh = reinterpret_cast<AUSkeletalMeshComponent*>(actor)->Mesh;
									if (!reinterpret_cast<AUSkeletalMeshComponent*>(actor)) continue;

									const BYTE bodyHead[] = { 4, 5, 6, 7, 8, 9 };
									const BYTE neckHandR[] = { 7, 41, 42, 43 };
									const BYTE neckHandL[] = { 7, 12, 13, 14 };
									const BYTE bodyFootR[] = { 4, 71, 72, 73, 74 };
									const BYTE bodyFootL[] = { 4, 66, 67, 68, 69 };

									const std::pair<const BYTE*, const BYTE> skeleton[] = { {bodyHead, 6}, {neckHandR, 4}, {neckHandL, 4}, {bodyFootR, 5}, {bodyFootL, 5} };

									const FMatrix comp2world = mesh->K2_GetComponentToWorld().ToMatrixWithScale();

									if (!RenderSkeleton(drawList, playerController, mesh, comp2world, skeleton, 5, col)) continue;


								}
								switch (cfg->esp.skeletons.boxType)
								{
								case Config::Configuration::EBox::E2DBoxes:
								{
									Render2DBox(drawList, headPos, footPos, height, width, col);
									break;
								}
								case Config::Configuration::EBox::E3DBoxes:
								{
									FRotator rotation = actor->K2_GetActorRotation();
									if (!Render3DBox(playerController, origin, extent, rotation, col)) continue;
									break;
								}
								}
								if (cfg->esp.skeletons.barType != Config::Configuration::EBar::ENone)
								{
									auto healthComp = reinterpret_cast<AHealthComponent*>(actor)->HealthComponent;
									if (!healthComp)
										continue;
									const float hp = healthComp->GetCurrentHealth() / healthComp->GetMaxHealth();
									const float width2 = width * 0.5f;
									const float adjust = height * 0.025f;

									switch (cfg->esp.skeletons.barType)
									{
									case Config::Configuration::EBar::ELeft:
									{
										const float len = height * hp;
										drawList->AddRectFilled({ headPos.X - width2 - adjust * 2.f, headPos.Y }, { headPos.X - width2 - adjust, footPos.Y - len }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
										drawList->AddRectFilled({ headPos.X - width2 - adjust * 2.f, footPos.Y - len }, { headPos.X - width2 - adjust, footPos.Y }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
										break;
									}
									case Config::Configuration::EBar::ERight:
									{
										const float len = height * hp;
										drawList->AddRectFilled({ headPos.X + width2 + adjust, headPos.Y }, { headPos.X + width2 + adjust * 2.f, footPos.Y - len }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
										drawList->AddRectFilled({ headPos.X + width2 + adjust, footPos.Y - len }, { headPos.X + width2 + adjust * 2.f, footPos.Y }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
										break;
									}
									case Config::Configuration::EBar::EBottom:
									{
										const float len = width * hp;
										drawList->AddRectFilled({ headPos.X - width2, footPos.Y + adjust }, { headPos.X - width2 + len, footPos.Y + adjust * 2.f }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
										drawList->AddRectFilled({ headPos.X - width2 + len, footPos.Y + adjust }, { headPos.X + width2, footPos.Y + adjust * 2.f }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
										break;
									}
									case Config::Configuration::EBar::ETop:
									{
										const float len = width * hp;
										drawList->AddRectFilled({ headPos.X - width2, headPos.Y - adjust * 2.f }, { headPos.X - width2 + len, headPos.Y - adjust }, ImGui::GetColorU32(IM_COL32(0, 255, 0, 255)));
										drawList->AddRectFilled({ headPos.X - width2 + len, headPos.Y - adjust * 2.f }, { headPos.X + width2, headPos.Y - adjust }, ImGui::GetColorU32(IM_COL32(255, 0, 0, 255)));
										break;
									}
									}

								}

							}
						} while (false);
					}
					if (cfg->esp.ships.enable)
					{
						do
						{
							error_code = 17;

							if (actor->isShip() || actor->isFarShip())
							{
								FVector location = actor->K2_GetActorLocation();
								const float dist = myLocation.DistTo(location) * 0.01f;
								location.Z = 3000;
								if (dist >= cfg->esp.ships.renderDistance) break;
								FVector2D screen;
								if (playerController->ProjectWorldLocationToScreen(location, screen))
								{
									char buf[0x64];
									ZeroMemory(buf, sizeof(buf));
									const float velocity = (actor->GetVelocity() * 0.01f).Size();
									float internalWater = 0.f;
									if (!actor->isFarShip()) internalWater = actor->GetInternalWater()->GetNormalizedWaterAmount() * 100.f;

									if (actor->isShip() && dist < 1726)
									{
										if (actor->compareName("BP_Small"))
											sprintf_s(buf, sizeof(buf), ICON_FA_SHIP " Sloop [%.0f%%][%.0fm][%.0fm/s]", internalWater, dist, velocity);
										else if (actor->compareName("BP_Medium"))
											sprintf_s(buf, sizeof(buf), ICON_FA_SHIP " Brigatine [%.0f%%][%.0fm][%.0fm/s]", internalWater, dist, velocity);
										else if (actor->compareName("BP_Large"))
											sprintf_s(buf, sizeof(buf), ICON_FA_SHIP " Galleon [%.0f%%][%.0fm][%.0fm/s]", internalWater, dist, velocity);
										else if (actor->compareName("AISmall") && cfg->esp.ships.skeletons)
											sprintf_s(buf, sizeof(buf), ICON_FA_SHIP " AI Sloop [%.0f%%][%.0fm][%.0fm/s]", internalWater, dist, velocity);
										else if (actor->compareName("AILarge") && cfg->esp.ships.skeletons)
											sprintf_s(buf, sizeof(buf), ICON_FA_SHIP " AI Galleon [%.0f%%][%.0fm][%.0fm/s]", internalWater, dist, velocity);

										RenderText(drawList, buf, screen, cfg->esp.ships.color, 14);
									}
									else if (actor->isFarShip() && dist > 1725)
									{
										if (actor->compareName("BP_Small"))
											sprintf_s(buf, sizeof(buf), ICON_FA_SHIP " Sloop [%.0fm]", dist);
										else if (actor->compareName("BP_Medium"))
											sprintf_s(buf, sizeof(buf), ICON_FA_SHIP " Brigatine [%.0fm]", dist);
										else if (actor->compareName("BP_Large"))
											sprintf_s(buf, sizeof(buf), ICON_FA_SHIP " Galleon [%.0fm]", dist);
										else if (actor->compareName("AISmall") && cfg->esp.ships.skeletons)
											sprintf_s(buf, sizeof(buf), ICON_FA_SHIP " AI Sloop [%.0fm]", dist);
										else if (actor->compareName("AILarge") && cfg->esp.ships.skeletons)
											sprintf_s(buf, sizeof(buf), ICON_FA_SHIP " AI Galleon [%.0fm]", dist);

										RenderText(drawList, buf, screen, cfg->esp.ships.color, 14);
									}
								}
							}
							if (cfg->esp.ships.ghosts && actor->isGhostShip())
							{
								error_code = 18;
								FVector location = actor->K2_GetActorLocation();
								const float dist = myLocation.DistTo(location) * 0.01f;
								location.Z = 3000;
								FVector2D screen;
								if (playerController->ProjectWorldLocationToScreen(location, screen))
								{
									int lives = reinterpret_cast<AAggressiveGhostShip*>(actor)->NumShotsLeftToKill;
									char buf[0x64];
									ZeroMemory(buf, sizeof(buf));
									const int len = sprintf_s(buf, sizeof(buf), "Ghost Ship [%.0fm] | ", dist);
									switch (lives)
									{
									case 3: snprintf(buf + len, sizeof(buf) - len, "***"); break;
									case 2: snprintf(buf + len, sizeof(buf) - len, "**"); break;
									case 1: snprintf(buf + len, sizeof(buf) - len, "*"); break;
									default: snprintf(buf + len, sizeof(buf) - len, "Lives: %d", lives); break;
									}
									if (lives > 0)
										RenderText(drawList, buf, screen, cfg->esp.ships.color, 14);
								}
							}
							if (actor->isShip())
							{
								error_code = 19;
								const FVector location = actor->K2_GetActorLocation();
								const int dist = myLocation.DistTo(location) * 0.01f;
								FVector2D screen;
								if (cfg->esp.ships.holes && dist <= 225)
								{
									auto const damage = actor->GetHullDamage();
									if (!damage) break;
									const auto holes = damage->ActiveHullDamageZones;
									for (auto h = 0u; h < holes.Count; h++)
									{
										auto const hole = holes[h];
										const FVector location = hole->K2_GetActorLocation();
										if (playerController->ProjectWorldLocationToScreen(location, screen))
										{
											auto color = cfg->esp.ships.holesColor;
											drawList->AddLine({ screen.X - 6.f, screen.Y + 6.f }, { screen.X + 6.f, screen.Y - 6.f }, ImGui::GetColorU32(color));
											drawList->AddLine({ screen.X - 6.f, screen.Y - 6.f }, { screen.X + 6.f, screen.Y + 6.f }, ImGui::GetColorU32(color));
										}
									}
								}
							}

							if (cfg->esp.ships.shipTray)
							{
								error_code = 42;
								if (actor->isShip())
								{
									try
									{
										if (actor->isFarShip() || actor->compareName("AISmall") || actor->compareName("AILarge") || actor->isGhostShip()) break;

										AActor* ship = reinterpret_cast<AActor*>(actor);
										auto angular_velocity = actor->ReplicatedMovement.AngularVelocity;
										auto tangential_velocity = actor->ReplicatedMovement.LinearVelocity;
										tangential_velocity.Z = 0;
										auto speed = FVector(tangential_velocity.X, tangential_velocity.Y, 0).Size();

										float const pi = 3.14159f;
										auto yaw_degrees = FVector(0, 0, angular_velocity.Z).Size();
										auto yaw_radians = (yaw_degrees * pi) / 180;

										auto turn_radius = speed / yaw_radians;
										bool left = angular_velocity.Z > 0.f;
										auto right = ship->GetActorRightVector(); right.Z = 0;
										auto rotated_center_unit = left ? right : right * -1;
										auto actor_location = actor->K2_GetActorLocation();
										actor_location.Z += cfg->esp.ships.shipTrayHeight * 100.f;
										auto center_of_rotation = (rotated_center_unit * turn_radius) + actor_location;
										FLinearColor color = FLinearColor(cfg->esp.ships.shipTrayCol.x, cfg->esp.ships.shipTrayCol.y, cfg->esp.ships.shipTrayCol.z, cfg->esp.ships.shipTrayCol.w);
										UKismetSystemLibrary::DrawDebugCircle(ship, center_of_rotation, turn_radius, 180, color, 0.f, cfg->esp.ships.shipTrayThickness, { 1,0,0 }, { 0,1,0 }, false);
									}
									catch (...) {
										if (cfg->dev.printErrorCodes)
											tslog::debug("Exception in Render Thread. Error Code: 42.");
										return;
									}
								}
							}

							if (cfg->esp.ships.showLadders && actor->isLadder())
							{
								error_code = 191;
								const FVector location = actor->K2_GetActorLocation();
								const float dist = myLocation.DistTo(location) * 0.01f;
								if (dist > 150.f) break;
								FVector2D screen;
								if (playerController->ProjectWorldLocationToScreen(location, screen))
								{
									char buf[0x64];
									ZeroMemory(buf, sizeof(buf));
									if (actor->compareName("ShipLadder_"))
									{
										sprintf_s(buf, sizeof(buf), ICON_FA_LEVEL_UP " [%.0fm]", dist);
										RenderText(drawList, buf, screen, { 1.f, 1.f, 0.f, 1.f }, 13);

									}
								}
							}
						} while (false);

					}

					if (cfg->esp.islands.marks && actor->isXMarkMap())
					{
						error_code = 20;
						if (XMarksMapCount == 1 && item && item->isXMarkMap())
							RenderText(drawList, "Recognized Maps:", { io.DisplaySize.x * 0.85f, io.DisplaySize.y * 0.85f }, { 1.f, 1.f, 0.f, 1.f }, 30);
						auto map = reinterpret_cast<AXMarksTheSpotMap*>(actor);
						FString mapName = map->MapTexturePath;
						auto mapMarks = map->Marks;

						char cMapName[0x64];
						const int len = mapName.multi(cMapName, sizeof(cMapName));
						int mapCode = getMapNameCode(cMapName);

						try
						{
							const auto islandDataEntries = AthenaGameViewportClient->World->GameState->IslandService->IslandDataAsset->IslandDataEntries;

							float scaleFactor = 1.041669;

							for (auto i = 0u; i < islandDataEntries.Count; i++)
							{
								auto island = islandDataEntries[i];
								const char* sIslandName = island->IslandName.GetNameFast();
								char ccIsland[0x64];
								ZeroMemory(ccIsland, sizeof(ccIsland));
								sprintf_s(ccIsland, sizeof(ccIsland), sIslandName);

								int code = getMapNameCode(ccIsland);

								if (mapCode == code)
								{
									std::string buf = getIslandNameByCode(mapCode);
									if (item && item->isXMarkMap())
										RenderText(drawList, buf.c_str(), { io.DisplaySize.x * 0.85f, io.DisplaySize.y * 0.85f + 10 + 20 * XMarksMapCount }, { 0.f, 1.f, 0.f, 1.f }, 22);
									XMarksMapCount++;

									auto const WorldMapData = island->WorldMapData;
									if (!WorldMapData) continue;

									const FVector islandLoc = WorldMapData->CaptureParams.WorldSpaceCameraPosition;
									auto islandOrtho = WorldMapData->CaptureParams.CameraOrthoWidth;


									for (auto i = 0u; i < mapMarks.Count; i++)
									{
										auto mark = mapMarks[i];

										Vector2 v = Vector2(mark.Position);
										Vector2 vectorRotated = RotatePoint(v, Vector2(0.5f, 0.5f), 180 + map->Rotation, false);
										Vector2 vectorAlligned = Vector2(vectorRotated.x - 0.5f, vectorRotated.y - 0.5f);

										float islandScale = islandOrtho / scaleFactor;
										Vector2 offsetPos = vectorAlligned * islandScale;


										FVector digSpot = FVector(islandLoc.X - offsetPos.x, islandLoc.Y - offsetPos.y, 0.f);

										FHitResult hit_result;
										auto start = FVector(digSpot.X, digSpot.Y, 5000.f);

										bool res = raytrace(world, start, digSpot, &hit_result);
										digSpot.Z = hit_result.ImpactPoint.Z;

										FVector2D screen;
										if (playerController->ProjectWorldLocationToScreen(digSpot, screen))
										{
											char buf3[0x64];
											const int dist = myLocation.DistTo(digSpot) * 0.01f;
											if (dist > cfg->esp.islands.marksRenderDistance) continue;
											sprintf_s(buf3, sizeof(buf3), "X [%dm]", dist);
											RenderText(drawList, buf3, screen, cfg->esp.islands.marksColor, 14);
										}

									}
								}
							}
						}
						catch (...) {
							if (cfg->dev.printErrorCodes)
								tslog::debug("Exception in Render Thread. Error Code: 20.");
							return;
						}

					}

					if (cfg->esp.islands.enable)
					{
						if (cfg->esp.islands.vaults && actor->isPuzzleVault())
						{
							do
							{
								error_code = 21;
								auto vault = reinterpret_cast<APuzzleVault*>(actor);
								const FVector location = reinterpret_cast<ACharacter*>(vault->OuterDoor)->K2_GetActorLocation();
								FVector2D screen;
								if (playerController->ProjectWorldLocationToScreen(location, screen))
								{
									char buf[0x64];
									const float dist = myLocation.DistTo(location) * 0.01f;
									if (dist > cfg->esp.islands.vaultsRenderDistance) break;;
									sprintf_s(buf, ICON_FA_KEY " [%.0fm]", dist);
									RenderText(drawList, buf, screen, cfg->esp.islands.vaultsColor, 20.f);
								}
							} while (false);
						}
						if (cfg->esp.islands.barrels && actor->IsA(ABuoyantStorageContainer::StaticClass()))
						{
							auto type = actor->GetName();
							error_code = 22;
							const FVector location = actor->K2_GetActorLocation();
							FVector2D screen;
							const int dist = myLocation.DistTo(location) * 0.01f;
							if (playerController->ProjectWorldLocationToScreen(location, screen) && dist < 500)
							{
								char buf[0x64];
								sprintf_s(buf, ICON_FA_DOT_CIRCLE, "");
								RenderText(drawList, buf, screen, cfg->esp.islands.barrelsColor, 14);
								ABuoyantStorageContainer* barrel = (ABuoyantStorageContainer*)actor;

								

								if (barrel->StorageContainer)
								{
									auto cNodes = barrel->StorageContainer->ContainerNodes.ContainerNodes;

									try {
										for (unsigned int k = 0; k < cNodes.Count; k++)
										{
											FStorageContainerNode node = cNodes[k];

											if (node.ItemDesc)
											{
												UItemDescEx* itemDesc = node.ItemDesc->CreateDefaultObject<UItemDescEx>();

												if (itemDesc)
												{
													if (!cfg->esp.islands.barrelspeek) break;
													char buf2[0x50];
													ZeroMemory(buf2, sizeof(buf2));
													std::string itemName2 = getShortName(itemDesc->GetName());


													int len = sprintf_s(buf2, sizeof(buf2), itemName2.c_str());
													sprintf_s(buf2 + len, sizeof(buf2) - len, " [%d]", node.NumItems);
													screen.Y += 20;
													ImVec4 color = itemName2 == "Chain" ? ImVec4(0.f, 1.f, 0.f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color1 = itemName2 == "Ballast" ? ImVec4(0.82f, 0.24f, 1.f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color2 = itemName2 == "Anchor" ? ImVec4(0.82f, 0.24f, 1.f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color3 = itemName2 == "Barrel" ? ImVec4(0.82f, 0.24f, 1.f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color4 = itemName2 == "Ringging" ? ImVec4(0.82f, 0.24f, 1.f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color5 = itemName2 == "Peace" ? ImVec4(0.82f, 0.24f, 1.f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color6 = itemName2 == "Helm" ? ImVec4(0.82f, 0.24f, 1.f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color7 = itemName2 == "Weary" ? ImVec4(0.16f, 1.f, 0.13f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color8 = itemName2 == "Grog" ? ImVec4(0.16f, 1.f, 0.13f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color9 = itemName2 == "Venom" ? ImVec4(0.16f, 1.f, 0.13f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color10 = itemName2 == "Jig" ? ImVec4(0.16f, 1.f, 0.13f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color11 = itemName2 == "Limp" ? ImVec4(0.16f, 1.f, 0.13f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color12 = itemName2 == "Pine" ? ImVec4(0.98f, 0.67f, 0.f, 1.f) : cfg->esp.islands.barrelsColor;
													if (cfg->esp.islands.barrelstoggle)
													{
														if (GetAsyncKeyState(0x52))
														{
															if (itemName2 == "Chain")
																RenderText(drawList, buf2, screen, { 0.f, 1.f, 0.f, 1.f }, 14);
															else if (itemName2 == "Ballast")
																RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
															else if (itemName2 == "Anchor")
																RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
															else if (itemName2 == "Barrel")
																RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
															else if (itemName2 == "Ringging")
																RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
															else if (itemName2 == "Peace")
																RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
															else if (itemName2 == "Helm")
																RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
															else if (itemName2 == "Weary")
																RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
															else if (itemName2 == "Grog")
																RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
															else if (itemName2 == "Venom")
																RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
															else if (itemName2 == "Jig")
																RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
															else if (itemName2 == "Limp")
																RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
															else if (itemName2 == "Pine")
																RenderText(drawList, buf2, screen, { 0.98f, 0.67f, 0.f, 1.f }, 14);
															else
																RenderText(drawList, buf2, screen, cfg->esp.islands.barrelsColor, 14);
														}
													}
													else
													{
														if (itemName2 == "Chain")
															RenderText(drawList, buf2, screen, { 0.f, 1.f, 0.f, 1.f }, 14);
														else if (itemName2 == "Ballast")
															RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
														else if (itemName2 == "Anchor")
															RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
														else if (itemName2 == "Barrel")
															RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
														else if (itemName2 == "Ringging")
															RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
														else if (itemName2 == "Peace")
															RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
														else if (itemName2 == "Helm")
															RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
														else if (itemName2 == "Weary")
															RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
														else if (itemName2 == "Grog")
															RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
														else if (itemName2 == "Venom")
															RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
														else if (itemName2 == "Jig")
															RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
														else if (itemName2 == "Limp")
															RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
														else if (itemName2 == "Pine")
															RenderText(drawList, buf2, screen, { 0.98f, 0.67f, 0.f, 1.f }, 14);
														else 
															RenderText(drawList, buf2, screen, cfg->esp.islands.barrelsColor, 14);
													}
												}
											}
										}
									}
									catch (...)
									{
										screen.Y -= 23;
										RenderText(drawList, "Invalid", screen, cfg->esp.islands.barrelsColor, 14);
									}
								}
							}
						}

						if (cfg->esp.islands.barrels && actor->isBarrel())
						{
							auto type = actor->GetName();
							error_code = 22;
							const FVector location = actor->K2_GetActorLocation();
							FVector2D screen;
							const int dist = myLocation.DistTo(location) * 0.01f;
							if (playerController->ProjectWorldLocationToScreen(location, screen) && dist < 500)
							{
								char buf[0x64];
								sprintf_s(buf, ICON_FA_DOT_CIRCLE, "");
								RenderText(drawList, buf, screen, cfg->esp.islands.barrelsColor, 14);
								AStorageContainer* barrel = (AStorageContainer*)actor;

								

								if (barrel->StorageContainer)
								{
									auto cNodes = barrel->StorageContainer->ContainerNodes.ContainerNodes;

									try {
										for (unsigned int k = 0; k < cNodes.Count; k++)
										{
											FStorageContainerNode node = cNodes[k];

											if (node.ItemDesc)
											{
												UItemDescEx* itemDesc = node.ItemDesc->CreateDefaultObject<UItemDescEx>();

												if (itemDesc)
												{
													if (!cfg->esp.islands.barrelspeek) break;
													char buf2[0x50];
													ZeroMemory(buf2, sizeof(buf2));



													std::string itemName2 = getShortName(itemDesc->GetName());


													int len = sprintf_s(buf2, sizeof(buf2), itemName2.c_str());
													sprintf_s(buf2 + len, sizeof(buf2) - len, " [%d]", node.NumItems);
													screen.Y += 20;
													ImVec4 color = itemName2 == "Chain" ? ImVec4(0.f, 1.f, 0.f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color1 = itemName2 == "Ballast" ? ImVec4(0.82f, 0.24f, 1.f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color2 = itemName2 == "Anchor" ? ImVec4(0.82f, 0.24f, 1.f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color3 = itemName2 == "Barrel" ? ImVec4(0.82f, 0.24f, 1.f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color4 = itemName2 == "Ringging" ? ImVec4(0.82f, 0.24f, 1.f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color5 = itemName2 == "Peace" ? ImVec4(0.82f, 0.24f, 1.f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color6 = itemName2 == "Helm" ? ImVec4(0.82f, 0.24f, 1.f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color7 = itemName2 == "Weary" ? ImVec4(0.16f, 1.f, 0.13f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color8 = itemName2 == "Grog" ? ImVec4(0.16f, 1.f, 0.13f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color9 = itemName2 == "Venom" ? ImVec4(0.16f, 1.f, 0.13f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color10 = itemName2 == "Jig" ? ImVec4(0.16f, 1.f, 0.13f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color11 = itemName2 == "Limp" ? ImVec4(0.16f, 1.f, 0.13f, 1.f) : cfg->esp.islands.barrelsColor;
													ImVec4 color12 = itemName2 == "Pine" ? ImVec4(0.98f, 0.67f, 0.f, 1.f) : cfg->esp.islands.barrelsColor;
													if (cfg->esp.islands.barrelstoggle)
													{
														if (GetAsyncKeyState(0x52))  
														{
															if (itemName2 == "Chain")
																RenderText(drawList, buf2, screen, { 0.f, 1.f, 0.f, 1.f }, 14);
															else if (itemName2 == "Ballast")
																RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
															else if (itemName2 == "Anchor")
																RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
															else if (itemName2 == "Barrel")
																RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
															else if (itemName2 == "Ringging")
																RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
															else if (itemName2 == "Peace")
																RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
															else if (itemName2 == "Helm")
																RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
															else if (itemName2 == "Weary")
																RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
															else if (itemName2 == "Grog")
																RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
															else if (itemName2 == "Venom")
																RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
															else if (itemName2 == "Jig")
																RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
															else if (itemName2 == "Limp")
																RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
															else if (itemName2 == "Pine")
																RenderText(drawList, buf2, screen, { 0.98f, 0.67f, 0.f, 1.f }, 14);
															else 
																RenderText(drawList, buf2, screen, cfg->esp.islands.barrelsColor, 14);
														}
													}
													else
													{
														if (itemName2 == "Chain")
															RenderText(drawList, buf2, screen, { 0.f, 1.f, 0.f, 1.f }, 14);
														else if (itemName2 == "Ballast")
															RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
														else if (itemName2 == "Anchor")
															RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
														else if (itemName2 == "Barrel")
															RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
														else if (itemName2 == "Ringging")
															RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
														else if (itemName2 == "Peace")
															RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
														else if (itemName2 == "Helm")
															RenderText(drawList, buf2, screen, { 0.82f, 0.24f, 1.f, 1.f }, 14);
														else if (itemName2 == "Weary")
															RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
														else if (itemName2 == "Grog")
															RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
														else if (itemName2 == "Venom")
															RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
														else if (itemName2 == "Jig")
															RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
														else if (itemName2 == "Limp")
															RenderText(drawList, buf2, screen, { 0.16f, 1.f, 0.13f, 1.f }, 14);
														else if (itemName2 == "Pine")
															RenderText(drawList, buf2, screen, { 0.98f, 0.67f, 0.f, 1.f }, 14);
														else
															RenderText(drawList, buf2, screen, cfg->esp.islands.barrelsColor, 14);
													}
												}
											}
										}
									}
									catch (...)
									{
										screen.Y -= 23;
										RenderText(drawList, "Invalid", screen, cfg->esp.islands.barrelsColor, 14);
									}
								}
							}
						}

						if (cfg->esp.islands.ammoChest && actor->isAmmoChest())
						{
							do
							{
								error_code = 23;
								const FVector location = actor->K2_GetActorLocation();
								FVector2D screen;
								if (actor->isAmmoChest())
								{
									const float dist = myLocation.DistTo(location) * 0.01f;
									if (dist > cfg->esp.islands.ammoChestRenderDistance) break;
									if (playerController->ProjectWorldLocationToScreen(location, screen))
									{
										char buf[0x64];
										sprintf_s(buf, "Ammo [%.0fm]", dist);
										RenderText(drawList, buf, screen, cfg->esp.islands.ammoChestColor, 14);
									}
								}
							} while (false);
						}
					}
					if (cfg->esp.items.enable && actor->isItem())
					{
						do
						{
							error_code = 24;
							auto location = actor->K2_GetActorLocation();
							const float dist = myLocation.DistTo(location) * 0.01f;
							FVector2D screen;
							if (actor->isItem())
							{
								if (dist > cfg->esp.items.renderDistance) break;

								if (playerController->ProjectWorldLocationToScreen(location, screen))
								{
									error_code = 2444;
									char buf[0x64];
									ZeroMemory(buf, sizeof(buf));

									auto const desc = actor->GetItemInfo()->Desc;
									if (!desc) break;
									const int len = desc->Title->multi(buf, 0x50);
									snprintf(buf + len, sizeof(buf) - len, " [%.0fm]", dist);
									if (cfg->esp.items.nameToggle)
									{
										if (GetAsyncKeyState(0x52))
											RenderText(drawList, buf, screen, cfg->esp.items.color, 14);
										else
											RenderText(drawList, buf, screen, cfg->esp.items.color, 30.f);
									}
									else
									{
										RenderText(drawList, buf, screen, cfg->esp.items.color, 14);
									}
									if (actor->compareName("BP_Medallion_"))
									{
										drawList->AddLine({ io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f }, { screen.X, screen.Y }, ImGui::ColorConvertFloat4ToU32(ImVec4(1.f, 0.f, 0.f, 1.f)), 1);
									}
								}
							}
							char buf[0x64];
							ZeroMemory(buf, sizeof(buf));
						} while (false);
					}
					if (cfg->esp.items.lostCargo && actor->IsA(ASeagulls2::StaticClass()) || cfg->esp.items.lostCargo && actor->isItem())
					{
						error_code = 251;
						auto location = actor->K2_GetActorLocation();
						const float dist = myLocation.DistTo(location) * 0.01f;
						FVector2D screen;
						char buf[0x64];
						ZeroMemory(buf, sizeof(buf));
						if (actor->compareName("BP_Seagull01_"))
						{
							if (playerController->ProjectWorldLocationToScreen(location, screen))
							{
								sprintf_s(buf, sizeof(buf), "Seagulls [%.0fm]", dist);
								RenderText(drawList, buf, screen, cfg->esp.items.cluesColor, 17);
							}
						}
						if (actor->compareName("BP_MessageInABottle_Clue_ItemInfo_C"))
						{
							if (playerController->ProjectWorldLocationToScreen(location, screen))
							{
								sprintf_s(buf, sizeof(buf), "Clue In A Bottle [%.0fm]", dist);
								RenderText(drawList, buf, screen, cfg->esp.items.cluesColor, 17);
							}
						}
						if (actor->compareName("BP_MA_CabinDoorKey_ItemInfo_C"))
						{
							if (playerController->ProjectWorldLocationToScreen(location, screen))
							{
								sprintf_s(buf, sizeof(buf), "Captains Key [%.0fm]", dist);
								RenderText(drawList, buf, screen, cfg->esp.items.cluesColor, 17);
							}
						}
					}
					if (cfg->esp.items.animals && actor->isAnimal())
					{
						do
						{
							error_code = 26;
							FVector origin, extent;
							actor->GetActorBounds(true, origin, extent);
							FVector2D headPos;
							if (!playerController->ProjectWorldLocationToScreen({ origin.X, origin.Y, origin.Z + extent.Z }, headPos)) break;
							FVector2D footPos;
							if (!playerController->ProjectWorldLocationToScreen({ origin.X, origin.Y, origin.Z - extent.Z }, footPos)) break;
							float height = abs(footPos.Y - headPos.Y);
							float width = height * 0.6f;
							const float dist = myLocation.DistTo(origin) * 0.01f;
							if (dist > cfg->esp.items.animalsRenderDistance) break;
							switch (cfg->esp.items.boxType)
							{
							case Config::Configuration::EBox::E2DBoxes:
							{
								Render2DBox(drawList, headPos, footPos, height, width, cfg->esp.items.animalsColor);
								break;
							}
							case Config::Configuration::EBox::E3DBoxes:
							{
								FRotator rotation = actor->K2_GetActorRotation();
								FVector ext = { 40.f, 40.f, extent.Z };
								if (!Render3DBox(playerController, origin, ext, rotation, cfg->esp.items.animalsColor)) continue;
								break;
							}
							}

							if (cfg->esp.items.bName)
							{
								FText displayNameText = reinterpret_cast<AFauna*>(actor)->DisplayName;
								FString displayNameString = FString(displayNameText.TextData->Text);
								if (displayNameString.IsValid()) {
									
									
									char buf[0x32];
									const int len = displayNameString.multi(buf, 0x50);
									if (dist >= 50)
									{
										snprintf(buf + len, sizeof(buf) - len, " [%.0fm]", dist);
									}
									const float adjust = height * 0.05f;
									FVector2D pos = { headPos.X, headPos.Y - adjust };
									RenderText(drawList, buf, pos, cfg->esp.items.animalsColor, 14);
								}
							}
						} while (false);
					}
					if (cfg->esp.items.sharks && actor->isShark())
					{
						do
						{
							error_code = 30;
							FVector origin, extent;
							actor->GetActorBounds(true, origin, extent);
							FVector2D headPos;
							if (!playerController->ProjectWorldLocationToScreen({ origin.X, origin.Y, origin.Z + extent.Z }, headPos)) break;
							FVector2D footPos;
							if (!playerController->ProjectWorldLocationToScreen({ origin.X, origin.Y, origin.Z - extent.Z }, footPos)) break;
							const float height = abs(footPos.Y - headPos.Y);
							const float width = height * 0.6f;
							char buf[0x20];
							const float dist = myLocation.DistTo(origin) * 0.01f;
							if (dist > cfg->esp.items.sharksRenderDistance) break;
							sprintf_s(buf, sizeof(buf), "Shark [%.0fm]", dist);
							const float adjust = height * 0.05f;
							FVector2D pos = { headPos.X, headPos.Y - adjust };
							RenderText(drawList, buf, pos, cfg->esp.items.sharksColor, 14);


							if (cfg->esp.items.bSkeleton)
							{
								auto const shark = reinterpret_cast<ASharkPawn*>(actor);
								auto const mesh = shark->Mesh;
								if (!reinterpret_cast<AUSkeletalMeshComponent*>(actor)) continue;
								const FMatrix comp2world = mesh->K2_GetComponentToWorld().ToMatrixWithScale();
								switch (shark->SwimmingCreatureType)
								{
								case ESwimmingCreatureType::Shark:
								{
									const BYTE bone1[] = { 17, 16, 5, 6, 7, 8, 9, 10, 11, 12 };
									const BYTE bone2[] = { 10, 13, 14 };
									const BYTE bone3[] = { 5, 18, 19 };
									const BYTE bone4[] = { 6, 15, 7 };
									const std::pair<const BYTE*, const BYTE> skeleton[] = { {bone1, 10}, {bone2, 3}, {bone3, 3}, {bone4, 3} };
									if (!RenderSkeleton(drawList, playerController, mesh, comp2world, skeleton, 4, cfg->esp.items.sharksColor)) continue;
									break;
								}
								case ESwimmingCreatureType::TinyShark:
								{
									const BYTE bone1[] = { 26, 25, 24, 23, 22, 21, 20, 19 };
									const BYTE bone2[] = { 28, 27, 24 };
									const BYTE bone3[] = { 33, 32, 31, 21, 34, 35, 36 };
									const std::pair<const BYTE*, const BYTE> skeleton[] = { {bone1, 8}, {bone2, 3}, {bone3, 7} };
									if (!RenderSkeleton(drawList, playerController, mesh, comp2world, skeleton, 3, cfg->esp.items.sharksColor)) continue;
									break;
								}
								}
							}



						} while (false);
					}
					if (cfg->esp.others.enable)
					{
						if (cfg->esp.others.shipwrecks && (actor->isShipwreck() || actor->compareName("BP_Shipwreck_01_a_NetProxy_C")))
						{
							do
							{
								error_code = 27;
								auto location = actor->K2_GetActorLocation();
								FVector2D screen;
								if (playerController->ProjectWorldLocationToScreen(location, screen))
								{
									const float dist = myLocation.DistTo(location) * 0.01f;
									if (dist > cfg->esp.others.shipwrecksRenderDistance) break;
									char buf[0x64];
									sprintf_s(buf, sizeof(buf), "Shipwreck [%.0fm]", dist);
									RenderText(drawList, buf, screen, cfg->esp.others.shipwrecksColor, 14);
								}
							} while (false);
						}
						if (cfg->esp.others.mermaids)
						{
							do
							{
								if (actor->isMermaid())
								{
									error_code = 28;
									FVector2D screen;
									const FVector location = actor->K2_GetActorLocation();
									const float dist = myLocation.DistTo(location) * 0.01f;
									if (dist > cfg->esp.others.mermaidsRenderDistance) break;
									{
										if (playerController->ProjectWorldLocationToScreen(location, screen))
										{
											char buf[0x16];
											sprintf_s(buf, sizeof(buf), "Mermaid [%.0fm]", dist);
											RenderText(drawList, buf, screen, cfg->esp.others.mermaidsColor, 15); 
										}

									}
								}
							} while (false);
						}
						if (cfg->esp.others.rowboats)
						{
							do
							{
								if (actor->isRowboat())
								{
									error_code = 29;
									const FVector location = actor->K2_GetActorLocation();
									const float dist = myLocation.DistTo(location) * 0.01f;
									if (dist > cfg->esp.others.rowboatsRenderDistance) break;
									FVector2D screen;
									if (playerController->ProjectWorldLocationToScreen(location, screen))
									{
										char buf[0x64];
										if (actor->compareName("Cannon"))
											sprintf_s(buf, sizeof(buf), "Cannon Rowboat [%.0fm]", dist);
										else if (actor->compareName("Harpoon"))
											sprintf_s(buf, sizeof(buf), "Harpoon Rowboat [%.0fm]", dist);
										else if (actor->compareName("Rowboat"))
											sprintf_s(buf, sizeof(buf), "Rowboat [%.0fm]", dist);
										RenderText(drawList, buf, screen, cfg->esp.others.rowboatsColor, 14);
									}
								}
							} while (false);
						}
						if (cfg->esp.others.events)
						{
							do
							{
								if (actor->isEvent())
								{
									error_code = 31;
									const FVector location = actor->K2_GetActorLocation();
									FVector2D screen;
									if (playerController->ProjectWorldLocationToScreen(location, screen))
									{
										auto type = actor->GetName();
										const float dist = myLocation.DistTo(location) * 0.01f;
										if (dist > cfg->esp.others.eventsRenderDistance) break;
										char buf[0x64];
										ZeroMemory(buf, sizeof(buf));
										if (actor->compareName("ShipCloud"))
											sprintf_s(buf, "Fleet [%.0fm]", dist);
										else if (actor->compareName("AshenLord"))
											sprintf_s(buf, "Ashen Lord [%.0fm]", dist);
										else if (actor->compareName("Flameheart"))
											sprintf_s(buf, "Flame Heart [%.0fm]", dist);
										else if (actor->compareName("LegendSkellyFort"))
											sprintf_s(buf, "Fort of Fortune [%.0fm]", dist);
										else if (actor->compareName("SkellyFortOfTheDamned"))
											sprintf_s(buf, "FOTD [%.0fm]", dist);
										else if (actor->compareName("BP_SkellyFort"))
											sprintf_s(buf, "Skull Fort [%.0fm]", dist);
										else if (actor->compareName("TornadoCloud"))
											sprintf_s(buf, "Tornado [%.0fm]", dist);
										RenderText(drawList, buf, screen, cfg->esp.others.eventsColor, 16);
									}
								}
							} while (false);
						}
					}
				}

				if (cfg->aim.enable)
				{



					if (attachObject && isHarpoon && cfg->aim.harpoon.enable)
					{
						if (actor->isItem())
						{
							do
							{
								FVector location = actor->K2_GetActorLocation();
								float dist = myLocation.DistTo(location);
								if (dist > 7500.f || dist < 260.f) { break; }
								if (cfg->aim.harpoon.visibleOnly) if (!playerController->LineOfSightTo(actor, cameraLocation, false)) { break; }
								auto harpoon = reinterpret_cast<AHarpoonLauncher*>(attachObject);
								auto center = UKismetMathLibrary::NormalizedDeltaRotator(cameraRotation, harpoon->rotation);
								FRotator delta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLocation, location), center);
								if (delta.Pitch < -35.f || delta.Pitch > 67.f || abs(delta.Yaw) > 50.f) { break; }
								FRotator diff = delta - harpoon->rotation;
								float absPitch = abs(diff.Pitch);
								float absYaw = abs(diff.Yaw);
								if (absPitch > 25.f || absYaw > 25.f) { break; }
								float sum = absYaw + absPitch;
								if (sum < aimBest.best)
								{
									aimBest.target = actor;
									aimBest.location = location;
									aimBest.delta = delta;
									aimBest.best = sum;
								}

							} while (false);
						}

					}


					if (attachObject && attachObject->isCannon() && cfg->aim.cannon.enable)
					{
						ACharacter* tmpCharacter = actor;
						error_code = 32;

						if (cfg->aim.cannon.chains && actor->isShip())
						{
							error_code = 33;
							do
							{
								if (actor == localPlayerActor->GetCurrentShip())
								{
									break;
								}
								FVector location = actor->K2_GetActorLocation();
								if (cfg->aim.cannon.visibleOnly && !playerController->LineOfSightTo(actor, cameraLocation, false))
								{
									break;
								}
								if (location.DistTo(cameraLocation) > 55000)
								{
									break;
								}
								int amount = 0;
								auto water = actor->GetInternalWater();
								amount = water->GetNormalizedWaterAmount() * 100.f;
								if (amount == 100)
									break;
								auto cannon = reinterpret_cast<ACannon*>(attachObject);
								float gravity_scale = cannon->ProjectileGravityScale;
								const FVector forward = actor->GetActorForwardVector();
								const FVector up = actor->GetActorUpVector();
								const FVector loc = actor->K2_GetActorLocation();
								FVector loc_mast = loc;
								loc_mast += forward * 80.f;
								loc_mast += up * 1300.f;
								location = loc_mast;
								gravity_scale = 1.f;
								FRotator low, high;
								if (cfg->aim.cannon.improvedVersion)
								{
									auto angularVelocity = actor->ReplicatedMovement.AngularVelocity;
									int i_solutions = AimAtShip(location, actor->GetVelocity(), angularVelocity, cameraLocation, attachObject->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, low, high);
									if (i_solutions < 1)
										break;
								}
								else
								{
									int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLocation, attachObject->GetVelocity(), low, high);
									if (i_solutions < 1)
										break;
								}
								low.Clamp();
								low -= attachObject->K2_GetActorRotation();
								low.Clamp();
								float absPitch = abs(low.Pitch);
								float absYaw = abs(low.Yaw);
								if (absPitch > cfg->aim.cannon.fPitch || absYaw > cfg->aim.cannon.fYaw) { break; }
								float sum = absYaw + absPitch;
								if (sum < aimBest.best)
								{
									aimBest.target = actor;
									aimBest.location = location;
									aimBest.delta = low;
									aimBest.best = sum;
								}
							} while (false);
						}
						if (cfg->aim.cannon.players && cfg->aim.cannon.chains == false && actor->isPlayer() && actor != localPlayerActor && !actor->IsDead())
						{
							error_code = 34;
							do
							{
								if (UCrewFunctions::AreCharactersInSameCrew(actor, localPlayerActor)) break;
								FVector location = actor->K2_GetActorLocation();
								if (location.DistTo(cameraLocation) > 55000)
								{
									break;
								}
								auto cannon = reinterpret_cast<ACannon*>(attachObject);
								float gravity_scale = cannon->ProjectileGravityScale;
								FRotator low, high;
								FVector acVelocity = actor->GetVelocity();
								FVector acFVelocity = actor->GetForwardVelocity();
								int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLocation, attachObject->GetVelocity(), low, high);
								if (i_solutions < 1)
									break;
								low.Clamp();
								low -= attachObject->K2_GetActorRotation();
								low.Clamp();
								float absPitch = abs(low.Pitch);
								float absYaw = abs(low.Yaw);
								if (absPitch > cfg->aim.cannon.fPitch || absYaw > cfg->aim.cannon.fYaw) { break; }
								float sum = absYaw + absPitch;
								if (sum < aimBest.best)
								{
									aimBest.target = actor;
									aimBest.location = location;
									aimBest.delta = low;
									aimBest.best = sum;
								}
							} while (false);
						}
						if (cfg->aim.cannon.skeletons && cfg->aim.cannon.chains == false && actor->isSkeleton() && actor != localPlayerActor && !actor->IsDead())
						{
							error_code = 35;
							do
							{
								FVector location = actor->K2_GetActorLocation();
								if (location.DistTo(cameraLocation) > 55000)
								{
									break;
								}
								auto cannon = reinterpret_cast<ACannon*>(attachObject);
								float gravity_scale = cannon->ProjectileGravityScale;
								FRotator low, high;
								int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLocation, attachObject->GetVelocity(), low, high);
								if (i_solutions < 1)
									break;
								low.Clamp();
								low -= attachObject->K2_GetActorRotation();
								low.Clamp();
								float absPitch = abs(low.Pitch);
								float absYaw = abs(low.Yaw);
								if (absPitch > cfg->aim.cannon.fPitch || absYaw > cfg->aim.cannon.fYaw) { break; }
								float sum = absYaw + absPitch;
								if (sum < aimBest.best)
								{
									aimBest.target = actor;
									aimBest.location = location;
									aimBest.delta = low;
									aimBest.best = sum;
								}

							} while (false);
						}
						if (cfg->aim.cannon.chains == false && actor->isShip())
						{
							error_code = 36;
							do
							{
								if (actor == localPlayerActor->GetCurrentShip())
								{
									break;
								}
								FVector location = actor->K2_GetActorLocation();
								if (cfg->aim.cannon.visibleOnly && !playerController->LineOfSightTo(actor, cameraLocation, false))
								{
									break;
								}
								if (location.DistTo(cameraLocation) > 55000)
								{
									break;
								}
								auto cannon = reinterpret_cast<ACannon*>(attachObject);
								int amount = 0;
								auto water = actor->GetInternalWater();
								amount = water->GetNormalizedWaterAmount() * 100.f;
								if (amount == 100)
									break;
								float gravity_scale = cannon->ProjectileGravityScale;
								if (cfg->aim.cannon.deckshots)
								{
									const FVector forward = actor->GetActorForwardVector();
									const FVector up = actor->GetActorUpVector();
									const FVector loc = actor->K2_GetActorLocation();
									FVector loc_mast = loc;
									loc_mast += forward * 52.f;
									loc_mast += up * 275.f;
									location = loc_mast;
									gravity_scale = 1.30f;
								}
								else if (cfg->aim.cannon.lowAim)
								{
									auto const damage = actor->GetHullDamage();
									if (damage)
									{
										FVector loc = pickHoleToAim(damage, myLocation);
										if (loc.Sum() != 9999.f)
											location = loc;
									}
								}
								FRotator low, high;

								if (cfg->aim.cannon.improvedVersion)
								{
									auto angularVelocity = actor->ReplicatedMovement.AngularVelocity;
									int i_solutions = AimAtShip(location, actor->GetVelocity(), angularVelocity, cameraLocation, attachObject->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, low, high);
									if (i_solutions < 1)
										break;
								}
								else
								{
									int i_solutions = AimAtMovingTarget(location, actor->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, cameraLocation, attachObject->GetVelocity(), low, high);
									if (i_solutions < 1)
										break;
								}


								low.Clamp();
								low -= attachObject->K2_GetActorRotation();
								low.Clamp();
								float absPitch = abs(low.Pitch);
								float absYaw = abs(low.Yaw);
								if (absPitch > cfg->aim.cannon.fPitch || absYaw > cfg->aim.cannon.fYaw) { break; }
								float sum = absYaw + absPitch;
								if (sum < aimBest.best)
								{
									aimBest.target = actor;
									aimBest.location = location;
									aimBest.delta = low;
									aimBest.best = sum;
								}
							} while (false);
						}
						if (cfg->aim.cannon.ghostShips && cfg->aim.cannon.chains == false && actor->isGhostShip())
						{
							error_code = 37;
							do
							{
								FVector location = actor->K2_GetActorLocation();
								auto ship = reinterpret_cast<AAggressiveGhostShip*>(actor);
								if (cfg->aim.cannon.visibleOnly && !playerController->LineOfSightTo(actor, cameraLocation, false))
								{
									break;
								}
								if (myLocation.DistTo(location) > 55000)
								{
									break;
								}
								if (actor == localPlayerActor->GetCurrentShip())
								{
									break;
								}
								if (ship->NumShotsLeftToKill < 1)
								{
									break;
								}
								auto cannon = reinterpret_cast<ACannon*>(attachObject);
								float gravity_scale = cannon->ProjectileGravityScale;
								auto forward = actor->GetActorForwardVector();
								forward *= ship->ShipState.ShipSpeed;
								FRotator low, high;
								if (cfg->aim.cannon.improvedVersion)
								{
									auto angularVelocity = actor->ReplicatedMovement.AngularVelocity;
									int i_solutions = AimAtShip(location, forward, angularVelocity, cameraLocation, attachObject->GetVelocity(), cannon->ProjectileSpeed, gravity_scale, low, high);
									if (i_solutions < 1)
										break;
								}
								else
								{
									int i_solutions = AimAtMovingTarget(location, forward, cannon->ProjectileSpeed, gravity_scale, cameraLocation, attachObject->GetVelocity(), low, high);
									if (i_solutions < 1)
										break;
								}

								low.Clamp();
								low -= attachObject->K2_GetActorRotation();
								low.Clamp();
								float absPitch = abs(low.Pitch);
								float absYaw = abs(low.Yaw);
								if (absPitch > cfg->aim.cannon.fPitch || absYaw > cfg->aim.cannon.fYaw) { break; }
								float sum = absYaw + absPitch;
								if (sum < aimBest.best)
								{
									aimBest.target = actor;
									aimBest.location = location;
									aimBest.delta = low;
									aimBest.best = sum;
								}
							} while (false);
						}
						actor = tmpCharacter;
					}
					else if (isWieldedWeapon && cfg->aim.weapon.enable)
					{
						do {
							error_code = 38;
							if (cfg->aim.weapon.players && actor->isPlayer() && actor != localPlayerActor && !actor->IsDead())
							{
								do
								{
									FVector playerLoc = actor->K2_GetActorLocation();
									if (!actor->IsInWater() && localWeapon->WeaponParameters.NumberOfProjectiles == 1)
									{
										playerLoc.Z += cfg->aim.weapon.height;
									}
									float dist = myLocation.DistTo(playerLoc);
									if (dist > localWeapon->WeaponParameters.ProjectileMaximumRange * 2.f) { break; }
									if (cfg->aim.weapon.visibleOnly) if (!playerController->LineOfSightTo(actor, cameraLocation, false)) { break; }
									if (UCrewFunctions::AreCharactersInSameCrew(actor, localPlayerActor)) break;
									FRotator rotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLocation, playerLoc), cameraRotation);
									float absYaw = abs(rotationDelta.Yaw);
									float absPitch = abs(rotationDelta.Pitch);
									if (absYaw > cfg->aim.weapon.fYaw || absPitch > cfg->aim.weapon.fPitch) { break; }
									float sum = absYaw + absPitch;
									if (sum < aimBest.best)
									{
										aimBest.target = actor;
										aimBest.location = playerLoc;
										aimBest.delta = rotationDelta;
										aimBest.best = sum;
										aimBest.smoothness = cfg->aim.weapon.smooth;
										engine::aimTarget = actor;
									}

								} while (false);
							}
							if (cfg->aim.weapon.kegs)
							{
								error_code = 39;
								if (actor->compareName("BP_MerchantCrate_GunpowderBarrel_"))
								{
									do
									{
										const FVector playerLoc = actor->K2_GetActorLocation();
										const float dist = myLocation.DistTo(playerLoc);
										if (dist > localWeapon->WeaponParameters.ProjectileMaximumRange * 2.f) break;
										if (cfg->aim.weapon.visibleOnly) if (!playerController->LineOfSightTo(actor, cameraLocation, false)) break;
										const FRotator rotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLocation, playerLoc), cameraRotation);
										const float absYaw = abs(rotationDelta.Yaw);
										const float absPitch = abs(rotationDelta.Pitch);
										if (absYaw > cfg->aim.weapon.fYaw || absPitch > cfg->aim.weapon.fPitch) break;
										const float sum = absYaw + absPitch;
										if (sum < aimBest.best)
										{
											aimBest.target = actor;
											aimBest.location = playerLoc;
											aimBest.delta = rotationDelta;
											aimBest.best = sum;
											aimBest.smoothness = cfg->aim.weapon.smooth;
										}

									} while (false);
								}
							}
							if (cfg->aim.weapon.skeletons && actor->isSkeleton() && !actor->IsDead())
							{
								error_code = 40;
								do
								{
									FVector playerLoc = actor->K2_GetActorLocation();
									if (localWeapon->WeaponParameters.NumberOfProjectiles == 1)
										playerLoc.Z += cfg->aim.weapon.height;
									const float dist = myLocation.DistTo(playerLoc);
									if (dist > localWeapon->WeaponParameters.ProjectileMaximumRange * 2.f) break;
									if (cfg->aim.weapon.visibleOnly) if (!playerController->LineOfSightTo(actor, cameraLocation, false)) break;
									const FRotator rotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLocation, playerLoc), cameraRotation);
									const float absYaw = abs(rotationDelta.Yaw);
									const float absPitch = abs(rotationDelta.Pitch);
									if (absYaw > cfg->aim.weapon.fYaw || absPitch > cfg->aim.weapon.fPitch) break;
									const float sum = absYaw + absPitch;
									if (sum < aimBest.best)
									{
										aimBest.target = actor;
										aimBest.location = playerLoc;
										aimBest.delta = rotationDelta;
										aimBest.best = sum;
										aimBest.smoothness = cfg->aim.weapon.smooth;
										engine::aimTarget = actor;
									}
								} while (false);
							}
						} while (false);


						if (cfg->aim.weapon.fastweapon && localWeapon && isWieldedWeapon)
						{

							if (localWeapon->WeaponParameters.RecoilDuration != -1.0f)
							{
								localWeapon->WeaponParameters.RecoilDuration = -1.0f;
							}
							if (localWeapon->WeaponParameters.EquipDuration != -1.0f)
							{
								localWeapon->WeaponParameters.EquipDuration = -1.0f;
							}
						}
					}
				}

				if (cfg->game.enable)
				{
					if (cfg->game.mapPins)
					{
						if (actor->isMapTable() && localPlayerActor->GetCurrentShip() == actor->GetParentActor())
						{
							auto maptable = reinterpret_cast<AMapTable*>(actor);
							auto map_pins = maptable->MapPins;

							char buf[0x64];
							FVector2D screen;

							for (int i = 0; i < map_pins.Count; i++)
							{
								if (i >= map_pins.Count) break; 

								FVector2D current_map_pin = map_pins[i] * 100.f;
								FVector current_map_pin_world(current_map_pin.X, current_map_pin.Y, 0.f);

								if (playerController->ProjectWorldLocationToScreen(current_map_pin_world, screen))
								{
									const float dist = myLocation.DistTo(current_map_pin_world) * 0.01f;
									sprintf_s(buf, sizeof(buf), ICON_FA_MAP " Pin [%.0fm]", dist);
									RenderText(drawList, buf, screen, { 1.f, 1.f, 1.f, 1.f }, 14);
								}
							}
						}
					}
					if (cfg->game.cooking)
					{
						error_code = 421;
						if (actor->isCookingPot())
						{
							cookingPots.push_back(actor);
						}
						auto type = actor->GetName();
						if (actor->compareName("BP_fod_"))
						{
							bool isInPot = false;
							const FVector location = actor->K2_GetActorLocation();
							const float dist = myLocation.DistTo(location) * 0.01f;
							FVector2D screen;

							// check if actor is in any cooking pot
							for (int i = 0; i < cookingPots.size(); i++)
							{
								if (location.DistTo(cookingPots[i]->K2_GetActorLocation()) < 1.f)
								{
									isInPot = true;
									break;  // no need to continue checking if already in a pot
								}
							}

							// render text if actor is in a pot and within distance range
							if (isInPot && dist <= 150.f && dist >= 1.f)
							{
								if (playerController->ProjectWorldLocationToScreen(location, screen))
								{
									char buf[0x64];
									ZeroMemory(buf, sizeof(buf));
									if (type.find("Raw") != std::string::npos) {
										sprintf_s(buf, sizeof(buf), "Cooking: Raw");
										RenderText(drawList, buf, screen, { 1.f,1.f,1.f,1.f }, 14);
									}
									else if (type.find("Undercooked") != std::string::npos)
									{
										sprintf_s(buf, sizeof(buf), "Cooking: UnderCooked");
										RenderText(drawList, buf, screen, { 0.75f, 0.75f, 0.f, 1.f }, 14);
									}
									else if (type.find("Cooked") != std::string::npos)
									{
										sprintf_s(buf, sizeof(buf), "Cooking: Cooked");
										RenderText(drawList, buf, screen, { 0.f, 1.f, 0.f, 1.f }, 14);
									}
									else if (type.find("Burned") != std::string::npos)
									{
										sprintf_s(buf, sizeof(buf), "Cooking: Burned");
										RenderText(drawList, buf, screen, { 1.f, 0.f, 0.f, 1.f }, 14);
									}
									else if (type.find("Burned") != std::string::npos) 
									{
										sprintf_s(buf, sizeof(buf), "Cooking: Burned");
										RenderText(drawList, buf, screen, { 1.f, 0.f, 0.f, 1.f }, 14);
									}
									else
									{
										sprintf_s(buf, sizeof(buf), "Raw");
										RenderText(drawList, buf, screen, { 1.f, 1.f, 1.f, 1.f }, 14);
									}
								}
							}
						}
					}
					if (cfg->game.shipInfo && actor->isShip())
					{
						error_code = 10;
						auto ship = localPlayerActor->GetCurrentShip();
						if (ship)
						{
							auto tangential_velocity = ship->ReplicatedMovement.LinearVelocity;
							tangential_velocity.Z = 0;
							const float velocity = (FVector(tangential_velocity.X, tangential_velocity.Y, 0) * 0.01f).Size();
							char buf[0xFF];
							FVector2D pos{ 10.f, 45.f };
							ImVec4 col{ 1.f, 1.f, 1.f, 1.f };
							sprintf_s(buf, "Speed: %.0fm/s", velocity);
							pos.Y += 5.f;
							RenderText(drawList, buf, pos, col, 14, false);
							int holes = ship->GetHullDamage()->ActiveHullDamageZones.Count;
							sprintf_s(buf, "Holes: %d", holes);
							pos.Y += 15.f;
							RenderText(drawList, buf, pos, col, 14, false);
							int amount = 0;
							auto water = ship->GetInternalWater();
							if (water) amount = water->GetNormalizedWaterAmount() * 100.f;
							sprintf_s(buf, "Water: %d%%", amount);
							pos.Y += 12.f;
							RenderText(drawList, buf, pos, col, 14, false);
							pos.Y += 19.f;
							float internal_water_percent = ship->GetInternalWater()->GetNormalizedWaterAmount();
							drawList->AddLine({ pos.X - 1, pos.Y }, { pos.X + 100 + 1, pos.Y }, 0xFF000000, 5);
							drawList->AddLine({ pos.X, pos.Y }, { pos.X + 100, pos.Y }, 0xFF00FF00, 3);
							drawList->AddLine({ pos.X, pos.Y }, { pos.X + (100.f * internal_water_percent), pos.Y }, 0xFF0000FF, 3);
						}
					}
					if (cfg->game.showSunk)
					{
						do
						{
							if (!actor->isShip() && !actor->isFarShip()) break;

							const FVector location = actor->K2_GetActorLocation();
							if (location.Z < -1500.f)
							{
								bool duplicated = false;
								for (int i = 0; i < trackedSinkLocs.size(); i++)
								{
									if ((trackedSinkLocs[i].DistTo(location) * 0.01f) < 50.f)
									{
										duplicated = true;
										break;
									}
								}
								if (duplicated) break;

								trackedSinkLocs.push_back(location);
							}
						} while (false);
					}

				}
			}
		}
		if (aimBest.target != nullptr || (engine::aimTarget && cfg->aim.others.rage))
		{
			error_code = 43;

			FVector2D screen;
			if (playerController->ProjectWorldLocationToScreen(aimBest.location, screen))
			{
				auto col = ImGui::GetColorU32(IM_COL32(0, 200, 0, 255));
				drawList->AddCircle({ screen.X, screen.Y }, 6.f, col, 0, 2);
				drawList->AddLine({ screen.X, screen.Y - 6.f }, { screen.X, screen.Y + 6.f }, col, 2);
				drawList->AddLine({ screen.X - 6.f, screen.Y }, { screen.X + 6.f, screen.Y }, col, 2);
			}
			static std::uintptr_t shotDesiredTime = 0;

			if (GetAsyncKeyState(VK_RBUTTON))
			{

				if (attachObject && attachObject->isCannon())
				{
					error_code = 44;
					auto cannon = reinterpret_cast<ACannon*>(attachObject);
					if (cannon)
					{
						if (((aimBest.delta.Pitch > cannon->PitchRange.max) || (aimBest.delta.Pitch < cannon->PitchRange.min)) || ((aimBest.delta.Yaw > cannon->YawRange.max) || (aimBest.delta.Yaw < cannon->YawRange.min)))
						{
							std::string str_text_message = "TARGET IS OUT OF RANGE!";
							drawList->AddText({ io.DisplaySize.x * 0.5f , io.DisplaySize.y * 0.5f + 30.f }, 0xFFFFFFFF, str_text_message.c_str());
						}
						else
						{
							cannon->ForceAimCannon(aimBest.delta.Pitch, aimBest.delta.Yaw);
							if (cfg->aim.cannon.instant && cannon->IsReadyToFire())
							{
								cannon->Fire();
							}
						}
					}
				}
				else if (isHarpoon)
				{
					reinterpret_cast<AHarpoonLauncher*>(attachObject)->rotation = aimBest.delta;
				}
				else
				{
					error_code = 45;
					FVector LV;
					FVector TV;

					if (cfg->aim.weapon.calcShipVel)
					{
						try
						{
							LV = { 0.f,0.f,0.f };
							if (auto const localShip = localPlayerActor->GetCurrentShip()) LV += localShip->GetVelocity();
							TV = aimBest.target->GetVelocity();
							if (auto const targetShip = aimBest.target->GetCurrentShip()) TV += targetShip->GetVelocity();
						}
						catch (...)
						{
							LV = { 0.f,0.f,0.f };
							TV = { 0.f,0.f,0.f };
						}
					}
					else
					{
						LV = { 0.f,0.f,0.f };
						TV = { 0.f,0.f,0.f };
					}

					FVector RV = TV - LV;
					float BS;
					if (localWeapon)
						BS = localWeapon->WeaponParameters.AmmoParams.Velocity;
					else
						BS = 1.f;
					const FVector RL = myLocation - aimBest.location;
					const float a = RV.Size() - BS * BS;
					const float b = (RL * RV * 2.f).Sum();
					const float c = RL.SizeSquared();
					const float D = b * b - 4 * a * c;
					if (D > 0)
					{
						const float DRoot = sqrtf(D);
						const float x1 = (-b + DRoot) / (2 * a);
						const float x2 = (-b - DRoot) / (2 * a);
						if (x1 >= 0 && x1 >= x2) aimBest.location += RV * x1;
						else if (x2 >= 0) aimBest.location += RV * x2;

						aimBest.delta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLocation, aimBest.location), cameraRotation);
						auto smoothness = 1.f / aimBest.smoothness;
						if (abs(aimBest.delta.Yaw) > 0.005f || abs(aimBest.delta.Pitch) > 0.005f)
						{

							playerController->AddYawInput(aimBest.delta.Yaw * smoothness);
							playerController->AddPitchInput(aimBest.delta.Pitch * -smoothness);
						}
					}
					static byte shotFixC = 0;
					static bool shotFixing = false;

					if (cfg->aim.weapon.trigger && isWieldedWeapon && localWeapon->CanFire())
					{
						error_code = 46;
						do {
							if (!cfg->aim.others.rage)
							{
								if (shotFixC != 1 && !shotFixing)
								{
									shotFixC++;
									break;
								}
								if (shotDesiredTime == 0)
								{
									shotDesiredTime = milliseconds_now() + 210;
									shotFixing = true;
								}
								if (milliseconds_now() >= shotDesiredTime)
								{
									shotDesiredTime = 0;
									shotFixing = false;
								}
								else
								{
									break;
								}
							}


							if (!playerController->LineOfSightTo(aimBest.target, cameraLocation, false) && !cfg->aim.others.rage) break;


							if (cfg->aim.others.rage && engine::aimTarget != nullptr && cfg->dev.interceptProcessEvent)
							{
								if (!playerController->LineOfSightTo(engine::aimTarget, cameraLocation, false)) break;
								auto deltaRage = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLocation, engine::aimTarget->K2_GetActorLocation()), cameraRotation);

								playerController->AddYawInput(deltaRage.Yaw);
								playerController->AddPitchInput(deltaRage.Pitch);

								reinterpret_cast<ATestProjectileWeapon*>(localWeapon)->FireInstantly();

								INPUT inputs[4] = {};
								ZeroMemory(inputs, sizeof(inputs));

								inputs[0].type = INPUT_KEYBOARD;
								inputs[0].ki.wVk = 0x58;

								inputs[1].type = INPUT_KEYBOARD;
								inputs[1].ki.wVk = 0x58;
								inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

								inputs[2].type = INPUT_KEYBOARD;
								inputs[2].ki.wVk = 0x58;

								inputs[3].type = INPUT_KEYBOARD;
								inputs[3].ki.wVk = 0x58;
								inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

								SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
							}
							else
							{
								INPUT inputs[6] = {};
								ZeroMemory(inputs, sizeof(inputs));
								inputs[0].type = INPUT_MOUSE;
								inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

								inputs[1].type = INPUT_MOUSE;
								inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

								inputs[2].type = INPUT_KEYBOARD;
								inputs[2].ki.wVk = 0x58;

								inputs[3].type = INPUT_KEYBOARD;
								inputs[3].ki.wVk = 0x58;
								inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

								inputs[4].type = INPUT_KEYBOARD;
								inputs[4].ki.wVk = 0x58;

								inputs[5].type = INPUT_KEYBOARD;
								inputs[5].ki.wVk = 0x58;
								inputs[5].ki.dwFlags = KEYEVENTF_KEYUP;

								SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
							}

							shotFixC = 0;
						} while (false);
					}
				}
			}
			else
			{
				shotDesiredTime = 0;
			}
		}
	}
	catch (...) {
		if (cfg->dev.printErrorCodes)
			tslog::debug("Exception in Render Thread. Error Code: %d.", error_code);
	}
}
bool initUE4(uintptr_t world, uintptr_t objects, uintptr_t names)
{
	try
	{
		UWorld::GWorld = reinterpret_cast<decltype(UWorld::GWorld)>(world);
		UObject::GObjects = reinterpret_cast<decltype(UObject::GObjects)>(objects);
		FName::GNames = *reinterpret_cast<decltype(FName::GNames)*>(names);
		AthenaGameViewportClient = UObject::FindObject<UAthenaGameViewportClient>("AthenaGameViewportClient Transient.AthenaGameEngine_1.AthenaGameViewportClient_1");

		if (!checkSDKObjects())
			return false;
	}
	catch (...)
	{
		return false;
	}
	return true;
}

bool setGameVars()
{
	localPlayer = AthenaGameViewportClient->GameInstance->LocalPlayers[0];
	playerController = localPlayer->PlayerController;
	localPlayerActor = (AAthenaPlayerCharacter*)playerController->K2_GetPawn();
	return true;
}

bool checkGameVars()
{
	if (!AthenaGameViewportClient) return false;
	if (!localPlayer) return false;
	if (!playerController) return false;
	if (!localPlayerActor) return false;

	auto world = *UWorld::GWorld;
	if (!world) return false;
	auto const game = world->GameInstance;
	if (!game) return false;
	auto const localPlayer = game->LocalPlayers[0];
	if (!localPlayer) return false;
	auto const localController = localPlayer->PlayerController;
	if (!localController) return false;
	auto const localCharacter = localController->Character;
	if (!localCharacter) return false;
	if (localCharacter->IsLoading()) return false;

	return true;
}

bool updateGameVars()
{
	if (!AthenaGameViewportClient) return false;
	localPlayer = AthenaGameViewportClient->GameInstance->LocalPlayers[0];
	if (!localPlayer) return false;
	playerController = localPlayer->PlayerController;
	if (!playerController) return false;
	localPlayerActor = (AAthenaPlayerCharacter*)playerController->K2_GetPawn();
	if (!localPlayerActor) return false;
	if (!checkGameVars()) return false;
	return true;
}

void RenderText(ImDrawList* drawList, const char* text, const FVector2D& pos, const ImVec4& color, const float dist, const bool outlined, const bool centered)
{
	if (!text) return;
	auto ImScreen = *reinterpret_cast<const ImVec2*>(&pos);
	if (dist <= cfg->dev.pinRenderDistanceMax && dist >= cfg->dev.pinRenderDistanceMin)
	{
		float pinSize = dist * -0.2 + 10;
		pinSize = fClamp(pinSize, 0.5f, 5.f);
		renderPin(drawList, ImScreen, color, pinSize);
	}
	if (centered)
	{
		auto size = ImGui::CalcTextSize(text);
		ImScreen.x -= size.x * 0.5f;
	}
	if (dist <= cfg->dev.nameTextRenderDistanceMax && dist >= cfg->dev.nameTextRenderDistanceMin)
	{
		float fontSize = -(std::sqrt(dist + 750.f)) + 50;
		fontSize = fClamp(fontSize, 10.f, 25.f);
		fontSize *= cfg->dev.renderTextSizeFactor;
		RenderText(drawList, text, ImScreen, color, fontSize);
	}

}

void RenderText(ImDrawList* drawList, const char* text, const FVector2D& pos, const ImVec4& color, const bool outlined, const bool centered)
{
	if (!text) return;
	auto ImScreen = *reinterpret_cast<const ImVec2*>(&pos);
	if (centered)
	{
		auto size = ImGui::CalcTextSize(text);
		ImScreen.x -= size.x * 0.5f;
		ImScreen.y += size.y;
	}
	drawList->AddText(nullptr, 20.f * cfg->dev.renderTextSizeFactor, ImScreen, ImGui::GetColorU32(color), text);
}

void RenderText(ImDrawList* drawList, const char* text, const FVector2D& pos, const ImVec4& color, const int fontSize, const bool centered)
{
    if (!text) return;
    auto ImScreen = *reinterpret_cast<const ImVec2*>(&pos);

    ImVec2 size(0, 0);
    if (centered)
    {
        size = ImGui::CalcTextSize(text);
        ImScreen.x -= size.x * 0.5f;
    }

    float fSize = fontSize * cfg->dev.renderTextSizeFactor;

    // combine the three AddText calls into one
    drawList->AddText(nullptr, fSize, ImVec2(ImScreen.x - 1.f, ImScreen.y - 1.f), ImGui::GetColorU32(IM_COL32_BLACK), text);
    drawList->AddText(nullptr, fSize, ImVec2(ImScreen.x + 1.f, ImScreen.y + 1.f), ImGui::GetColorU32(IM_COL32_BLACK), text);
    drawList->AddText(nullptr, fSize, ImScreen, ImGui::GetColorU32(color), text);
}

bool RenderSkeleton(ImDrawList* drawList, AController* const controller, USkeletalMeshComponent* const mesh, const FMatrix& comp2world, const std::pair<const BYTE*, const BYTE>* skeleton, int size, const ImVec4& color)
{
	for (auto s = 0; s < size; s++)
	{
		auto& bone = skeleton[s];
		FVector2D previousBone;
		for (auto i = 0; i < skeleton[s].second; i++)
		{
			FVector loc;
			int res = mesh->GetBone(bone.first[i], comp2world, loc);
			if (res == 0)
				return false;
			FVector2D screen;
			if (!controller->ProjectWorldLocationToScreen(loc, screen)) return false;
			if (previousBone.Size() != 0) {
				auto ImScreen1 = *reinterpret_cast<ImVec2*>(&previousBone);
				auto ImScreen2 = *reinterpret_cast<ImVec2*>(&screen);
				ImGui::GetCurrentWindow()->DrawList->AddLine(ImScreen1, ImScreen2, ImGui::GetColorU32(color), 2.f);
			}
			previousBone = screen;
		}
	}
	return true;
}

void RenderText(ImDrawList* drawList, const char* text, const ImVec2& screen, const ImVec4& color, const float size, const bool outlined, const bool centered)
{
	auto window = ImGui::GetCurrentWindow();
	float fSize = size * cfg->dev.renderTextSizeFactor;
	window->DrawList->AddText(nullptr, fSize, ImVec2(screen.x - 1.f, screen.y - 1.f), ImGui::GetColorU32(IM_COL32_BLACK), text);
	window->DrawList->AddText(nullptr, fSize, ImVec2(screen.x + 1.f, screen.y + 1.f), ImGui::GetColorU32(IM_COL32_BLACK), text);
	window->DrawList->AddText(nullptr, fSize, screen, ImGui::GetColorU32(color), text);

}

void renderPin(ImDrawList* drawList, const ImVec2& ImScreen, const ImVec4& color, const float radius)
{
	uint32_t colorU32 = ImGui::GetColorU32(color);
	drawList->AddCircle(ImVec2(ImScreen.x, ImScreen.y), radius, colorU32, 16, 2.f); // number of segments 
}

void Render2DBox(ImDrawList* drawList, const FVector2D& top, const FVector2D& bottom, const float height, const float width, const ImVec4& color)
{
	uint32_t colorU32 = ImGui::GetColorU32(color);
	drawList->AddRect({ top.X - width * 0.5f, top.Y }, { top.X + width * 0.5f, bottom.Y }, colorU32, 0.f, 15, 1.5f);
}

bool Render3DBox(AController* const controller, const FVector& origin, const FVector& extent, const FRotator& rotation, const ImVec4& color)
{
	FVector vertex[2][4];
	vertex[0][0] = { -extent.X, -extent.Y,  -extent.Z };
	vertex[0][1] = { extent.X, -extent.Y,  -extent.Z };
	vertex[0][2] = { extent.X, extent.Y,  -extent.Z };
	vertex[0][3] = { -extent.X, extent.Y, -extent.Z };

	vertex[1][0] = { -extent.X, -extent.Y, extent.Z };
	vertex[1][1] = { extent.X, -extent.Y, extent.Z };
	vertex[1][2] = { extent.X, extent.Y, extent.Z };
	vertex[1][3] = { -extent.X, extent.Y, extent.Z };

	FVector2D screen[2][4];
	FTransform const Transform(rotation);
	for (auto k = 0; k < 2; k++)
	{
		for (auto i = 0; i < 4; i++)
		{
			auto& vec = vertex[k][i];
			vec = Transform.TransformPosition(vec) + origin;
			if (!controller->ProjectWorldLocationToScreen(vec, screen[k][i])) return false;
		}

	}

	auto ImScreen = reinterpret_cast<ImVec2(&)[2][4]>(screen);

	auto window = ImGui::GetCurrentWindow();
	for (auto i = 0; i < 4; i++)
	{
		window->DrawList->AddLine(ImScreen[0][i], ImScreen[0][(i + 1) % 4], ImGui::GetColorU32(color));
		window->DrawList->AddLine(ImScreen[1][i], ImScreen[1][(i + 1) % 4], ImGui::GetColorU32(color));
		window->DrawList->AddLine(ImScreen[0][i], ImScreen[1][i], ImGui::GetColorU32(color));
	}

	return true;
}

float fClamp(float v, const float min, const float max)
{
	if (v < min) v = min;
	if (v > max) v = max;
	return v;
}


#define Assert( _exp ) ((void)0)

struct vMatrix
{
	vMatrix() {}
	vMatrix(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23)
	{
		m_flMatVal[0][0] = m00;	m_flMatVal[0][1] = m01; m_flMatVal[0][2] = m02; m_flMatVal[0][3] = m03;
		m_flMatVal[1][0] = m10;	m_flMatVal[1][1] = m11; m_flMatVal[1][2] = m12; m_flMatVal[1][3] = m13;
		m_flMatVal[2][0] = m20;	m_flMatVal[2][1] = m21; m_flMatVal[2][2] = m22; m_flMatVal[2][3] = m23;
	}

	float* operator[](int i) { Assert((i >= 0) && (i < 3)); return m_flMatVal[i]; }
	const float* operator[](int i) const { Assert((i >= 0) && (i < 3)); return m_flMatVal[i]; }
	float* Base() { return &m_flMatVal[0][0]; }
	const float* Base() const { return &m_flMatVal[0][0]; }

	float m_flMatVal[3][4];
};

vMatrix Matrix(Vector3 rot, Vector3 origin)
{
	origin = Vector3(0, 0, 0);
	float radPitch = (rot.x * float(PI) / 180.f);
	float radYaw = (rot.y * float(PI) / 180.f);
	float radRoll = (rot.z * float(PI) / 180.f);

	float SP = sinf(radPitch);
	float CP = cosf(radPitch);
	float SY = sinf(radYaw);
	float CY = cosf(radYaw);
	float SR = sinf(radRoll);
	float CR = cosf(radRoll);

	vMatrix matrix;
	matrix[0][0] = CP * CY;
	matrix[0][1] = CP * SY;
	matrix[0][2] = SP;
	matrix[0][3] = 0.f;

	matrix[1][0] = SR * SP * CY - CR * SY;
	matrix[1][1] = SR * SP * SY + CR * CY;
	matrix[1][2] = -SR * CP;
	matrix[1][3] = 0.f;

	matrix[2][0] = -(CR * SP * CY + SR * SY);
	matrix[2][1] = CY * SR - CR * SP * SY;
	matrix[2][2] = CR * CP;
	matrix[2][3] = 0.f;



	return matrix;
}

bool WorldToScreen(Vector3 origin, Vector2* out, const FVector& cameraLocation, const FRotator& cameraRotation, const float fov) {
	Vector3 Screenlocation = Vector3(0, 0, 0);
	Vector3 Rotation = Vector3(cameraRotation.Pitch, cameraRotation.Yaw, cameraRotation.Roll);
	Vector3 Location = Vector3(cameraLocation.X, cameraLocation.Y, cameraLocation.Z);

	vMatrix tempMatrix = Matrix(Rotation, Vector3(0, 0, 0));

	Vector3 vAxisX, vAxisY, vAxisZ;

	vAxisX = Vector3(tempMatrix[0][0], tempMatrix[0][1], tempMatrix[0][2]);
	vAxisY = Vector3(tempMatrix[1][0], tempMatrix[1][1], tempMatrix[1][2]);
	vAxisZ = Vector3(tempMatrix[2][0], tempMatrix[2][1], tempMatrix[2][2]);

	Vector3 vDelta = origin - Location;
	Vector3 vTransformed = Vector3(vDelta.Dot(vAxisY), vDelta.Dot(vAxisZ), vDelta.Dot(vAxisX));

	if (vTransformed.z < 1.f)
		vTransformed.z = 1.f;

	auto& io = ImGui::GetIO();
	float FovAngle = fov;
	float ScreenCenterX = io.DisplaySize.x * 0.5f;
	float ScreenCenterY = io.DisplaySize.y * 0.5f;

	out->x = ScreenCenterX + vTransformed.x * (ScreenCenterX / tanf(FovAngle * static_cast<float>(PI) / 360.f)) / vTransformed.z;
	out->y = ScreenCenterY - vTransformed.y * (ScreenCenterX / tanf(FovAngle * static_cast<float>(PI) / 360.f)) / vTransformed.z;


	return true;
}



bool checkSDKObjects()
{
	if (!UCrewFunctions::Init()) return false;
	tslog::verbose("UCrewFunctions - Ready.");
	if (!UKismetMathLibrary::Init()) return false;
	tslog::verbose("UKismetMathLibrary - Ready.");
	if (!UKismetSystemLibrary::Init()) return false;
	tslog::verbose("UKismetSystemLibrary - Ready.");
	return true;
}

uintptr_t milliseconds_now() {
	static LARGE_INTEGER s_frequency;
	static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
	if (s_use_qpc) {
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return (1000LL * now.QuadPart) / s_frequency.QuadPart;
	}
	else {
		return GetTickCount64();
	}
}


Vector2 RotatePoint(Vector2 pointToRotate, Vector2 centerPoint, float angle, bool angleInRadians)
{
	if (!angleInRadians)
		angle = static_cast<float>(angle * (PI / 180.f));
	float cosTheta = static_cast<float>(cos(angle));
	float sinTheta = static_cast<float>(sin(angle));
	Vector2 returnVec = Vector2(cosTheta * (pointToRotate.x - centerPoint.x) - sinTheta * (pointToRotate.y - centerPoint.y), sinTheta * (pointToRotate.x - centerPoint.x) + cosTheta * (pointToRotate.y - centerPoint.y)
	);
	returnVec += centerPoint;
	return returnVec;
}

bool raytrace(UWorld* world, const struct FVector& start, const struct FVector& end, struct FHitResult* hit)
{
	if (world == nullptr || world->PersistentLevel == nullptr)
		return false;
	return UKismetMathLibrary::LineTraceSingle_NEW((UObject*)world, start, end, ETraceTypeQuery::TraceTypeQuery3, true, TArray<AActor*>(), EDrawDebugTrace::EDrawDebugTrace__None, true, hit);
}


FVector pickHoleToAim(AHullDamage* damage, const FVector& localLoc)
{
	FVector fLocation = { 0.f, 0.f, 9999.f };
	FVector location = FVector();
	float currentDist = 55000.f;
	const auto holes = damage->DamageZones;
	for (auto h = 0u; h < holes.Count; h++)
	{
		auto const hole = holes[h];
		if (hole->DamageLevel < 3)
		{
			location = FVector(reinterpret_cast<ACharacter*>(hole)->K2_GetActorLocation());
			float dist = localLoc.DistTo(location);
			if (dist <= currentDist)
			{
				currentDist = dist;
				fLocation = location;
			}
		}
	}
	return fLocation;
}


void processhk(void* Object, UFunction* Function, void* Params)
{
	if (FunctionIndex::BoxedRpcDispatcherComponent == -1)
		if (Function->GetFullName().compare("Function AthenaEngine.BoxedRpcDispatcherComponent.Server_SendRpc") == 0)
			FunctionIndex::BoxedRpcDispatcherComponent = Function->InternalIndex;

	if (Function->InternalIndex == FunctionIndex::BoxedRpcDispatcherComponent)
	{
		auto result = (FSerialisedRpc*)Params;
		if (cfg->dev.printRPCCalls)
		{
			char buf[0x64];
			ZeroMemory(buf, sizeof(buf));
			int len = sprintf_s(buf, sizeof(buf), "Name: %s", result->ContentsType->GetFullName().c_str());
			if (len > 6)
				tslog::debug(buf);
		}

		if (result->ContentsType->GetFullName().find("ScriptStruct Athena.PrepareInstantFireRpc") != std::string::npos) {
			tslog::debug("Intercepting PrepareInstantFireRpc");
			return;
		}
	}
	return oProcessEvent(Object, Function, Params);
}

void hookProcessEvent()
{
	engine::oProcessEvent = nullptr;

	if (AthenaGameViewportClient)
	{
		DWORD64** ProcessEvent = *(DWORD64***)(AthenaGameViewportClient)+55;

		if (MH_CreateHook(reinterpret_cast<LPVOID>(*ProcessEvent), processhk, reinterpret_cast<LPVOID*>(&engine::oProcessEvent)) != MH_OK)
		{

			return;
		}

		if (MH_EnableHook(reinterpret_cast<LPVOID>(*ProcessEvent)) != MH_OK)
		{

			MH_RemoveHook(reinterpret_cast<LPVOID>(*ProcessEvent));
			return;
		}
	}
}
void unhookProcessEvent()
{
	if (engine::oProcessEvent)
	{

		if (MH_DisableHook(reinterpret_cast<LPVOID>(engine::oProcessEvent)) == MH_OK &&
			MH_RemoveHook(reinterpret_cast<LPVOID>(engine::oProcessEvent)) == MH_OK)
		{
			engine::oProcessEvent = nullptr;
		}
	}
}
void EngineShutdown()
{
	unhookProcessEvent();
}


void ClearSunkList()
{
	engine::bClearSunkList = true;
}

bool loadPirateGenerator()
{
	bool found = false;
	TArray<ULevel*> levels = AthenaGameViewportClient->World->Levels;
	for (UINT32 i = 0; i < levels.Count; i++)
	{
		if (!levels[i])
			continue;
		TArray<ACharacter*> actors = levels[i]->AActors;
		for (UINT32 j = 0; j < actors.Count; j++)
		{
			ACharacter* actor = actors[j];
			if (!actor)
				continue;
			if (actor->compareName("BP_PirateGenerator_LineUpUI_C"))
			{
				found = true;
				engine::carousel = reinterpret_cast<PirateGeneratorLineUpUI*>(actor);
				break;
			}
		}
		if (found)
			break;
	}
	return found;
}

uintptr_t getPirateGenerator()
{
	return (uintptr_t)engine::carousel;
}

int getMapNameCode(char* name)
{
	if (strstr(name, "wsp_resource_island_02_e") != NULL) // Barnacle Cay
	{
		return 1;
	}
	if (strstr(name, "wld_resource_island_01_b") != NULL) // Black Sand Atoll
	{
		return 2;
	}
	if (strstr(name, "wld_resource_island_02_b") != NULL) // Black Water Enclave
	{
		return 3;
	}
	if (strstr(name, "wld_resource_island_01_d") != NULL) // Blind Man's Lagoon
	{
		return 4;
	}
	if (strstr(name, "wsp_resource_island_01_f") != NULL) // Booty Isle
	{
		return 5;
	}
	if (strstr(name, "bsp_resource_island_01_a") != NULL) // Boulder Cay
	{
		return 6;
	}
	if (strstr(name, "dvr_resource_island_01_h") != NULL) // Brimstone Rock
	{
		return 7;
	}
	if (strstr(name, "wsp_resource_island_01_c") != NULL) // Castaway Isle
	{
		return 8;
	}
	if (strstr(name, "wsp_resource_island_01_a") != NULL) // Chicken Isle
	{
		return 9;
	}
	if (strstr(name, "dvr_resource_island_01_c") != NULL) // Cinder Islet
	{
		return 10;
	}
	if (strstr(name, "dvr_resource_island_01_b") != NULL) // Cursewater Shores
	{
		return 11;
	}
	if (strstr(name, "wsp_resource_island_01_e") != NULL) // Cutlass Cay
	{
		return 12;
	}
	if (strstr(name, "dvr_resource_island_01_g") != NULL) // Flame's End
	{
		return 13;
	}
	if (strstr(name, "wsp_resource_island_01_b") != NULL) // Fools Lagoon
	{
		return 14;
	}
	if (strstr(name, "dvr_resource_island_01_e") != NULL) // Glowstone Cay
	{
		return 15;
	}
	if (strstr(name, "wld_resource_island_02_c") != NULL) // Isle of Last Words
	{
		return 16;
	}
	if (strstr(name, "bsp_resource_island_02_f") != NULL) // Lagoon of Whispers
	{
		return 17;
	}
	if (strstr(name, "wld_resource_island_01_k") != NULL) // Liar's Backbone
	{
		return 18;
	}
	if (strstr(name, "bsp_resource_island_02_c") != NULL) // Lonely Isle
	{
		return 19;
	}
	if (strstr(name, "wsp_resource_island_02_a") != NULL) // Lookout Point
	{
		return 20;
	}
	if (strstr(name, "dvr_resource_island_01_i") != NULL) // Magma's Tide
	{
		return 21;
	}
	if (strstr(name, "wsp_resource_island_02_f") != NULL) // Mutineer Rock
	{
		return 22;
	}
	if (strstr(name, "wsp_resource_island_01_g") != NULL) // Old Salts Atoll
	{
		return 23;
	}
	if (strstr(name, "wsp_resource_island_01_d") != NULL) // Paradise Spring
	{
		return 24;
	}
	if (strstr(name, "bsp_resource_island_01_i") != NULL) // Picaroon Palms
	{
		return 25;
	}
	if (strstr(name, "wld_resource_island_01_c") != NULL) // Plunderer's Plight
	{
		return 26;
	}
	if (strstr(name, "bsp_resource_island_02_d") != NULL) // Rapier Cay
	{
		return 27;
	}
	if (strstr(name, "dvr_resource_island_01_f") != NULL) // Roaring Sands
	{
		return 28;
	}
	if (strstr(name, "bsp_resource_island_02_a") != NULL) // Rum Runner Isle
	{
		return 29;
	}
	if (strstr(name, "bsp_resource_island_01_b") != NULL) // Salty Sands
	{
		return 30;
	}
	if (strstr(name, "bsp_resource_island_02_b") != NULL) // Sandy Shallows
	{
		return 31;
	}
	if (strstr(name, "dvr_resource_island_01_a") != NULL) // Schored Pass
	{
		return 32;
	}
	if (strstr(name, "wld_resource_island_01_j") != NULL) // Scurvy Isley
	{
		return 33;
	}
	if (strstr(name, "bsp_resource_island_02_e") != NULL) // Sea Dog's Rest
	{
		return 34;
	}
	if (strstr(name, "wld_resource_island_01_h") != NULL) // Shark Tooth Key
	{
		return 35;
	}
	if (strstr(name, "wld_resource_island_02_e") != NULL) // Shiver Retreat
	{
		return 36;
	}
	if (strstr(name, "dvr_resource_island_01_d") != NULL) // The Forsaken Brink
	{
		return 37;
	}
	if (strstr(name, "wld_feature_tribute_peak") != NULL) // Tribute Peak
	{
		return 38;
	}
	if (strstr(name, "wld_resource_island_02_d") != NULL) // Tri-Rock Isle
	{
		return 39;
	}
	if (strstr(name, "bsp_resource_island_01_e") != NULL) // Twin Groves
	{
		return 40;
	}
	if (strstr(name, "dvr_feature_island_01_e") != NULL) // Ashen Reaches
	{
		return 41;
	}
	if (strstr(name, "bsp_feature_crescent_cove") != NULL) // Cannon Cove
	{
		return 42;
	}
	if (strstr(name, "bsp_feature_crescent_island") != NULL) // Crescent Isle
	{
		return 43;
	}
	if (strstr(name, "wsp_feature_crooks_hollow") != NULL) // Crook's Hollow
	{
		return 44;
	}
	if (strstr(name, "wsp_feature_devils_ridge") != NULL) // Devil's Ridge
	{
		return 45;
	}
	if (strstr(name, "wsp_feature_discovery_ridge") != NULL) // Discovery Ridge
	{
		return 46;
	}
	if (strstr(name, "dvr_feature_island_01_a") != NULL) // Fetcher's Rest
	{
		return 47;
	}
	if (strstr(name, "dvr_feature_island_01_b") != NULL) // Flintlock Peninsula
	{
		return 48;
	}
	if (strstr(name, "wld_feature_dragons_breath") != NULL) // Kraken's Fall
	{
		return 49;
	}
	if (strstr(name, "bsp_feature_lone_cove") != NULL) // Lone Cove
	{
		return 50;
	}
	if (strstr(name, "wld_feature_arches") != NULL) // Marauder's Arch
	{
		return 51;
	}
	if (strstr(name, "bsp_feature_mermaids_hideaway") != NULL) // Mermaid's Hideaway
	{
		return 52;
	}
	if (strstr(name, "wld_feature_old_faithful") != NULL) // Old Faithful Isle
	{
		return 53;
	}
	if (strstr(name, "wsp_feature_plunder_valley") != NULL) // Plunder Valley
	{
		return 54;
	}
	if (strstr(name, "dvr_feature_island_01_c") != NULL) // Ruby's Fall
	{
		return 55;
	}
	if (strstr(name, "bsp_feature_sailors_bounty") != NULL) // Sailor's Bounty
	{
		return 56;
	}
	if (strstr(name, "wsp_feature_sharkbait_cove") != NULL) // Shark Bait Cove
	{
		return 57;
	}
	if (strstr(name, "wld_feature_shipwreck_bay") != NULL) // Shipwreck Bay
	{
		return 58;
	}
	if (strstr(name, "bsp_feature_smugglers_bay") != NULL) // Smugglers' Bay
	{
		return 59;
	}
	if (strstr(name, "wsp_feature_snake_island") != NULL) // Snake Island
	{
		return 60;
	}
	if (strstr(name, "wld_feature_three_peaks") != NULL) // The Crooked Masts
	{
		return 61;
	}
	if (strstr(name, "dvr_feature_island_01_d") != NULL) // The Devil's Thirst
	{
		return 62;
	}
	if (strstr(name, "wld_feature_cavern_isle") != NULL) // The Sunken Grove
	{
		return 63;
	}
	if (strstr(name, "wsp_feature_thieves_haven") != NULL) // Thieves' Haven
	{
		return 64;
	}
	if (strstr(name, "bsp_feature_wanderers_archipelago") != NULL) // Wanderers Refuge
	{
		return 65;
	}
	return 0;
}


std::string getIslandNameByCode(int code)
{
	switch (code)
	{
	case 1: return "BarnaCle Cay";
	case 2: return "Black Sand Atoll";
	case 3: return "Black Water Enclave";
	case 4: return "Blind Man's Lagoon";
	case 5: return "Booty Isle";
	case 6: return "Boulder Cay";
	case 7: return "Brimstone Rock";
	case 8: return "Castaway Isle";
	case 9: return "Chicken Isle";
	case 10:return "Cinder Islet";
	case 11:return "Cursewater Shores";
	case 12:return "Cutlass Cay";
	case 13:return "Flame's End";
	case 14:return "Fools Lagoon";
	case 15:return "Glowstone Cay";
	case 16:return "Isle of Last Words";
	case 17:return "Lagoon of Whispers";
	case 18:return "Liar's Backbone";
	case 19:return "Lonely Isle";
	case 20:return "Lookout Point";
	case 21:return "Magma's Tide";
	case 22:return "Mutineer Rock";
	case 23:return "Old Salts Atoll";
	case 24:return "Paradise Spring";
	case 25:return "Picaroon Palms";
	case 26:return "Plunderer's Plight";
	case 27:return "Rapier Cay";
	case 28:return "Roaring Sands";
	case 29:return "Rum Runner Isle";
	case 30:return "Salty Sands";
	case 31:return "Sandy Shallows";
	case 32:return "Schored Pass";
	case 33:return "Scurvy Isley";
	case 34:return "Sea Dog's Rest";
	case 35:return "Shark Tooth Key";
	case 36:return "Shiver Retreat";
	case 37:return "The Forsaken Brink";
	case 38:return "Tribute Peak";
	case 39:return "Tri-Rock Isle";
	case 40:return "Twin Groves";
	case 41:return "Ashen Reaches";
	case 42:return "Cannon Cove";
	case 43:return "Crescent Isle";
	case 44:return "Crook's Hollow";
	case 45:return "Devil's Ridge";
	case 46:return "Discovery Ridge";
	case 47:return "Fetcher's Rest";
	case 48:return "Flintlock Peninsula";
	case 49:return "Kraken's Fall";
	case 50:return "Lone Cove";
	case 51:return "Marauder's Arch";
	case 52:return "Mermaid's Hideaway";
	case 53:return "Old Faithful Isle";
	case 54:return "Plunder Valley";
	case 55:return "Ruby's Fall";
	case 56:return "Sailor's Bounty";
	case 57:return "Shark Bait Cove";
	case 58:return "Shipwreck Bay";
	case 59:return "Smugglers' Bay";
	case 60:return "Snake Island";
	case 61:return "The Crooked Masts";
	case 62:return "The Devil's Thirst";
	case 63:return "The Sunken Grove";
	case 64:return "Thieves' Haven";
	case 65:return "Wanderers Refuge";
	default:return "No Island Data";
	}
}

std::string getShortName(std::string name)
{
	if (name.find("cannon_ball") != std::string::npos)
		return "Cannon";
	if (name.find("cannonball_chain_shot") != std::string::npos)
		return "Chain";
	if (name.find("cannonball_Grenade") != std::string::npos)
		return "Blunder";
	if (name.find("cannonball_cur_fire") != std::string::npos)
		return "Fire";
	if (name.find("cannonball_cur_ballast") != std::string::npos)
		return "Ballast";
	if (name.find("cannonball_cur_snooze") != std::string::npos)
		return "Weary";
	if (name.find("cannonball_cur_anchor") != std::string::npos)
		return "Anchor";
	if (name.find("cannonball_cur_grog") != std::string::npos)
		return "Grog";
	if (name.find("cannonball_cur_barrel") != std::string::npos)
		return "Barrel";
	if (name.find("cannonball_cur_rigging") != std::string::npos)
		return "Ringging";
	if (name.find("cannonball_cur_snake") != std::string::npos)
		return "Venom";
	if (name.find("cannonball_cur_passive") != std::string::npos)
		return "Peace";
	if (name.find("cannonball_cur_boogie") != std::string::npos)
		return "Jig";
	if (name.find("cannonball_cur_limp") != std::string::npos)
		return "Limp";
	if (name.find("cannonball_cur_rudder") != std::string::npos)
		return "Helm";

	if (name.find("repair_wood") != std::string::npos)
		return "Wood";
	if (name.find("PomegranateFresh") != std::string::npos)
		return "Pome";
	if (name.find("CoconutFresh") != std::string::npos)
		return "Coco";
	if (name.find("BananaFresh") != std::string::npos)
		return "Banana";
	if (name.find("PineappleFresh") != std::string::npos)
		return "Pineapple";
	if (name.find("MangoFresh") != std::string::npos)
		return "Mango";
	if (name.find("GrubsFresh") != std::string::npos)
		return "Grub";
	if (name.find("LeechesFresh") != std::string::npos)
		return "Lee";
	if (name.find("EarthwormsFresh") != std::string::npos)
		return "Earth";
	if (name.find("fireworks_flare") != std::string::npos)
		return "Flare";
	if (name.find("fireworks_rocket") != std::string::npos)
		return "Fire S";
	if (name.find("fireworks_cake") != std::string::npos)
		return "Fire M";
	if (name.find("fireworks_living") != std::string::npos)
		return "Fire L";
	if (name.find("MapInABarrel") != std::string::npos)
		return "Map";
	if (name.find("ChickenMeatUndercooked") != std::string::npos)
		return "Chicken";
	if (name.find("PorkMeatUndercooked") != std::string::npos)
		return "Pork";
	if (name.find("SnakeMeatUndercooked") != std::string::npos)
		return "Snake";
	if (name.find("SharkMeatUndercooked") != std::string::npos)
		return "Shark";
	if (name.find("MegMeatUndercooked") != std::string::npos)
		return "Meg";
	if (name.find("KrakenMeatUndercooked") != std::string::npos)
		return "Kraken";
	if (name.find("RubyUndercooked") != std::string::npos) 
		return "Rubyfish";
	if (name.find("SunnyUndercooked") != std::string::npos)
		return "Sunnyfish";
	if (name.find("IndigoUndercooked") != std::string::npos)
		return "Indigofish";
	if (name.find("UmberUndercooked") != std::string::npos)
		return "Umberfish";
	if (name.find("SeafoamUndercooked") != std::string::npos)
		return "Seafoamfish";
	if (name.find("CharcoalUndercooked") != std::string::npos)
		return "Charcoalfish";
	if (name.find("BronzeUndercooked") != std::string::npos)
		return "Bronzefish";
	if (name.find("BrightUndercooked") != std::string::npos)
		return "Brightfish";
	if (name.find("MoonskyUndercooked") != std::string::npos)
		return "Moonskyfish";
	if (name.find("StoneUndercooked") != std::string::npos)
		return "Stonefish";
	if (name.find("MossUndercooked") != std::string::npos)
		return "Mossfish";
	if (name.find("HoneyUndercooked") != std::string::npos)
		return "Honeyfish";
	if (name.find("RavenUndercooked") != std::string::npos)
		return "Ravenfish";
	if (name.find("AmethystUndercooked") != std::string::npos)
		return "Amethystfish";
	if (name.find("ChickenMeatCooked") != std::string::npos)
		return "Coocked Chicken";
	if (name.find("PorkMeatCooked") != std::string::npos)
		return "Coocked Pork";
	if (name.find("SnakeMeatCooked") != std::string::npos)
		return "Coocked Snake";
	if (name.find("SharkMeatCooked") != std::string::npos)
		return "Coocked Shark";
	if (name.find("MegMeatCooked") != std::string::npos)
		return "Coocked Meg";
	if (name.find("KrakenMeatCooked") != std::string::npos)
		return "Coocked Kraken";
	if (name.find("RubyCooked") != std::string::npos) 
		return "Coocked Rubyfish";
	if (name.find("SunnyCooked") != std::string::npos)
		return "Coocked SunnyFish";
	if (name.find("IndigoCooked") != std::string::npos)
		return "Coocked Indigofish";
	if (name.find("UmberCooked") != std::string::npos)
		return "Coocked Umberfish";
	if (name.find("SeafoamCooked") != std::string::npos)
		return "Coocked Seafoamfish";
	if (name.find("CharcoalCooked") != std::string::npos)
		return "Coocked Fish";
	if (name.find("BronzeCooked") != std::string::npos)
		return "Coocked Charcoalfish";
	if (name.find("BrightCooked") != std::string::npos)
		return "Coocked Brightfish";
	if (name.find("MoonskyCooked") != std::string::npos)
		return "Coocked Moonskyfish";
	if (name.find("StoneCooked") != std::string::npos)
		return "Coocked Stonefish";
	if (name.find("MossCooked") != std::string::npos)
		return "Coocked Mossfish";
	if (name.find("HoneyCooked") != std::string::npos)
		return "Coocked Honeyfish";
	if (name.find("RavenCooked") != std::string::npos)
		return "Coocked Ravenfish";
	if (name.find("AmethystCooked") != std::string::npos)
		return "Coocked Amethystfish";
	return name;
}