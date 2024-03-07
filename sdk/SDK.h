#pragma once
#include <Windows.h>
#include "../library/ue4/UE4.h"
#include <string>

struct UFunction;
class UClass;

inline void ProcessEvent(void* obj, UFunction* function, void* parms)
{
	auto vtable = *reinterpret_cast<void***>(obj);
	reinterpret_cast<void(*)(void*, UFunction*, void*)>(vtable[55])(obj, function, parms);
}

template<typename Fn>
inline Fn GetVFunction(const void* instance, std::size_t index)
{
	auto vtable = *reinterpret_cast<const void***>(const_cast<void*>(instance));
	return reinterpret_cast<Fn>(vtable[index]);
}

struct FNameEntry
{
	uint32_t Index;
	uint32_t pad;
	FNameEntry* HashNext;
	char AnsiName[1024];

	const int GetIndex() const { return Index >> 1; }
	const char* GetAnsiName() const { return AnsiName; }
};

class TNameEntryArray
{
public:

	bool IsValidIndex(uint32_t index) const { return index < NumElements; }

	FNameEntry const* GetById(uint32_t index) const { return *GetItemPtr(index); }

	FNameEntry const* const* GetItemPtr(uint32_t Index) const {
		const auto ChunkIndex = Index / 16384;
		const auto WithinChunkIndex = Index % 16384;
		const auto Chunk = Chunks[ChunkIndex];
		return Chunk + WithinChunkIndex;
	}

	FNameEntry** Chunks[128];
	uint32_t NumElements = 0;
	uint32_t NumChunks = 0;
};


struct FTextData
{
public:
	wchar_t* Text;
};

struct FText
{
public:
	struct FTextData* TextData;
};

class FString : public TArray<wchar_t>
{
public:
	inline FString()
	{
	};

	FString(const wchar_t* other)
	{
		Max = Count = *other ? static_cast<int32_t>(std::wcslen(other)) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	};

	inline bool IsValid() const
	{
		return Data != nullptr;
	}

	int multi(char* name, int size) const
	{
		return WideCharToMultiByte(CP_UTF8, 0, Data, Count, name, size, nullptr, nullptr) - 1;
	}

	inline const wchar_t* wide() const
	{
		return Data;
	}

	inline const wchar_t* c_str() const
	{
		if (Data)
			return Data;
		return L"";
	}
};

struct FName
{
	int ComparisonIndex = 0;
	int Number = 0;

	static inline TNameEntryArray* GNames = nullptr;

	static const char* GetNameByIdFast(int Id) {
		auto NameEntry = GNames->GetById(Id);
		if (!NameEntry) return nullptr;
		return NameEntry->GetAnsiName();
	}

	static std::string GetNameById(int Id) {
		auto NameEntry = GNames->GetById(Id);
		if (!NameEntry) return std::string();
		return NameEntry->GetAnsiName();
	}

	const char* GetNameFast() const {
		auto NameEntry = GNames->GetById(ComparisonIndex);
		if (!NameEntry) return nullptr;
		return NameEntry->GetAnsiName();
	}

	const std::string GetName() const {
		auto NameEntry = GNames->GetById(ComparisonIndex);
		if (!NameEntry) return std::string();
		return NameEntry->GetAnsiName();
	};

	inline bool operator==(const FName& other) const {
		return ComparisonIndex == other.ComparisonIndex;
	};

	FName() {}

	FName(const char* find) {
		for (auto i = 6000u; i < GNames->NumElements; i++)
		{
			auto name = GetNameByIdFast(i);
			if (!name) continue;
			if (strcmp(name, find) == 0) {
				ComparisonIndex = i;
				return;
			};
		}
	}
};

struct FUObjectItem
{
	class UObject* Object;
	int Flags;
	int ClusterIndex;
	int SerialNumber;
	int pad;
};

struct TUObjectArray
{
	FUObjectItem* Objects;
	int MaxElements;
	int NumElements;

	class UObject* GetByIndex(int index) { return Objects[index].Object; }
};

struct UObject {
	UObject(UObject* addr) { *this = addr; }
	static inline TUObjectArray* GObjects = nullptr;
	void* Vtable;
	int ObjectFlags;
	int InternalIndex;
	UClass* Class;
	FName Name;
	UObject* Outer;

	std::string GetName() const;
	const char* GetNameFast() const;
	std::string GetFullName() const;
	bool IsA(UClass* cmp) const;

	template<typename T>
	static T* FindObject(const std::string& name)
	{
		for (int i = 0; i < GObjects->NumElements; ++i)
		{
			auto object = GObjects->GetByIndex(i);

			if (object == nullptr)
			{
				continue;
			}

			if (object->GetFullName() == name)
			{
				return static_cast<T*>(object);
			}
		}
		return nullptr;
	}

	static UClass* FindClass(const std::string& name)
	{
		return FindObject<UClass>(name);
	}
};

struct FVector_NetQuantize : public FVector
{

};
// ScriptStruct Engine.HitResult
// Size: 0x80 (Inherited: 0x00)
struct FHitResult
{
	unsigned char                                      bBlockingHit : 1;                                         // 0x0000(0x0001)
	unsigned char                                      bStartPenetrating : 1;                                    // 0x0000(0x0001)
	unsigned char                                      UnknownData00[0x3];                                       // 0x0001(0x0003) MISSED OFFSET
	float                                              Time;                                                     // 0x0004(0x0004) (ZeroConstructor, IsPlainOldData)
	float                                              Distance;                                                 // 0x0008(0x0004) (ZeroConstructor, IsPlainOldData)
	struct FVector_NetQuantize                         ImpactPoint;                                              // 0x0018(0x000C)
	char pad_5894[0x48];
	float                                              PenetrationDepth;                                         // 0x0054(0x0004) (ZeroConstructor, IsPlainOldData)
	int                                                Item;                                                     // 0x0058(0x0004) (ZeroConstructor, IsPlainOldData)
	char pad_3424[0x18];
	struct FName                                       BoneName;                                                 // 0x0074(0x0008) (ZeroConstructor, IsPlainOldData)
	int                                                FaceIndex;                                                // 0x007C(0x0004) (ZeroConstructor, IsPlainOldData)
};

// Enum Athena.EPlayerActivityType
enum class EPlayerActivityType : uint8_t
{
	None = 0,
	Bailing = 1,
	Cannon = 2,
	Cannon_END = 3,
	Capstan = 4,
	Capstan_END = 5,
	CarryingBooty = 6,
	CarryingBooty_END = 7,
	Dead = 8,
	Dead_END = 9,
	Digging = 10,
	Dousing = 11,
	EmptyingBucket = 12,
	Harpoon = 13,
	Harpoon_END = 14,
	LoseHealth = 15,
	Repairing = 16,
	Sails = 17,
	Sails_END = 18,
	UndoingRepair = 19,
	Wheel = 20,
	Wheel_END = 21,
};


// Class Engine.PlayerState
// Size: 0x450 (Inherited: 0x3c8)
struct APlayerState {
	char pad_00[0x3c8];
	float Score; // 0x3c8(0x04)
	char Ping; // 0x3cc(0x01)
	char UnknownData_3CD[0x3]; // 0x3cd(0x03)
	struct FString PlayerName; // 0x3d0(0x10)
	char UnknownData_3E0[0x10]; // 0x3e0(0x10)
	int32_t PlayerId; // 0x3f0(0x04)
	char bIsSpectator : 1; // 0x3f4(0x01)
	char bOnlySpectator : 1; // 0x3f4(0x01)
	char bIsABot : 1; // 0x3f4(0x01)
	char UnknownData_3F4_3 : 1; // 0x3f4(0x01)
	char bIsInactive : 1; // 0x3f4(0x01)
	char bFromPreviousLevel : 1; // 0x3f4(0x01)
	char UnknownData_3F4_6 : 2; // 0x3f4(0x01)
	char UnknownData_3F5[0x3]; // 0x3f5(0x03)
	int32_t StartTime; // 0x3f8(0x04)
	char UnknownData_3FC[0x4]; // 0x3fc(0x04)
	struct UClass* EngineMessageClass; // 0x400(0x08)
	char UnknownData_408[0x18]; // 0x408(0x18)
	char UniqueId[0x18]; // 0x420(0x18)
	char UnknownData_438[0x18]; // 0x438(0x18)

	void OnRep_UniqueId(); // Function Engine.PlayerState.OnRep_UniqueId // Native|Public // @ game+0x2ceae70
};

// Class Athena.DamageZone
// Size: 0x820 (Inherited: 0x400)
struct ADamageZone {
	char pad[0x654];
	int32_t DamageLevel; // 0x654(0x04)
};

// Class Athena.HullDamage
// Size: 0x5d0 (Inherited: 0x3c8)
struct AHullDamage {
	char pad[0x410];
	struct TArray<struct ADamageZone*> DamageZones; // 0x410(0x10)
	TArray<class ACharacter*> ActiveHullDamageZones; // 0x0420
};

struct AShipInternalWater {
	float GetNormalizedWaterAmount() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ShipInternalWater.GetNormalizedWaterAmount");
		float params = 0.f;
		ProcessEvent(this, fn, &params);
		return params;
	}

	void AddWaterAmount(float WaterToAdd) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ShipInternalWater.AddWaterAmount");
		struct {
			float WaterToAdd;
		} params;
		params.WaterToAdd = WaterToAdd;
		ProcessEvent(this, fn, &params);
	}
};

// Class AthenaAI.Fauna
// Size: 0xbe0 (Inherited: 0x810)
struct AFauna {
	char pad1[0x818];
	FText DisplayName; // 0x0818
};

// ScriptStruct Engine.RepMovement
// Size: 0x38 (Inherited: 0x00)
struct FRepMovement {
	struct FVector LinearVelocity; // 0x00(0x0c)
	struct FVector AngularVelocity; // 0x0c(0x0c)
	struct FVector Location; // 0x18(0x0c)
	struct FRotator Rotation; // 0x24(0x0c)
	char bSimulatedPhysicSleep : 1; // 0x30(0x01)
	char bRepPhysics : 1; // 0x30(0x01)
	char UnknownData_30_2 : 6; // 0x30(0x01)
	char LocationQuantizationLevel; // 0x31(0x01)
	char VelocityQuantizationLevel; // 0x32(0x01)
	char RotationQuantizationLevel; // 0x33(0x01)
	char UnknownData_34[0x4]; // 0x34(0x04)
};

// Class Engine.Actor
// Size: 0x3c8 (Inherited: 0x28)
struct AActor : UObject {
	char PrimaryActorTick[0x50]; // 0x28(0x50)
	float CustomTimeDilation; // 0x78(0x04)
	char bAllowRemovalFromServerWhenCollisionMerged : 1; // 0x7c(0x01)
	char bAllowRemovalFromServerWhenAutomaticallyInstanced : 1; // 0x7c(0x01)
	char bHidden : 1; // 0x7c(0x01)
	char bNetTemporary : 1; // 0x7c(0x01)
	char bNetStartup : 1; // 0x7c(0x01)
	char bOnlyRelevantToOwner : 1; // 0x7c(0x01)
	char bAlwaysRelevant : 1; // 0x7c(0x01)
	char bReplicateMovement : 1; // 0x7c(0x01)
	char bTearOff : 1; // 0x7d(0x01)
	char bExchangedRoles : 1; // 0x7d(0x01)
	char bPendingNetUpdate : 1; // 0x7d(0x01)
	char bNetLoadOnClient : 1; // 0x7d(0x01)
	char bNetUseOwnerRelevancy : 1; // 0x7d(0x01)
	char bBlockInput : 1; // 0x7d(0x01)
	char UnknownData_7D_6 : 1; // 0x7d(0x01)
	char bCanBeInCluster : 1; // 0x7d(0x01)
	char UnknownData_7E_0 : 2; // 0x7e(0x01)
	char bActorEnableCollision : 1; // 0x7e(0x01)
	char UnknownData_7E_3 : 1; // 0x7e(0x01)
	char bReplicateAttachment : 1; // 0x7e(0x01)
	char UnknownData_7E_5 : 1; // 0x7e(0x01)
	char bReplicates : 1; // 0x7e(0x01)
	char UnknownData_7F[0x1]; // 0x7f(0x01)
	char OnPreNetOwnershipChange; // 0x80(0x01)
	char UnknownData_81[0x2]; // 0x81(0x02)
	char RemoteRole; // 0x83(0x01)
	char UnknownData_84[0x4]; // 0x84(0x04)
	struct AActor* Owner; // 0x88(0x08)
	struct FRepMovement ReplicatedMovement; // 0x90(0x38)
	char AttachmentReplication[0x48]; // 0xc8(0x48)
	char Role; // 0x110(0x01)
	char UnknownData_111[0x1]; // 0x111(0x01)
	char SpawnRestrictions; // 0x112(0x01)
	char AutoReceiveInput; // 0x113(0x01)
	int32_t InputPriority; // 0x114(0x04)
	struct UInputComponent* InputComponent; // 0x118(0x08)
	float NetCullDistanceSquared; // 0x120(0x04)
	char UnknownData_124[0x4]; // 0x124(0x04)
	int32_t NetTag; // 0x128(0x04)
	float NetUpdateTime; // 0x12c(0x04)
	float NetUpdateFrequency; // 0x130(0x04)
	float NetPriority; // 0x134(0x04)
	float LastNetUpdateTime; // 0x138(0x04)
	struct FName NetDriverName; // 0x13c(0x08)
	char bAutoDestroyWhenFinished : 1; // 0x144(0x01)
	char bCanBeDamaged : 1; // 0x144(0x01)
	char bActorIsBeingDestroyed : 1; // 0x144(0x01)
	char bCollideWhenPlacing : 1; // 0x144(0x01)
	char bFindCameraComponentWhenViewTarget : 1; // 0x144(0x01)
	char bRelevantForNetworkReplays : 1; // 0x144(0x01)
	char UnknownData_144_6 : 2; // 0x144(0x01)
	char UnknownData_145[0x3]; // 0x145(0x03)
	char SpawnCollisionHandlingMethod; // 0x148(0x01)
	char UnknownData_149[0x7]; // 0x149(0x07)
	struct APawn* Instigator; // 0x150(0x08)
	struct TArray<struct AActor*> Children; // 0x158(0x10)
	struct USceneComponent* RootComponent; // 0x168(0x08)
	struct TArray<struct AMatineeActor*> ControllingMatineeActors; // 0x170(0x10)
	float InitialLifeSpan; // 0x180(0x04)
	char UnknownData_184[0x4]; // 0x184(0x04)
	char bAllowReceiveTickEventOnDedicatedServer : 1; // 0x188(0x01)
	char UnknownData_188_1 : 7; // 0x188(0x01)
	char UnknownData_189[0x7]; // 0x189(0x07)
	struct TArray<struct FName> Layers; // 0x190(0x10)
	char ParentComponentActor[0x8]; // 0x1a0(0x08)
	struct TArray<struct AActor*> ChildComponentActors; // 0x1a8(0x10)
	char UnknownData_1B8[0x8]; // 0x1b8(0x08)
	char bActorSeamlessTraveled : 1; // 0x1c0(0x01)
	char bIgnoresOriginShifting : 1; // 0x1c0(0x01)
	char bEnableAutoLODGeneration : 1; // 0x1c0(0x01)
	char InvertFeatureCheck : 1; // 0x1c0(0x01)
	char UnknownData_1C0_4 : 4; // 0x1c0(0x01)
	char UnknownData_1C1[0x3]; // 0x1c1(0x03)
	struct FName Feature; // 0x1c4(0x08)
	char UnknownData_1CC[0x4]; // 0x1cc(0x04)
	struct TArray<struct FName> Tags; // 0x1d0(0x10)
	uint64_t HiddenEditorViews; // 0x1e0(0x08)
	char OnActorBeginOverlap; // 0x1e8(0x01)
	char OnActorEndOverlap; // 0x1e9(0x01)
	char OnActorHit; // 0x1ea(0x01)
	char OnDestroyed; // 0x1eb(0x01)
	char UnknownData_1EC[0x3c]; // 0x1ec(0x3c)
	char OnEndPlay; // 0x228(0x01)
	bool bDoOverlapNotifiesOnLoad; // 0x229(0x01)
	char UnknownData_22A[0xf6]; // 0x22a(0xf6)
	struct TArray<struct UActorComponent*> BlueprintCreatedComponents; // 0x320(0x10)
	struct TArray<struct UActorComponent*> InstanceComponents; // 0x330(0x10)
	char UnknownData_340[0x8]; // 0x340(0x08)
	struct TArray<struct AActor*> ChildActorInterfaceProviders; // 0x348(0x10)
	char UnknownData_358[0x68]; // 0x358(0x68)
	double DormancyLingeringInSeconds; // 0x3c0(0x08)

	struct FVector GetActorRightVector()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorRightVector");
		FVector ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	}
};

// Class Engine.Pawn
// Size: 0x440 (Inherited: 0x3c8)
struct ANamedPawn {
	char pad[0x3e8];
	struct APlayerState* PlayerState; // 0x3e8(0x08)
};

// Class Engine.Pawn
// Size: 0x440 (Inherited: 0x3c8)
struct APawn : AActor {
	char pad[0x20];
	struct APlayerState* PlayerState; // 0x3e8(0x08)
	char pad2[0x50];
};

// ScriptStruct Athena.WorldMapIslandDataCaptureParams
// Size: 0x40 (Inherited: 0x00)
struct FWorldMapIslandDataCaptureParams
{
	char pad1[0x18];
	struct FVector                                     WorldSpaceCameraPosition;                                  // 0x0018(0x000C) (Edit, ZeroConstructor, Transient, EditConst, IsPlainOldData, NoDestructor)
	char pad2[0x8];
	float                                              CameraOrthoWidth;                                          // 0x002C(0x0004) (Edit, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)

};

// Class Athena.WorldMapIslandDataAsset
// Size: 0x78 (Inherited: 0x28)
struct UWorldMapIslandDataAsset {
	char pad[0x30];
	struct FWorldMapIslandDataCaptureParams            CaptureParams;                                             // 0x0030(0x0040) (Edit, BlueprintVisible, BlueprintReadOnly)
	FVector WorldSpaceCameraPosition;
	// ADD THE OFFSET OF CAPTUREPARAMS TO THIS OFFSET
};

// Class Athena.IslandDataAssetEntry
// Size: 0x118 (Inherited: 0x28)
struct UIslandDataAssetEntry {
	char pad_00[0x28];
	struct FName IslandName; // 0x28(0x08)
	struct TArray<struct FTreasureMapData> TreasureMaps; // 0x30(0x10)
	struct UWorldMapIslandDataAsset* WorldMapData; // 0x40(0x08)
	struct UGeneratedLocationsDataAsset* UndergroundTreasureLocationsSource; // 0x48(0x08)
	struct TArray<struct FVector> UndergroundTreasureLocations; // 0x50(0x10)
	struct ULandmarkTreasureLocationsDataAsset* LandmarkTreasureLocationsSource; // 0x60(0x08)
	struct UGeneratedLocationsDataAsset* AISpawnLocationsSource; // 0x68(0x08)
	struct TArray<struct FVector> AISpawnLocations; // 0x70(0x10)
	struct TArray<struct UIslandItemDataAsset*> IslandItemLocationDataSources; // 0x80(0x10)
	struct TArray<struct UIslandSalvageSpawnerCollection*> IslandSalvageSpawnerCollections; // 0x90(0x10)
	struct TArray<struct FTypedIslandItemSpawnLocationData> SalvageItemsLocationData; // 0xa0(0x10)
	FString* LocalisedName; // 0xb0(0x38)
	struct UAISpawner* AISpawner; // 0xe8(0x08)
	struct UAISpawner* CannonsAISpawner; // 0xf0(0x08)
	struct UAISpawner* EmergentCaptainSpawner; // 0xf8(0x08)
	struct UIslandMaterialStatusZone* IslandMaterialStatusZone; // 0x100(0x08)
	char StatToFireWhenPlayerSetsFootOnIsland[0x4]; // 0x108(0x04)
	char StatToFireWhenShipVisitsAnIsland[0x4]; // 0x10c(0x04)
	bool ShouldBeHiddenOnWorldMap; // 0x110(0x01)
	bool UseAdvancedSearchForMermaidSpawn; // 0x111(0x01)
	char IslandActiveEventType; // 0x112(0x01)
	char UnknownData_113[0x5]; // 0x113(0x05)
};

// Class Athena.IslandDataAsset
// Size: 0x58 (Inherited: 0x28)
struct UIslandDataAsset {
	char pad[0x48];
	struct TArray<struct UIslandDataAssetEntry*> IslandDataEntries; // 0x48(0x10)
};

// Class Athena.IslandService
// Size: 0x5e8 (Inherited: 0x3c8)
struct AIslandService {
	char pad[0x458];
	UIslandDataAsset* IslandDataAsset; // 0x458
};

struct FGuid
{
	int                                                A;                                                         // 0x0000(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                B;                                                         // 0x0004(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                C;                                                         // 0x0008(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	int                                                D;                                                         // 0x000C(0x0004) (Edit, ZeroConstructor, SaveGame, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

// ScriptStruct Athena.WorldMapShipLocation
// Size: 0x40 (Inherited: 0x00)
struct FWorldMapShipLocation {
	struct FGuid CrewId; // 0x00(0x10)
	struct UClass* ShipSize; // 0x10(0x08)
	struct FVector2D Location; // 0x18(0x08)
	float Rotation; // 0x20(0x04)
	char ReplicatedRotation; // 0x24(0x01)
	char Flags; // 0x25(0x01)
	char UnknownData_26[0x2]; // 0x26(0x02)
	struct UTexture* CrewLiveryOverlayIcon; // 0x28(0x08)
	char ReapersMarkLevel; // 0x30(0x01)
	char EmissaryLevel; // 0x31(0x01)
	bool OwnerIsInFaction; // 0x32(0x01)
	bool OwnerIsMaxFaction; // 0x33(0x01)
	struct FName OwnerFactionName; // 0x34(0x08)
	bool OwnerIsInvadingShip; // 0x3c(0x01)
	char UnknownData_3D[0x3]; // 0x3d(0x03)
};

// ScriptStruct Sessions.SessionTemplate
// Size: 0x18 (Inherited: 0x00)
struct FSessionTemplate
{
	struct FString                                     TemplateName;                                              // 0x0000(0x0010) (ZeroConstructor, Protected, HasGetValueTypeHash)
	unsigned char             SessionType;                                               // 0x0010(0x0001) (ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
	unsigned char                                      UnknownData_2Q1C[0x3];                                     // 0x0011(0x0003) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	int                                                MaxPlayers;                                                // 0x0014(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)

};

// ScriptStruct Sessions.CrewSessionTemplate
// 0x0020 (0x0038 - 0x0018)
struct FCrewSessionTemplate : public FSessionTemplate
{
	struct FString                                     MatchmakingHopper;                                         // 0x0018(0x0010) (ZeroConstructor, HasGetValueTypeHash)
	class UClass* ShipSize;                                                  // 0x0028(0x0008) (ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash)
	int                                                MaxMatchmakingPlayers;                                     // 0x0030(0x0004) (ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	unsigned char                                      UnknownData_JPXK[0x4];                                     // 0x0034(0x0004) MISSED OFFSET (PADDING)

};

// ScriptStruct Athena.Crew
// Size: 0xa0 (Inherited: 0x00)
struct FCrew {
	struct FGuid CrewId; // 0x00(0x10)
	struct FGuid SessionId; // 0x10(0x10)
	struct TArray<struct APlayerState*> Players; // 0x20(0x10)
	struct FCrewSessionTemplate CrewSessionTemplate; // 0x30(0x38)
	struct FGuid LiveryID; // 0x68(0x10)
	char UnknownData_78[0x8]; // 0x78(0x08)
	struct TArray<struct AActor*> AssociatedActors; // 0x80(0x10)
	bool HasEverSetSail; // 0x90(0x01)
	bool RequiresDiveOnceShipHasSpawned; // 0x91(0x01)
	bool CrewDivingToWorldEvent; // 0x92(0x01)
	char UnknownData_93[0x1]; // 0x93(0x01)
	int32_t ScrambleNameIndex; // 0x94(0x04)
	char pad_FGenericTeamId_TeamID[0x1]; // 0x98(0x01)
	char UnknownData_99[0x7]; // 0x99(0x07)
};

struct ACrewService {
	char pad[0x4A0];
	struct TArray<struct FCrew> Crews; // 0x4a0(0x10)
};

struct AShipService
{
	int GetNumShips()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ShipService.GetNumShips");
		int num;
		ProcessEvent(this, fn, &num);
		return num;
	}
};

// Class Athena.AthenaGameState
// Size: 0xca8 (Inherited: 0x540)
struct AAthenaGameState {
	char UnknownData_00[0x540]; // 0x0(0x540)
	char UnknownData_540[0x78]; // 0x540(0x78)
	struct AWindService* WindService; // 0x5b8(0x08)
	struct APlayerManagerService* PlayerManagerService; // 0x5c0(0x08)
	struct AShipService* ShipService; // 0x5c8(0x08)
	struct AWatercraftService* WatercraftService; // 0x5d0(0x08)
	struct ATimeService* TimeService; // 0x5d8(0x08)
	struct UHealthCustomizationService* HealthService; // 0x5e0(0x08)
	struct UCustomWeatherService* CustomWeatherService; // 0x5e8(0x08)
	struct UCustomStatusesService* CustomStatusesService; // 0x5f0(0x08)
	struct AFFTWaterService* WaterService; // 0x5f8(0x08)
	struct AStormService* StormService; // 0x600(0x08)
	struct ACrewService* CrewService; // 0x608(0x08)
	struct ACaptainedSessionService* CaptainedSessionService; // 0x610(0x08)
	struct AContestZoneService* ContestZoneService; // 0x618(0x08)
	struct AIslandService* IslandService; // 0x620(0x08)
	struct ANPCService* NPCService; // 0x628(0x08)
	struct ASkellyFortService* SkellyFortService; // 0x630(0x08)
	struct ADeepSeaRegionService* DeepSeaRegionService; // 0x638(0x08)
	struct UAIActionSpotService* AIActionSpotService; // 0x640(0x08)
	struct AAIDioramaService* AIDioramaService; // 0x648(0x08)
	struct AAshenLordEncounterService* AshenLordEncounterService; // 0x650(0x08)
	struct AAggressiveGhostShipsEncounterService* AggressiveGhostShipsEncounterService; // 0x658(0x08)
	struct ATallTaleService* TallTaleService; // 0x660(0x08)
	struct AAIShipObstacleService* AIShipObstacleService; // 0x668(0x08)
	struct AAIShipService* AIShipService; // 0x670(0x08)
	struct AAITargetService* AITargetService; // 0x678(0x08)
	struct UShipLiveryCatalogueService* ShipLiveryCatalogueService; // 0x680(0x08)
	struct ADrawDebugService* DrawDebugService; // 0x688(0x08)
	struct AWorldEventZoneService* WorldEventZoneService; // 0x690(0x08)
	struct UWorldResourceRegistry* WorldResourceRegistry; // 0x698(0x08)
	struct AKrakenService* KrakenService; // 0x6a0(0x08)
	struct UPlayerNameService* PlayerNameService; // 0x6a8(0x08)
	struct ATinySharkService* TinySharkService; // 0x6b0(0x08)
	struct AProjectileService* ProjectileService; // 0x6b8(0x08)
	struct AProjectileHitRegService* ProjectileHitRegService; // 0x6c0(0x08)
	struct ULaunchableProjectileService* LaunchableProjectileService; // 0x6c8(0x08)
	struct UServerNotificationsService* ServerNotificationsService; // 0x6d0(0x08)
	struct AAIManagerService* AIManagerService; // 0x6d8(0x08)
	struct AAIEncounterService* AIEncounterService; // 0x6e0(0x08)
	struct AAIEncounterGenerationService* AIEncounterGenerationService; // 0x6e8(0x08)
	struct UEncounterService* EncounterService; // 0x6f0(0x08)
	struct UGameEventSchedulerService* GameEventSchedulerService; // 0x6f8(0x08)
	struct UGameEventsOnDemandAvailabilityService* GameEventsOnDemandAvailabilityService; // 0x700(0x08)
	struct AGameEventsOnDemandLoserTunnelService* GameEventsOnDemandLoserTunnelService; // 0x708(0x08)
	struct UHideoutService* HideoutService; // 0x710(0x08)
	struct UAthenaStreamedLevelService* StreamedLevelService; // 0x718(0x08)
	struct ULocationProviderService* LocationProviderService; // 0x720(0x08)
	struct AHoleService* HoleService; // 0x728(0x08)
	struct APlayerBuriedItemService* PlayerBuriedItemService; // 0x730(0x08)
	struct ULoadoutService* LoadoutService; // 0x738(0x08)
	struct UOcclusionService* OcclusionService; // 0x740(0x08)
	struct UPetsService* PetsService; // 0x748(0x08)
	struct UAthenaAITeamsService* AthenaAITeamsService; // 0x750(0x08)
	struct AAllianceService* AllianceService; // 0x758(0x08)
	struct UMaterialAccessibilityService* MaterialAccessibilityService; // 0x760(0x08)
	struct UNPCLoadedOnClientService* NPCLoadedOnClientService; // 0x768(0x08)
	struct AReapersMarkService* ReapersMarkService; // 0x770(0x08)
	struct AEmissaryLevelService* EmissaryLevelService; // 0x778(0x08)
	struct AFactionService* FactionService; // 0x780(0x08)
	struct ACampaignService* CampaignService; // 0x788(0x08)
	struct AStoryService* StoryService; // 0x790(0x08)
	struct AStorySpawnedActorsService* StorySpawnedActorsService; // 0x798(0x08)
	struct AStoryClaimedResourcesService* StoryClaimedResourcesService; // 0x7a0(0x08)
	struct UGlobalVoyageDirectorService* GlobalVoyageDirector; // 0x7a8(0x08)
	struct UUtilityVoyageService* UtilityVoyageService; // 0x7b0(0x08)
	struct AFlamesOfFateSettingsService* FlamesOfFateSettingsService; // 0x7b8(0x08)
	struct AServiceStatusNotificationsService* ServiceStatusNotificationsService; // 0x7c0(0x08)
	struct UMigrationService* MigrationService; // 0x7c8(0x08)
	struct UShipStockService* ShipStockService; // 0x7d0(0x08)
	struct AShroudBreakerService* ShroudBreakerService; // 0x7d8(0x08)
	struct UServerUpdateReportingService* ServerUpdateReportingService; // 0x7e0(0x08)
	struct AGenericMarkerService* GenericMarkerService; // 0x7e8(0x08)
	struct AMechanismsService* MechanismsService; // 0x7f0(0x08)
	struct UMerchantContractsService* MerchantContractsService; // 0x7f8(0x08)
	struct UShipFactory* ShipFactory; // 0x800(0x08)
	struct URewindPhysicsService* RewindPhysicsService; // 0x808(0x08)
	struct UNotificationMessagesDataAsset* NotificationMessagesDataAsset; // 0x810(0x08)
	struct AProjectileCooldownService* ProjectileCooldownService; // 0x818(0x08)
	struct UIslandReservationService* IslandReservationService; // 0x820(0x08)
	struct APortalService* PortalService; // 0x828(0x08)
	struct UMeshMemoryConstraintService* MeshMemoryConstraintService; // 0x830(0x08)
	struct ABootyStorageService* BootyStorageService; // 0x838(0x08)
	struct ALoadoutCostService* LoadoutCostService; // 0x840(0x08)
	struct ASpireService* SpireService; // 0x848(0x08)
	struct AFireworkService* FireworkService; // 0x850(0x08)
	struct AInvasionService* InvasionService; // 0x858(0x08)
	struct UAirGivingService* AirGivingService; // 0x860(0x08)
	struct UCutsceneService* CutsceneService; // 0x868(0x08)
	struct ACargoRunService* CargoRunService; // 0x870(0x08)
	struct ACommodityDemandService* CommodityDemandService; // 0x878(0x08)
	struct ADebugTeleportationDestinationService* DebugTeleportationDestinationService; // 0x880(0x08)
	struct ASeasonProgressionUIService* SeasonProgressionUIService; // 0x888(0x08)
	struct UTransientActorService* TransientActorService; // 0x890(0x08)
	struct ATunnelsOfTheDamnedService* TunnelsOfTheDamnedService; // 0x898(0x08)
	struct UWorldSequenceService* WorldSequenceService; // 0x8a0(0x08)
	struct UItemLifetimeManagerService* ItemLifetimeManagerService; // 0x8a8(0x08)
	struct AShipStorageJettisonService* ShipStorageJettisonService; // 0x8b0(0x08)
	struct ASeaFortsService* SeaFortsService; // 0x8b8(0x08)
	struct ACustomServerLocalisationService* CustomServerLocalisationService; // 0x8c0(0x08)
	struct ABeckonService* BeckonService; // 0x8c8(0x08)
	struct UVolcanoService* VolcanoService; // 0x8d0(0x08)
	struct UShipAnnouncementService* ShipAnnouncementService; // 0x8d8(0x08)
	struct AShipsLogService* ShipsLogService; // 0x8e0(0x08)
	struct UAsyncLoadingMonitoringService* AsyncLoadingMonitoringService; // 0x8e8(0x08)
	struct AActorOfInterestService* ActorOfInterestService; // 0x8f0(0x08)
	struct AEnemyAIOnShipService* EnemyAIOnShipService; // 0x8f8(0x08)
	struct AUserSettingsService* UserSettingsService; // 0x900(0x08)
	struct AGuildService* GuildService; // 0x908(0x08)
	struct UGuildChroniclesService* GuildChroniclesService; // 0x910(0x08)
	struct APlayerLootMarkerService* PlayerLootMarkerService; // 0x918(0x08)
	struct AInventoryPersistenceService* InventoryPersistenceService; // 0x920(0x08)
	struct AAdventureOnDemandService* AdventureOnDemandService; // 0x928(0x08)
	struct AScrambledNamesService* ScrambledNamesService; // 0x930(0x08)
	struct UExperienceService* ExperienceService; // 0x938(0x08)
	struct UQuestTableService* QuestTableService; // 0x940(0x08)
	struct ACompetitiveVoyagesService* CompetitiveVoyagesService; // 0x948(0x08)
	char UnknownData_958[0x1c0]; // 0x958(0x1c0)
	char ServiceCoordinator[0x20]; // 0xb18(0x20)
	char UnknownData_B38[0x28]; // 0xb38(0x28)
	char ChatComponents[0x10]; // 0xb60(0x10)
	char UnknownData_B70[0xb0]; // 0xb70(0xb0)
	bool IsXboxGamePadOnlyServer; // 0xc20(0x01)
	bool ShouldDisableAsyncOcclusionCheck; // 0xc21(0x01)
	char UnknownData_C22[0x6]; // 0xc22(0x06)
	struct FString SubPlayMode; // 0xc28(0x10)
	struct UCustomVaultService* CustomVaultService; // 0xc38(0x08)
	struct UEntityEnumerationService* EntityEnumerationService; // 0xc40(0x08)
	struct ULevelAssetCachingService* LevelAssetCachingService; // 0xc48(0x08)
	struct UCrewSkillRatingService* CrewSkillRatingService; // 0xc50(0x08)
	char UnknownData_C58[0x50]; // 0xc58(0x50)
};

struct UItemDesc {
	char pad_28[0x28];
	FString* Title; // 0x28(0x38)
	char pad_c0[0xe8];
};

// Class Athena.EntitlementDesc
// Size: 0xc8 (Inherited: 0x28)
struct UItemDescEx : UObject {
	struct FText Title; // 0x28(0x38)
	struct FText Description; // 0x60(0x38)
	char pad_98[0x30];
	char pad_c0[0x58];
};

// Class Athena.ItemInfo
// Size: 0x530 (Inherited: 0x3c8)
struct AItemInfo {
	char pad[0x460];
	struct UItemDesc* Desc; // 0x460(0x08)
};

// Class Engine.Character
// Size: 0x5e0 (Inherited: 0x440)
struct ACharacter : APawn {
	char pad[0x1a0];

	bool IsLoading() {
		static auto fn = UObject::FindObject<UFunction>("Function AthenaLoadingScreen.AthenaLoadingScreenBlueprintFunctionLibrary.IsLoadingScreenVisible");
		bool isLoading = true;
		ProcessEvent(this, fn, &isLoading);
		return isLoading;
	}

	inline bool isItem() {
		static auto obj = UObject::FindClass("Class Athena.ItemProxy");
		return IsA(obj);
	}

	inline bool isFarShip() {
		static auto obj = UObject::FindClass("Class Athena.ShipNetProxy");
		return IsA(obj);
	}

	inline bool isShip() {
		static auto obj = UObject::FindClass("Class Athena.Ship");
		return IsA(obj);
	}

	inline bool isGhostShip() {
		static auto obj = UObject::FindClass("Class Athena.AggressiveGhostShip");
		return IsA(obj);
	}

	inline bool isSkeleton() {
		static auto obj = FindClass("Class Athena.AthenaAICharacter");
		return IsA(obj);
	}

	inline bool isPlayer() {
		static auto obj = UObject::FindClass("Class Athena.AthenaPlayerCharacter");
		return IsA(obj);
	}

	inline bool isAmmoChest() {
		static auto obj = FindClass("Class Athena.AmmoChest");
		return IsA(obj);
	}

	inline bool isXMarkMap() {
		static auto obj = UObject::FindClass("Class Athena.XMarksTheSpotMap");
		return IsA(obj);
	}

	inline bool isShipwreck() {
		static auto obj = UObject::FindClass("Class Athena.Shipwreck");
		return IsA(obj);
	}

	inline bool isMermaid() {
		static auto obj = UObject::FindClass("Class Athena.Mermaid");
		return IsA(obj);
	}

	inline bool isRowboat() {
		static auto obj = UObject::FindClass("Class Watercrafts.Rowboat");
		return IsA(obj);
	}

	inline bool isShark() {
		static auto obj = UObject::FindClass("Class Athena.SharkPawn");
		return IsA(obj);
	}

	inline bool isEvent() {
		static auto obj = UObject::FindClass("Class Athena.GameplayEventSignal");
		return IsA(obj);
	}

	inline bool isPuzzleVault() {
		static auto obj = UObject::FindClass("Class Athena.PuzzleVault");
		return IsA(obj);
	}

	inline bool isBarrel() {
		static auto obj = UObject::FindClass("Class Athena.StorageContainer");
		return IsA(obj);
	}

	inline bool isStorageComponent() {
		static auto obj = UObject::FindClass("Class Athena.StorageContainerComponent");
		return IsA(obj);
	}

	inline bool isAnimal() {
		static auto obj = UObject::FindClass("Class AthenaAI.Fauna");
		return IsA(obj);
	}

	bool IsDead() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.IsDead");
		bool isDead = true;
		ProcessEvent(this, fn, &isDead);
		return isDead;
	}

	bool IsInWater() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.IsInWater");
		bool isInWater = false;
		ProcessEvent(this, fn, &isInWater);
		return isInWater;
	}

	bool isWeapon() {
		static auto obj = UObject::FindClass("Class Athena.ProjectileWeapon");
		return IsA(obj);
	}

	bool isSpyglass() {
		static auto obj = UObject::FindClass("Class Athena.Spyglass");
		return IsA(obj);
	}

	inline bool isCannon() {
		static auto obj = UObject::FindClass("Class Athena.Cannon");
		return IsA(obj);
	}

	inline bool isMapTable() {
		static auto obj = UObject::FindClass("Class Athena.MapTable");
		return IsA(obj);
	}

	inline bool isCookingPot() {
		static auto obj = UObject::FindClass("Class Cooking.CookingPot");
		return IsA(obj);
	}

	inline bool isCookableComponent() {
		static auto obj = UObject::FindClass("Class Cooking.CookableComponent");
		return IsA(obj);
	}

	inline bool isLightningController() {
		static auto obj = UObject::FindClass("Class Athena.LightingController");
		return IsA(obj);
	}

	AHullDamage* GetHullDamage() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Ship.GetHullDamage");
		AHullDamage* params = nullptr;
		ProcessEvent(this, fn, &params);
		return params;
	}

	AShipInternalWater* GetInternalWater() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Ship.GetInternalWater");
		AShipInternalWater* params = nullptr;
		ProcessEvent(this, fn, &params);
		return params;
	}

	FVector GetActorForwardVector() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorForwardVector");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	FVector GetActorUpVector() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorUpVector");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	ACharacter* GetParentActor() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetParentActor");
		ACharacter* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	};

	ACharacter* GetAttachParentActor() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetAttachParentActor");
		ACharacter* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	}

	ACharacter* GetCurrentShip()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.AthenaCharacter.GetCurrentShip");

		struct
		{
			EShipTrackedType type = EShipTrackedType::OnShipDeck;
			ACharacter* ReturnValue = nullptr;
		} params;

		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}

	// Enum Athena.EShipTrackedType
	enum class EShipTrackedType : uint8_t {
		OnShipDeck,
		OnShipDeckOrLadder,
		ShipTrackedTypeMax,
		EShipTrackedType_MAX,
	};

	FVector GetVelocity() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetVelocity");
		FVector velocity;
		ProcessEvent(this, fn, &velocity);
		return velocity;
	}

	FVector GetForwardVelocity() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorForwardVector");
		FVector ForwardVelocity;
		ProcessEvent(this, fn, &ForwardVelocity);
		return ForwardVelocity;
	}

	FVector K2_GetActorLocation() {
		static auto fn = FindObject<UFunction>("Function Engine.Actor.K2_GetActorLocation");
		FVector params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	FRotator K2_GetActorRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.K2_GetActorRotation");
		FRotator params;
		ProcessEvent(this, fn, &params);
		return params;
	}

	AItemInfo* GetItemInfo() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ItemProxy.GetItemInfo");
		AItemInfo* info = nullptr;
		ProcessEvent(this, fn, &info);
		return info;
	}

	void GetActorBounds(bool bOnlyCollidingComponents, FVector& Origin, FVector& BoxExtent) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Actor.GetActorBounds");
		struct
		{
			bool bOnlyCollidingComponents = false;
			FVector Origin;
			FVector BoxExtent;
		} params;

		params.bOnlyCollidingComponents = bOnlyCollidingComponents;

		ProcessEvent(this, fn, &params);

		Origin = params.Origin;
		BoxExtent = params.BoxExtent;
	}

	float GetTargetFOV(class AAthenaPlayerCharacter* Character) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.FOVHandlerFunctions.GetTargetFOV");
		struct
		{
			AAthenaPlayerCharacter* Character;
			float ReturnValue = 0.f;
		}params;
		params.Character = Character;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}

	inline bool compareName(const char* name)
	{
		auto type = GetName();
		if (type.find(name) != std::string::npos)
			return true;
		return false;
	}
};

// ScriptStruct Athena.StorageContainerNode
// Size: 0x10 (Inherited: 0x00)
struct FStorageContainerNode {
	struct UClass* ItemDesc; // 0x00(0x08)
	int32_t NumItems; // 0x08(0x04)
	char UnknownData_C[0x4]; // 0x0c(0x04)
};

// ScriptStruct Athena.StorageContainerBackingStore
// Size: 0x40 (Inherited: 0x00)
struct FStorageContainerBackingStore {
	char pad_0[0x20];
	struct TArray<struct FStorageContainerNode> ContainerNodes; // 0x20(0x10)
	bool AllowedItemsAreCached; // 0x30(0x01)
	char UnknownData_31[0x7]; // 0x31(0x07)
	char CachedAllowedItems[0x8]; // 0x38(0x08)
};

// Class Athena.StorageContainerComponent
// Size: 0x3c0 (Inherited: 0xc8)
struct UStorageContainerComponent {
	char pad_00[0xc8];
	char UnknownData_C8[0x20]; // 0xc8(0x20)
	char ContainerDisplayName[0x38]; // 0xe8(0x38)
	char InstanceTransform[0x30]; // 0x120(0x30)
	struct FStorageContainerBackingStore ContainerNodes; // 0x150(0x40)
	char UnknownData_228[0x8]; // 0x228(0x08)
	struct TArray<struct AActor*> QuickGivers; // 0x230(0x10)
	struct TArray<struct AActor*> QuickTakers; // 0x240(0x10)
	char AddItemSFX[0x8]; // 0x250(0x08)
	char TakeItemSFX[0x8]; // 0x258(0x08)
	char OpenContainerSFX[0x8]; // 0x260(0x08)
	char BeginQuickGiveSFX[0x8]; // 0x268(0x08)
	char EndQuickGiveSFX[0x8]; // 0x270(0x08)
	char BeginQuickTakeSFX[0x8]; // 0x278(0x08)
	char EndQuickTakeSFX[0x8]; // 0x280(0x08)
	char CrateFillerContainerSelector[0x8]; // 0x288(0x08)
	char StorageContainerSelector[0x8]; // 0x290(0x08)
	char UnknownData_298[0x30]; // 0x298(0x30)
	char SfxPool[0x8]; // 0x2c8(0x08)
	char UnknownData_2D0[0x40]; // 0x2d0(0x40)
	bool ShowCapacityInDescription; // 0x310(0x01)
	char UnknownData_311[0x9f]; // 0x311(0x9f)
	char Aggregator[0x8]; // 0x3b0(0x08)
	char UnknownData_3B8[0x8]; // 0x3b8(0x08)



	//void TakeItem(struct AActor* Player, int32_t NodeIndex); // Function Athena.StorageContainerComponent.TakeItem // Final|Native|Public|BlueprintCallable // @ game+0x38b5810
	//void OnRep_QuickTakers(struct TArray<struct AActor*> InOldTakers); // Function Athena.StorageContainerComponent.OnRep_QuickTakers // Final|Native|Private // @ game+0x38b5560
	//void OnRep_QuickGivers(struct TArray<struct AActor*> InOldGivers); // Function Athena.StorageContainerComponent.OnRep_QuickGivers // Final|Native|Private // @ game+0x38b5460
	//void OnRep_ContentsChanged(struct FStorageContainerBackingStore InOldItemCount); // Function Athena.StorageContainerComponent.OnRep_ContentsChanged // Final|Native|Private // @ game+0x38b52d0
	//void Multicast_DetachAllPlayersRPC(); // Function Athena.StorageContainerComponent.Multicast_DetachAllPlayersRPC // Final|Net|NetReliableNative|Event|NetMulticast|Private // @ game+0x38b52b0
	//struct FText GetContainerDisplayName(); // Function Athena.StorageContainerComponent.GetContainerDisplayName // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x38b5220
	//void AddItem(struct AActor* Player, struct UClass* InItemDesc); // Function Athena.StorageContainerComponent.AddItem // Final|Native|Public|BlueprintCallable // @ game+0x38b5010

};

// Class Athena.StorageContainer
// Size: 0x4c8 (Inherited: 0x468)
struct AStorageContainer {
	char pad_00[0x468];
	char UnknownData_468[0x8]; // 0x468(0x08)
	char Mesh[0x8]; // 0x470(0x08)
	char InteractionRegion[0x8]; // 0x478(0x08)
	char UnknownData_480[0x14]; // 0x480(0x14)
	char TrackedActorType; // 0x494(0x01)
	char UnknownData_495[0x33]; // 0x495(0x33)
	char random_Bytes[0x10];
	class UStorageContainerComponent* StorageContainer;
};

struct FFloatRange {
	float pad1;
	float min;
	float pad2;
	float max;
};

// Class Athena.Cannon
// Size: 0xdb0 (Inherited: 0x510)
struct ACannonLoadedItemInfo
{
	char pad[0x770];
	struct AItemInfo* LoadedItemInfo; // 0x770(0x08)
};

// Class Athena.Cannon
// Size: 0xdb0 (Inherited: 0x510)
struct ACannon {
	char pad_00[0x510];
	char UnknownData_510[0x30]; // 0x510(0x30)
	struct USkeletalMeshMemoryConstraintComponent* BaseMeshComponent; // 0x540(0x08)
	struct UStaticMeshMemoryConstraintComponent* BarrelMeshComponent; // 0x548(0x08)
	struct UStaticMeshComponent* FuseMeshComponent; // 0x550(0x08)
	struct UReplicatedShipPartCustomizationComponent* CustomizationComponent; // 0x558(0x08)
	struct ULoadableComponent* LoadableComponent; // 0x560(0x08)
	struct ULoadingPointComponent* LoadingPointComponent; // 0x568(0x08)
	struct UChildActorComponent* CannonBarrelInteractionComponent; // 0x570(0x08)
	struct UFuseComponent* FuseComponent; // 0x578(0x08)
	struct FName CameraSocket; // 0x580(0x08)
	struct FName CameraInsideCannonSocket; // 0x588(0x08)
	struct FName LaunchSocket; // 0x590(0x08)
	struct FName TooltipSocket; // 0x598(0x08)
	struct FName AudioAimRTPCName; // 0x5a0(0x08)
	struct FName InsideCannonRTPCName; // 0x5a8(0x08)
	struct UClass* ProjectileClass; // 0x5b0(0x08)
	float TimePerFire; // 0x5b8(0x04)
	float ProjectileSpeed; // 0x5bc(0x04)
	float ProjectileGravityScale; // 0x5c0(0x04)
	struct FFloatRange PitchRange; // 0x5c4(0x10)
	struct FFloatRange YawRange; // 0x5d4(0x10)
	float PitchSpeed; // 0x5e4(0x04)
	float YawSpeed; // 0x5e8(0x04)
	char UnknownData_5EC[0x4]; // 0x5ec(0x04)
	struct UClass* CameraShake; // 0x5f0(0x08)
	float ShakeInnerRadius; // 0x5f8(0x04)
	float ShakeOuterRadius; // 0x5fc(0x04)
	float CannonFiredAINoiseRange; // 0x600(0x04)
	struct FName AINoiseTag; // 0x604(0x08)
	char UnknownData_60C[0x4]; // 0x60c(0x04)
	struct FText CannonDisabledToolTipText; // 0x610(0x38)
	struct FText LoadingDisabledToolTipText; // 0x648(0x38)
	struct UClass* UseCannonInputId; // 0x680(0x08)
	struct UClass* StartLoadingCannonInputId; // 0x688(0x08)
	struct UClass* StopLoadingCannonInputId; // 0x690(0x08)
	float DefaultFOV; // 0x698(0x04)
	float AimFOV; // 0x69c(0x04)
	float IntoAimBlendSpeed; // 0x6a0(0x04)
	float OutOfAimBlendSpeed; // 0x6a4(0x04)
	struct UWwiseEvent* FireSfx; // 0x6a8(0x08)
	struct UWwiseEvent* DryFireSfx; // 0x6b0(0x08)
	struct UWwiseEvent* LoadingSfx_Play; // 0x6b8(0x08)
	struct UWwiseEvent* LoadingSfx_Stop; // 0x6c0(0x08)
	struct UWwiseEvent* UnloadingSfx_Play; // 0x6c8(0x08)
	struct UWwiseEvent* UnloadingSfx_Stop; // 0x6d0(0x08)
	struct UWwiseEvent* LoadedPlayerSfx; // 0x6d8(0x08)
	struct UWwiseEvent* UnloadedPlayerSfx; // 0x6e0(0x08)
	struct UWwiseEvent* FiredPlayerSfx; // 0x6e8(0x08)
	struct UWwiseObjectPoolWrapper* SfxPool; // 0x6f0(0x08)
	struct UWwiseEvent* StartPitchMovement; // 0x6f8(0x08)
	struct UWwiseEvent* StopPitchMovement; // 0x700(0x08)
	struct UWwiseEvent* StartYawMovement; // 0x708(0x08)
	struct UWwiseEvent* StopYawMovement; // 0x710(0x08)
	struct UWwiseEvent* StopMovementAtEnd; // 0x718(0x08)
	struct UWwiseObjectPoolWrapper* SfxMovementPool; // 0x720(0x08)
	struct UObject* FuseVfxFirstPerson; // 0x728(0x08)
	struct UObject* FuseVfxThirdPerson; // 0x730(0x08)
	struct UObject* MuzzleFlashVfxFirstPerson; // 0x738(0x08)
	struct UObject* MuzzleFlashVfxThirdPerson; // 0x740(0x08)
	struct FName FuseSocketName; // 0x748(0x08)
	struct FName BarrelSocketName; // 0x750(0x08)
	struct UClass* RadialCategoryFilter; // 0x758(0x08)
	struct UClass* DefaultLoadedItemDesc; // 0x760(0x08)
	float ClientRotationBlendTime; // 0x768(0x04)
	char UnknownData_76C[0x4]; // 0x76c(0x04)
	struct AItemInfo* LoadedItemInfo; // 0x770(0x08)
	bool FiringDisabled; // 0x778(0x01)
	char UnknownData_779[0x1f]; // 0x779(0x1f)
	struct UMemoryConstrainedMeshInitializer* BaseMMCMeshInitializer; // 0x798(0x08)
	struct UMemoryConstrainedMeshInitializer* BarrelMMCMeshInitializer; // 0x7a0(0x08)
	struct UCannonDescAsset* DescToSetWhenSafe; // 0x7a8(0x08)
	struct UCannonDescAsset* CurrentCannonDesc; // 0x7b0(0x08)
	float ServerPitch; // 0x7b8(0x04)
	float ServerYaw; // 0x7bc(0x04)
	struct UParticleSystemComponent* LoadedItemVFXComp; // 0x7c0(0x08)
	struct UStaticMesh* DefaultFuseMesh; // 0x7c8(0x08)
	char UnknownData_7D0[0x5d0]; // 0x7d0(0x5d0)
	char InteractionState; // 0xce0(0x01)
	char UnknownData_DA1[0xf]; // 0xda1(0x0f)

	void HandlePitchInput(float Pitch)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.HandlePitchInput");
		struct {
			float Pitch;
		} params;
		params.Pitch = Pitch;

		ProcessEvent(this, fn, &params);
	}

	void HandleYawInput(float Yaw)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.HandleYawInput");
		struct {
			float Yaw;
		} params;
		params.Yaw = Yaw;

		ProcessEvent(this, fn, &params);
	}

	void ForceAimCannon(float Pitch, float Yaw)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.ForceAimCannon");
		struct {
			float Pitch;
			float Yaw;
		} params;
		params.Pitch = Pitch;
		params.Yaw = Yaw;
		ProcessEvent(this, fn, &params);
	}

	bool IsReadyToFire() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.IsReadyToFire");
		bool is_ready = true;
		ProcessEvent(this, fn, &is_ready);
		return is_ready;
	}

	void Fire() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.Fire");
		ProcessEvent(this, fn, nullptr);
	}

	bool IsReadyToReload() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.IsReadyToReload");
		bool is_ready = true;
		ProcessEvent(this, fn, &is_ready);
		return is_ready;
	}

	void Reload() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.Reload");
		ProcessEvent(this, fn, nullptr);
	}

	void Server_Fire(float Pitch, float Yaw)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.Cannon.Server_Fire");
		struct {
			float Pitch;
			float Yaw;
		} params;
		params.Pitch = Pitch;
		params.Yaw = Yaw;
		ProcessEvent(this, fn, &params);
	}

};

// Class Athena.SlidingDoor
// Size: 0x540 (Inherited: 0x4f8)
struct ASlidingDoor {
	char pad_0x0[0x534];
	FVector InitialDoorMeshLocation; // 0x534(0x0c)

	void OpenDoor() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.SkeletonFortDoor.OpenDoor");
		ProcessEvent(this, fn, nullptr);
	}
};

// Class Athena.PuzzleVault
// Size: 0x14b0 (Inherited: 0x468)
struct APuzzleVault {
	char pad[0x1030];
	ASlidingDoor* OuterDoor; // 0x1030(0x08)
};

// ScriptStruct Athena.XMarksTheSpotMapMark
// Size: 0x10 (Inherited: 0x00)
struct FXMarksTheSpotMapMark {
	struct FVector2D Position; // 0x00(0x08)
	float Rotation; // 0x08(0x04)
	bool IsUnderground; // 0x0c(0x01)
	char UnknownData_D[0x3]; // 0x0d(0x03)
};

// Class Athena.XMarksTheSpotMap
// Size: 0x950 (Inherited: 0x830)
struct AXMarksTheSpotMap
{
	char pad1[0x830];
	int32_t CanvasWidth; // 0x830(0x04)
	int32_t CanvasHeight; // 0x834(0x04)
	struct UTexture* MarkTexture; // 0x838(0x08)
	struct UTexture* AltMarkTexture; // 0x840(0x08)
	struct UTexture* UndergroundMarkTexture; // 0x848(0x08)
	struct UTexture* AltUndergroundMarkTexture; // 0x850(0x08)
	float MarkWidthRatio; // 0x858(0x04)
	float MarkHeightRatio; // 0x85c(0x04)
	char MarkBlendMode; // 0x860(0x01)
	char UnknownData_861[0x7]; // 0x861(0x07)
	struct FString MapTexturePath; // 0x868(0x10)
	char MapInventoryTexturePath[0x10]; // 0x878(0x10)
	char UnknownData_888[0x70]; // 0x888(0x70)
	struct TArray<struct FXMarksTheSpotMapMark> Marks; // 0x8f8(0x10)
	char UnknownData_908[0x18]; // 0x908(0x18)
	float Rotation; // 0x920(0x04)
	char UnknownData_924[0x2c]; // 0x924(0x2c)
};

// Class Athena.WieldedItemComponent
// Size: 0x440 (Inherited: 0x2e0)
struct UWieldedItemComponent {
	char pad[0x2f0];
	ACharacter* CurrentlyWieldedItem; // 0x2f0(0x08)
};


// ScriptStruct Athena.AthenaCharacterSwimParams
// Size: 0x9c (Inherited: 0x00)
struct FAthenaCharacterSwimParams {
	float EnterSwimmingDepth; // 0x00(0x04)
	float ExitSwimmingDepth; // 0x04(0x04)
	float FloatHeight; // 0x08(0x04)
	float ThirdPersonMeshOffset; // 0x0c(0x04)
	float ThirdPersonMeshAdjustTime; // 0x10(0x04)
	float FullGravityHeight; // 0x14(0x04)
	float SurfaceSwimDepth; // 0x18(0x04)
	float SurfaceSwimExitDepth; // 0x1c(0x04)
	float SubmergedBuoyancyDepth; // 0x20(0x04)
	float ZeroDragDepth; // 0x24(0x04)
	float MaxBuoyancyAcceleration; // 0x28(0x04)
	float SubmergedBuoyancyAcceleration; // 0x2c(0x04)
	float SubmergedBuoyancyTime; // 0x30(0x04)
	float DragFactor; // 0x34(0x04)
	float MaxDragAcceleration; // 0x38(0x04)
	float MinPushDownRoofAngle; // 0x3c(0x04)
	float MinPushDownRoofZ; // 0x40(0x04)
	float MinCorrectionVelForSurfaceSwim; // 0x44(0x04)
	float SurfaceVelBlendTime; // 0x48(0x04)
	float UnderwaterSwimmingEntryPitch; // 0x4c(0x04)
	float UnderwaterSwimmingEntryAccel; // 0x50(0x04)
	float UnderwaterSwimDepth; // 0x54(0x04)
	float UnderwaterSwimmingDragSpeed; // 0x58(0x04)
	float UnderwaterSwimmingSpeed; // 0x5c(0x04)
	float PitchBlendOffRate; // 0x60(0x04)
	float MaxSwimDepth; // 0x64(0x04)
	float MaxSwimDepthBuoyancy; // 0x68(0x04)
	float MaxSwimDepthBuoyancyRange; // 0x6c(0x04)
	float MaxSwimDepthDragFactor; // 0x70(0x04)
	float CharacterCentreOffset; // 0x74(0x04)
	char WaterDepthTraceQueryType; // 0x78(0x01)
	char UnknownData_79[0x3]; // 0x79(0x03)
	char SurfaceSwimSpeeds[0x0c]; // 0x7c(0x0c)
	char UnderwaterSwimSpeeds[0x0c]; // 0x88(0x0c)
	float SurfaceSwimmingDepthWhenEnteredWater; // 0x94(0x04)
	char UnknownData_98[0x4]; // 0x98(0x04)
};

// Class Athena.AthenaCharacterMovementComponent
// Size: 0x860 (Inherited: 0x590)
struct UAthenaCharacterMovementComponent {
	char pad_00[0x590];
	char UnknownData_590[0x10]; // 0x590(0x10)
	struct FAthenaCharacterSwimParams SwimParams; // 0x5a0(0x9c)
	float SprintSpdAmp; // 0x63c(0x04)
	float SprintAccelAmp; // 0x640(0x04)
	float LookAtYawRate; // 0x644(0x04)
	float LookAtPitchRate; // 0x648(0x04)
	char UnknownData_64C[0x4]; // 0x64c(0x04)
	float MaxSpeedRatioWhenWalkingInNonSwimWater; // 0x650(0x04)
	float MaxAccelerationRatioWhenWalkingInNonSwimWater; // 0x654(0x04)
	char bCharacterCollisionSweepsEnabled : 1; // 0x658(0x01)
	char UnknownData_658_1 : 7; // 0x658(0x01)
	char UnknownData_659[0x3]; // 0x659(0x03)
	float CharacterCollisionRadius; // 0x65c(0x04)
	char CharacterCollisionProfile[0x8]; // 0x660(0x08)
	char CollisionPanicFallbackProfileName[0x8]; // 0x668(0x08)
	float CharacterCollisionWalkingFriction; // 0x670(0x04)
	float CharacterCollisionSwimmingFriction; // 0x674(0x04)
	float CharacterCollisionUnderwaterOffset; // 0x678(0x04)
	char UnknownData_67C[0x4]; // 0x67c(0x04)
	char UnderwaterMovement[0x8]; // 0x680(0x08)
	bool bCreateDisturbance; // 0x688(0x01)
	char UnknownData_689[0x3]; // 0x689(0x03)
	float DisturbanceSize; // 0x68c(0x04)
	float DisturbanceVelocityScale; // 0x690(0x04)
	bool UseAsRVOObstacleOnly; // 0x694(0x01)
	char UnknownData_695[0x63]; // 0x695(0x63)
	float FanPushOutAngle; // 0x6f8(0x04)
	int32_t FanPushOutNumSteps; // 0x6fc(0x04)
	float MaxFanPushOutScalar; // 0x700(0x04)
	char UnknownData_704[0x4]; // 0x704(0x04)
	char UnderwaterVelocityToAnglePitchProjectileHitVolume[0x8]; // 0x708(0x08)
	char UnderwaterVelocityToAngleRollProjectileHitVolume[0x8]; // 0x710(0x08)
	char UnderwaterVelocityToZOffsetProjectileHitVolume[0x8]; // 0x718(0x08)
	char OnLandVelocityToAnglePitchProjectileHitVolume[0x8]; // 0x720(0x08)
	char OnLandVelocityToXOffsetProjectileHitVolume[0x8]; // 0x728(0x08)
	char OnLandVelocityToYOffsetProjectileHitVolume[0x8]; // 0x730(0x08)
	char OnLandVelocityToZOffsetProjectileHitVolume[0x8]; // 0x738(0x08)
	char FallingVelocityToAnglePitchProjectileHitVolume[0x8]; // 0x740(0x08)
	char FallingVelocityToXOffsetProjectileHitVolume[0x8]; // 0x748(0x08)
	char FallingVelocityToZOffsetProjectileHitVolume[0x8]; // 0x750(0x08)
	char UnknownData_758[0xc8]; // 0x758(0xc8)
	struct FName NonUnderwaterCollisionProfileName; // 0x820(0x08)
	struct FName UnderwaterCollisionProfileName; // 0x828(0x08)
	char UnknownData_830[0x30]; // 0x830(0x30)

	void UnforceSwimmingClientTrustThreshold(); // Function Athena.AthenaCharacterMovementComponent.UnforceSwimmingClientTrustThreshold // Final|Native|Public|BlueprintCallable // @ game+0xda4200
	void SetMovementMode(char NewMovementMode, char NewCustomMode); // Function Athena.AthenaCharacterMovementComponent.SetMovementMode // RequiredAPI|Native|Public|BlueprintCallable // @ game+0xda2d80
	void SetErrorOnResolvePenetration(bool ErrorOnResolve); // Function Athena.AthenaCharacterMovementComponent.SetErrorOnResolvePenetration // Final|Native|Public|BlueprintCallable // @ game+0xda2690
	bool HasValidAckedMoveClientOnly(); // Function Athena.AthenaCharacterMovementComponent.HasValidAckedMoveClientOnly // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xd9f210
	float GetTimestampOfMostRecentPredictedMoveClientOnly(); // Function Athena.AthenaCharacterMovementComponent.GetTimestampOfMostRecentPredictedMoveClientOnly // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xd9ea00
	float GetTimestampOfLastAckedMoveClientOnly(); // Function Athena.AthenaCharacterMovementComponent.GetTimestampOfLastAckedMoveClientOnly // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xd9e9d0
	float GetTerminalVelocity(); // Function Athena.AthenaCharacterMovementComponent.GetTerminalVelocity // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xd9e960
	char GetMovementMode(); // Function Athena.AthenaCharacterMovementComponent.GetMovementMode // RequiredAPI|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xd9e310
	float GetMaxSprintSpeed(); // Function Athena.AthenaCharacterMovementComponent.GetMaxSprintSpeed // RequiredAPI|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xd9e260
	float GetMaxMoveSpeedScalar(); // Function Athena.AthenaCharacterMovementComponent.GetMaxMoveSpeedScalar // Final|RequiredAPI|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0xd9e230
	void ForceSwimmingClientTrustThreshold(float NewSwimmingClientTrustThreshold); // Function Athena.AthenaCharacterMovementComponent.ForceSwimmingClientTrustThreshold // Final|Native|Public|BlueprintCallable // @ game+0xd9b390
	void FindCurrentFloor(); // Function Athena.AthenaCharacterMovementComponent.FindCurrentFloor // Final|Native|Public|BlueprintCallable // @ game+0xd9b090
};
// Class Athena.AthenaCharacter
// Size: 0xc40 (Inherited: 0x5e0)
struct AAthenaCharacter : ACharacter {
	char pad[0x140];
	struct UAthenaCharacterMovementComponent* CharacterMovementComponent; // 0x720(0x08)
	char pad2[0x158];
	UWieldedItemComponent* WieldedItemComponent; // 0x880(0x08)
	char pad3[0x3b8];
};

// Class Athena.AthenaPlayerCharacter
// Size: 0x1ff0 (Inherited: 0xc40)
struct AAthenaPlayerCharacter : AAthenaCharacter {
	char pad[0x1d0];
	struct UDrowningComponent* DrowningComponent; // 0xe10(0x08)
	char pad2[0x11d8];

	ACharacter* GetWieldedItem() {
		if (!WieldedItemComponent) return nullptr;
		return WieldedItemComponent->CurrentlyWieldedItem;
	}

};

// Class Athena.DrowningComponent
// Size: 0x1c8 (Inherited: 0xc8)
struct UDrowningComponent {
	char pad[0x110];
	float OxygenLevel; // 0x110(0x04)
	char pad2[0xb4];

	float GetOxygenLevel() {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.DrowningComponent.GetOxygenLevel");
		float oxygen;
		ProcessEvent(this, fn, &oxygen);
		return oxygen;
	} // Function Athena.DrowningComponent.GetOxygenLevel // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x39cb270
};

struct AGameCustomizationService_SetTime_Params
{
	int Hours;

	void SetTime(int Hours)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.GameCustomizationService.SetTime");
		AGameCustomizationService_SetTime_Params params;
		params.Hours = Hours;
		ProcessEvent(this, fn, &params);
	}
};

// ScriptStruct Engine.MinimalViewInfo
// Size: 0x5a0 (Inherited: 0x00)
struct FMinimalViewInfo {
	FVector Location;
	FRotator Rotation;
	char UnknownData_18[0x10];
	//float FOV;
};

// ScriptStruct Engine.CameraCacheEntry
// Size: 0x5b0 (Inherited: 0x00)
struct FCameraCacheEntry {
	float TimeStamp;
	char pad[0xc];
	FMinimalViewInfo POV;
};

// ScriptStruct Engine.TViewTarget
// Size: 0x5c0 (Inherited: 0x00)
struct FTViewTarget
{
	class AActor* Target;                                                    // 0x0000(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	unsigned char                                      UnknownData_KHBN[0x8];                                     // 0x0008(0x0008) MISSED OFFSET (FIX SPACE BETWEEN PREVIOUS PROPERTY)
	struct FMinimalViewInfo                            POV;                                                       // 0x0010(0x05A0) (Edit, BlueprintVisible)
	class APlayerState* PlayerState;                                               // 0x05B0(0x0008) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, Protected, HasGetValueTypeHash)
	unsigned char                                      UnknownData_QIMB[0x8];                                     // 0x05B8(0x0008) MISSED OFFSET (PADDING)

};

// Class Engine.PlayerCameraManager
// Size: 0x1c60 (Inherited: 0x3c8)
struct APlayerCameraManager {
	char pad[0x440];
	FCameraCacheEntry CameraCache; // 0x440(0x5b0)
	FCameraCacheEntry LastFrameCameraCache;
	FTViewTarget ViewTarget;
	FTViewTarget PendingViewTarget;


	FVector GetCameraLocation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerCameraManager.GetCameraLocation");
		FVector location;
		ProcessEvent(this, fn, &location);
		return location;
	};
	FRotator GetCameraRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerCameraManager.GetCameraRotation");
		FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}
};

// ScriptStruct InputCore.Key
// Size: 0x20 (Inherited: 0x00)
struct FKey
{
	FName KeyName;
	unsigned char UnknownData00[0x18] = {};

	FKey() {};
	FKey(const char* InName) : KeyName(FName(InName)) {}
};

struct AController_K2_GetPawn_Params
{
	class APawn* ReturnValue;
};

struct AController {

	char pad_0000[0x3e0];
	class ACharacter* Character; // 0x3e0(0x08) // Class Engine.Controller
	char pad_0480[0x70];
	APlayerCameraManager* PlayerCameraManager; // 0x458(0x08) // Class Engine.PlayerController
	char pad_04f8[0x1309];
	bool IdleDisconnectEnabled; // 0x1769(0x01) // Class Athena.OnlineAthenaPlayerController

	void UnPossess()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Controller.UnPossess");
		ProcessEvent(this, fn, nullptr);
	}

	void SetTime(int Hours)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.GameCustomizationService.SetTime");
		AGameCustomizationService_SetTime_Params params;
		params.Hours = Hours;
		ProcessEvent(this, fn, &params);
	}

	void SendToConsole(FString& cmd) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.SendToConsole");
		ProcessEvent(this, fn, &cmd);
	}

	bool WasInputKeyJustPressed(const FKey& Key) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.WasInputKeyJustPressed");
		struct
		{
			FKey Key;

			bool ReturnValue = false;
		} params;

		params.Key = Key;
		ProcessEvent(this, fn, &params);

		return params.ReturnValue;
	}

	APawn* K2_GetPawn() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Controller.K2_GetPawn");
		AController_K2_GetPawn_Params params;
		class APawn* ReturnValue;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}

	bool ProjectWorldLocationToScreen(const FVector& WorldLocation, FVector2D& ScreenLocation) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.ProjectWorldLocationToScreen");
		struct
		{
			FVector WorldLocation;
			FVector2D ScreenLocation;
			bool ReturnValue = false;
		} params;

		params.WorldLocation = WorldLocation;
		ProcessEvent(this, fn, &params);
		ScreenLocation = params.ScreenLocation;
		return params.ReturnValue;
	}

	FRotator GetControlRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Pawn.GetControlRotation");
		struct FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}

	FRotator GetDesiredRotation() {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Pawn.GetDesiredRotation");
		struct FRotator rotation;
		ProcessEvent(this, fn, &rotation);
		return rotation;
	}

	void AddYawInput(float Val) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.AddYawInput");
		ProcessEvent(this, fn, &Val);
	}

	void AddPitchInput(float Val) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.AddPitchInput");
		ProcessEvent(this, fn, &Val);
	}

	void FOV(float NewFOV) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.FOV");
		ProcessEvent(this, fn, &NewFOV);
	}

	bool LineOfSightTo(ACharacter* Other, const FVector& ViewPoint, const bool bAlternateChecks) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.Controller.LineOfSightTo");
		struct {
			ACharacter* Other = nullptr;
			FVector ViewPoint;
			bool bAlternateChecks = false;
			bool ReturnValue = false;
		} params;
		params.Other = Other;
		params.ViewPoint = ViewPoint;
		params.bAlternateChecks = bAlternateChecks;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}
};

// Class Engine.PlayerController
// Size: 0x770 (Inherited: 0x428)
struct APlayerController : AController {
	struct UPlayer* Player; // 0x428(0x08)
	char UnknownData_430[0x8]; // 0x430(0x08)
	struct APawn* AcknowledgedPawn; // 0x438(0x08)
	struct UInterpTrackInstDirector* ControllingDirTrackInst; // 0x440(0x08)
	char UnknownData_448[0x8]; // 0x448(0x08)
	struct AHUD* MyHUD; // 0x450(0x08)
	struct APlayerCameraManager* PlayerCameraManager; // 0x458(0x08)
	struct UClass* PlayerCameraManagerClass; // 0x460(0x08)
	bool bAutoManageActiveCameraTarget; // 0x468(0x01)
	char UnknownData_469[0x3]; // 0x469(0x03)
	struct FRotator TargetViewRotation; // 0x46c(0x0c)
	char UnknownData_478[0x10]; // 0x478(0x10)
	struct TArray<struct AActor*> HiddenActors; // 0x488(0x10)
	float LastSpectatorStateSynchTime; // 0x498(0x04)
	struct FVector LastSpectatorSyncLocation; // 0x49c(0x0c)
	struct FRotator LastSpectatorSyncRotation; // 0x4a8(0x0c)
	int32_t ClientCap; // 0x4b4(0x04)
	struct UCheatManager* CheatManager; // 0x4b8(0x08)
	struct UClass* CheatClass; // 0x4c0(0x08)
	struct UPlayerInput* PlayerInput; // 0x4c8(0x08)
	struct TArray<struct FActiveForceFeedbackEffect> ActiveForceFeedbackEffects; // 0x4d0(0x10)
	char UnknownData_4E0[0x90]; // 0x4e0(0x90)
	char UnknownData_570_0 : 2; // 0x570(0x01)
	char bPlayerIsWaiting : 1; // 0x570(0x01)
	char UnknownData_570_3 : 5; // 0x570(0x01)
	char UnknownData_571[0x3]; // 0x571(0x03)
	char NetPlayerIndex; // 0x574(0x01)
	bool bHasVoiceHandshakeCompleted; // 0x575(0x01)
	char UnknownData_576[0x2]; // 0x576(0x02)
	struct UNetConnection* PendingSwapConnection; // 0x578(0x08)
	struct UNetConnection* NetConnection; // 0x580(0x08)
	char UnknownData_588[0xc]; // 0x588(0x0c)
	float InputYawScale; // 0x594(0x04)
	float InputPitchScale; // 0x598(0x04)
	float InputRollScale; // 0x59c(0x04)
	char bShowMouseCursor : 1; // 0x5a0(0x01)
	char bEnableClickEvents : 1; // 0x5a0(0x01)
	char bEnableTouchEvents : 1; // 0x5a0(0x01)
	char bEnableMouseOverEvents : 1; // 0x5a0(0x01)
	char bEnableTouchOverEvents : 1; // 0x5a0(0x01)
	char bForceFeedbackEnabled : 1; // 0x5a0(0x01)
	char UnknownData_5A0_6 : 2; // 0x5a0(0x01)
	char UnknownData_5A1[0x3]; // 0x5a1(0x03)
	char DefaultMouseCursor; // 0x5a4(0x01)
	char CurrentMouseCursor; // 0x5a5(0x01)
	char DefaultClickTraceChannel; // 0x5a6(0x01)
	char CurrentClickTraceChannel; // 0x5a7(0x01)
	float HitResultTraceDistance; // 0x5a8(0x04)
	float ForceFeedbackIntensity; // 0x5ac(0x04)
	char UnknownData_5B0[0x88]; // 0x5b0(0x88)
	struct UInputComponent* InactiveStateInputComponent; // 0x638(0x08)
	char UnknownData_640[0x20]; // 0x640(0x20)
	struct UTouchInterface* CurrentTouchInterface; // 0x660(0x08)
	char UnknownData_668[0x30]; // 0x668(0x30)
	struct ASpectatorPawn* SpectatorPawn; // 0x698(0x08)
	struct FVector SpawnLocation; // 0x6a0(0x0c)
	char UnknownData_6AC[0x4]; // 0x6ac(0x04)
	char UnknownData_6B0[0x50]; // 0x6b0(0x50)
	char UnknownData_700[0x14]; // 0x700(0x14)
	bool bIsLocalPlayerController; // 0x714(0x01)
	char UnknownData_715[0x1]; // 0x715(0x01)
	uint16_t SeamlessTravelCount; // 0x716(0x02)
	uint16_t LastCompletedSeamlessTravelCount; // 0x718(0x02)
	bool bInCinematicMode; // 0x71a(0x01)
	char UnknownData_71B[0x55]; // 0x71b(0x55)

	bool WasInputKeyJustReleased(struct FKey Key); // Function Engine.PlayerController.WasInputKeyJustReleased // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b33810
	bool WasInputKeyJustPressed(struct FKey Key); // Function Engine.PlayerController.WasInputKeyJustPressed // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b33710
	void ToggleSpeaking(bool bInSpeaking); // Function Engine.PlayerController.ToggleSpeaking // Exec|Native|Public // @ game+0x2b33340
	void SwitchLevel(struct FString URL); // Function Engine.PlayerController.SwitchLevel // Exec|Native|Public // @ game+0x2b33250
	void StopHapticEffect(char Hand); // Function Engine.PlayerController.StopHapticEffect // Final|Native|Public|BlueprintCallable // @ game+0x2b331b0
	void StartFire(char FireModeNum); // Function Engine.PlayerController.StartFire // Exec|Native|Public // @ game+0x2b32d80
	void SetVirtualJoystickVisibility(bool bVisible); // Function Engine.PlayerController.SetVirtualJoystickVisibility // Native|Public|BlueprintCallable // @ game+0x2b326a0
	void SetViewTargetWithBlend(struct AActor* NewViewTarget, float BlendTime, char BlendFunc, float BlendExp, bool bLockOutgoing); // Function Engine.PlayerController.SetViewTargetWithBlend // Native|Public|BlueprintCallable // @ game+0x2b32520
	void SetName(struct FString S); // Function Engine.PlayerController.SetName // Exec|Native|Public // @ game+0x2b31ea0
	void SetIgnoreMoveInput(bool bNewMoveInput); // Function Engine.PlayerController.SetIgnoreMoveInput // Native|Public|BlueprintCallable // @ game+0x2b319e0
	void SetIgnoreLookInput(bool bNewLookInput); // Function Engine.PlayerController.SetIgnoreLookInput // Native|Public|BlueprintCallable // @ game+0x2b31950
	void SetHapticsByValue(float Frequency, float Amplitude, char Hand); // Function Engine.PlayerController.SetHapticsByValue // Final|Native|Public|BlueprintCallable // @ game+0x2b31770
	void SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning); // Function Engine.PlayerController.SetCinematicMode // Native|Public|BlueprintCallable // @ game+0x2b31290
	void SetAudioListenerOverride(struct USceneComponent* AttachToComponent, struct FVector Location, struct FRotator Rotation); // Function Engine.PlayerController.SetAudioListenerOverride // Final|Native|Public|HasDefaults|BlueprintCallable // @ game+0x2b30ff0
	void ServerViewSelf(struct FViewTargetTransitionParams TransitionParams); // Function Engine.PlayerController.ServerViewSelf // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b30860
	void ServerViewPrevPlayer(); // Function Engine.PlayerController.ServerViewPrevPlayer // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b30810
	void ServerViewNextPlayer(); // Function Engine.PlayerController.ServerViewNextPlayer // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b307c0
	void ServerVerifyViewTarget(); // Function Engine.PlayerController.ServerVerifyViewTarget // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30770
	void ServerUpdateCamera(struct FVector_NetQuantize CamLoc, int32_t CamPitchAndYaw); // Function Engine.PlayerController.ServerUpdateCamera // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b30650
	void ServerUnmutePlayer(struct FUniqueNetIdRepl PlayerId); // Function Engine.PlayerController.ServerUnmutePlayer // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30500
	void ServerToggleAILogging(); // Function Engine.PlayerController.ServerToggleAILogging // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b304b0
	void ServerShortTimeout(); // Function Engine.PlayerController.ServerShortTimeout // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30460
	void ServerSetSpectatorWaiting(bool bWaiting); // Function Engine.PlayerController.ServerSetSpectatorWaiting // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b303a0
	void ServerSetSpectatorLocation(struct FVector NewLoc, struct FRotator NewRot); // Function Engine.PlayerController.ServerSetSpectatorLocation // Net|Native|Event|Public|NetServer|HasDefaults|NetValidate // @ game+0x2b30270
	void ServerRestartPlayer(); // Function Engine.PlayerController.ServerRestartPlayer // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30220
	void ServerPause(); // Function Engine.PlayerController.ServerPause // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b301d0
	void ServerNotifyLoadedWorld(struct FName WorldPackageName); // Function Engine.PlayerController.ServerNotifyLoadedWorld // Final|Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b30120
	void ServerMutePlayer(struct FUniqueNetIdRepl PlayerId); // Function Engine.PlayerController.ServerMutePlayer // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2ffd0
	void ServerCheckClientPossessionReliable(); // Function Engine.PlayerController.ServerCheckClientPossessionReliable // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2ff80
	void ServerCheckClientPossession(); // Function Engine.PlayerController.ServerCheckClientPossession // Net|Native|Event|Public|NetServer|NetValidate // @ game+0x2b2ff30
	void ServerChangeName(struct FString S); // Function Engine.PlayerController.ServerChangeName // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2fe60
	void ServerCamera(struct FName NewMode); // Function Engine.PlayerController.ServerCamera // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2fda0
	void ServerAcknowledgePossession(struct APawn* P); // Function Engine.PlayerController.ServerAcknowledgePossession // Net|NetReliableNative|Event|Public|NetServer|NetValidate // @ game+0x2b2fce0
	void SendToConsole(struct FString Command); // Function Engine.PlayerController.SendToConsole // Exec|Native|Public // @ game+0x2b2fc30
	void RestartLevel(); // Function Engine.PlayerController.RestartLevel // Exec|Native|Public // @ game+0x2b2fc10
	void ResetIgnoreMoveInput(); // Function Engine.PlayerController.ResetIgnoreMoveInput // Native|Public|BlueprintCallable // @ game+0x2b2fbd0
	void ResetIgnoreLookInput(); // Function Engine.PlayerController.ResetIgnoreLookInput // Native|Public|BlueprintCallable // @ game+0x2b2fbb0
	void ResetIgnoreInputFlags(); // Function Engine.PlayerController.ResetIgnoreInputFlags // Native|Public|BlueprintCallable // @ game+0x2b2fb90
	bool ProjectWorldLocationToScreen(const FVector& WorldLocation, FVector2D& ScreenLocation) // Function Engine.PlayerController.ProjectWorldLocationToScreen // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2f450
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.PlayerController.ProjectWorldLocationToScreen");
		struct
		{
			FVector WorldLocation;
			FVector2D ScreenLocation;
			bool ReturnValue = false;
		} params;

		params.WorldLocation = WorldLocation;
		ProcessEvent(this, fn, &params);
		ScreenLocation = params.ScreenLocation;
		return params.ReturnValue;
	}
	void PlayHapticEffect(struct UHapticFeedbackEffect* HapticEffect, char Hand, float Scale); // Function Engine.PlayerController.PlayHapticEffect // Final|Native|Public|BlueprintCallable // @ game+0x2b2f2b0
	void PlayDynamicForceFeedback(float Intensity, float Duration, bool bAffectsLeftLarge, bool bAffectsLeftSmall, bool bAffectsRightLarge, bool bAffectsRightSmall, char Action, struct FLatentActionInfo LatentInfo); // Function Engine.PlayerController.PlayDynamicForceFeedback // Final|Native|Public|BlueprintCallable // @ game+0x2b2f020
	void Pause(); // Function Engine.PlayerController.Pause // Exec|Native|Public // @ game+0x2b2eaf0
	void OnServerStartedVisualLogger(bool bIsLogging); // Function Engine.PlayerController.OnServerStartedVisualLogger // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b2ea60
	void LocalTravel(struct FString URL); // Function Engine.PlayerController.LocalTravel // Exec|Native|Public // @ game+0x2b2e5e0
	bool IsSeamlessTravelInProgress(); // Function Engine.PlayerController.IsSeamlessTravelInProgress // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a930
	bool IsPossessingServerAcknowledgedPawn(); // Function Engine.PlayerController.IsPossessingServerAcknowledgedPawn // Final|Native|Public|BlueprintCallable|BlueprintPure // @ game+0x2b2a8d0
	bool IsMoveInputIgnored(); // Function Engine.PlayerController.IsMoveInputIgnored // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a810
	bool IsLookInputIgnored(); // Function Engine.PlayerController.IsLookInputIgnored // Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a7e0
	bool IsInputKeyDown(struct FKey Key); // Function Engine.PlayerController.IsInputKeyDown // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a680
	void GetViewportSize(int32_t SizeX, int32_t SizeY); // Function Engine.PlayerController.GetViewportSize // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b2a290
	struct ASpectatorPawn* GetSpectatorPawn(); // Function Engine.PlayerController.GetSpectatorPawn // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b29d20
	bool GetMousePosition(float LocationX, float LocationY); // Function Engine.PlayerController.GetMousePosition // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b29500
	struct FVector GetInputVectorKeyState(struct FKey Key); // Function Engine.PlayerController.GetInputVectorKeyState // Final|Native|Public|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b292b0
	void GetInputTouchState(char FingerIndex, float LocationX, float LocationY, bool bIsCurrentlyPressed); // Function Engine.PlayerController.GetInputTouchState // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b29010
	void GetInputMouseDelta(float DeltaX, float DeltaY); // Function Engine.PlayerController.GetInputMouseDelta // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28f30
	void GetInputMotionState(struct FVector Tilt, struct FVector RotationRate, struct FVector Gravity, struct FVector Acceleration); // Function Engine.PlayerController.GetInputMotionState // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28da0
	float GetInputKeyTimeDown(struct FKey Key); // Function Engine.PlayerController.GetInputKeyTimeDown // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28ca0
	void GetInputAnalogStickState(char WhichStick, float StickX, float StickY); // Function Engine.PlayerController.GetInputAnalogStickState // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b289e0
	float GetInputAnalogKeyState(struct FKey Key); // Function Engine.PlayerController.GetInputAnalogKeyState // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b288e0
	struct AHUD* GetHUD(); // Function Engine.PlayerController.GetHUD // Final|Native|Public|BlueprintCallable|BlueprintPure|Const // @ game+0x2b27e10
	bool GetHitResultUnderFingerForObjects(char FingerIndex, struct TArray<char> ObjectTypes, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderFingerForObjects // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b285d0
	bool GetHitResultUnderFingerByChannel(char FingerIndex, char TraceChannel, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderFingerByChannel // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28430
	bool GetHitResultUnderFinger(char FingerIndex, char TraceChannel, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderFinger // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28290
	bool GetHitResultUnderCursorForObjects(struct TArray<char> ObjectTypes, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderCursorForObjects // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b28100
	bool GetHitResultUnderCursorByChannel(char TraceChannel, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderCursorByChannel // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b27fa0
	bool GetHitResultUnderCursor(char TraceChannel, bool bTraceComplex, struct FHitResult HitResult); // Function Engine.PlayerController.GetHitResultUnderCursor // Final|Native|Public|HasOutParms|BlueprintCallable|BlueprintPure|Const // @ game+0x2b27e40
	struct FVector GetFocalLocation(); // Function Engine.PlayerController.GetFocalLocation // Native|Public|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b27d90
	void EnableCheats(); // Function Engine.PlayerController.EnableCheats // Exec|Native|Public // @ game+0x2b265a0
	bool DeprojectScreenPositionToWorld(float ScreenX, float ScreenY, struct FVector WorldLocation, struct FVector WorldDirection); // Function Engine.PlayerController.DeprojectScreenPositionToWorld // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b26190
	bool DeprojectMousePositionToWorld(struct FVector WorldLocation, struct FVector WorldDirection); // Function Engine.PlayerController.DeprojectMousePositionToWorld // Final|Native|Public|HasOutParms|HasDefaults|BlueprintCallable|BlueprintPure|Const // @ game+0x2b26090
	void ConsoleKey(struct FKey Key); // Function Engine.PlayerController.ConsoleKey // Exec|Native|Public // @ game+0x2b25e50
	void ClientWasKicked(struct FText KickReason); // Function Engine.PlayerController.ClientWasKicked // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25d20
	void ClientVoiceHandshakeComplete(); // Function Engine.PlayerController.ClientVoiceHandshakeComplete // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25d00
	void ClientUpdateLevelStreamingStatusBatched(struct FString PackageBasePath, struct TArray<struct FLevelStreamingStatusUpdateInfo> LevelStreamingStatusUpdateInfo); // Function Engine.PlayerController.ClientUpdateLevelStreamingStatusBatched // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25bd0
	void ClientUpdateLevelStreamingStatus(struct FName PackageName, bool bNewShouldBeLoaded, bool bNewShouldBeVisible, bool bNewShouldBlockOnLoad, int32_t LODIndex); // Function Engine.PlayerController.ClientUpdateLevelStreamingStatus // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25a20
	void ClientUnmutePlayer(struct FUniqueNetIdRepl PlayerId); // Function Engine.PlayerController.ClientUnmutePlayer // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25930
	void ClientTravelInternal(struct FString URL, char TravelType, bool bSeamless, struct FGuid MapPackageGuid); // Function Engine.PlayerController.ClientTravelInternal // Net|NetReliableNative|Event|Public|HasDefaults|NetClient // @ game+0x2b257b0
	void ClientTravel(struct FString URL, char TravelType, bool bSeamless, struct FGuid MapPackageGuid); // Function Engine.PlayerController.ClientTravel // Final|Native|Public|HasDefaults // @ game+0x2b25630
	void ClientTeamMessage(struct APlayerState* SenderPlayerState, struct FString S, struct FName Type, float MsgLifeTime); // Function Engine.PlayerController.ClientTeamMessage // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b254c0
	void ClientStopForceFeedback(struct UForceFeedbackEffect* ForceFeedbackEffect, struct FName Tag); // Function Engine.PlayerController.ClientStopForceFeedback // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b253f0
	void ClientStopCameraShake(struct UClass* Shake); // Function Engine.PlayerController.ClientStopCameraShake // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b25360
	void ClientStopCameraAnim(struct UCameraAnim* AnimToStop); // Function Engine.PlayerController.ClientStopCameraAnim // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b252d0
	void ClientStartOnlineSession(); // Function Engine.PlayerController.ClientStartOnlineSession // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b252b0
	void ClientSpawnCameraLensEffect(struct UClass* LensEffectEmitterClass); // Function Engine.PlayerController.ClientSpawnCameraLensEffect // Net|Native|Event|Public|NetClient|BlueprintCallable // @ game+0x2b25220
	void ClientSetViewTarget(struct AActor* A, struct FViewTargetTransitionParams TransitionParams); // Function Engine.PlayerController.ClientSetViewTarget // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25100
	void ClientSetSpectatorWaiting(bool bWaiting); // Function Engine.PlayerController.ClientSetSpectatorWaiting // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b25070
	void ClientSetHUD(struct UClass* NewHUDClass); // Function Engine.PlayerController.ClientSetHUD // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b24e20
	void ClientSetForceMipLevelsToBeResident(struct UMaterialInterface* Material, float ForceDuration, int32_t CinematicTextureGroups); // Function Engine.PlayerController.ClientSetForceMipLevelsToBeResident // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24d20
	void ClientSetCinematicMode(bool bInCinematicMode, bool bAffectsMovement, bool bAffectsTurning, bool bAffectsHUD); // Function Engine.PlayerController.ClientSetCinematicMode // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24bb0
	void ClientSetCameraMode(struct FName NewCamMode); // Function Engine.PlayerController.ClientSetCameraMode // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24b20
	void ClientSetCameraFade(bool bEnableFading, struct FColor FadeColor, struct FVector2D FadeAlpha, float FadeTime, bool bFadeAudio); // Function Engine.PlayerController.ClientSetCameraFade // Net|NetReliableNative|Event|Public|HasDefaults|NetClient // @ game+0x2b24990
	void ClientSetBlockOnAsyncLoading(); // Function Engine.PlayerController.ClientSetBlockOnAsyncLoading // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24970
	void ClientSeamlessTravelComplete(); // Function Engine.PlayerController.ClientSeamlessTravelComplete // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24950
	void ClientReturnToMainMenu(struct FString ReturnReason); // Function Engine.PlayerController.ClientReturnToMainMenu // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b248a0
	void ClientRetryClientRestart(struct APawn* NewPawn); // Function Engine.PlayerController.ClientRetryClientRestart // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24810
	void ClientRestart(struct APawn* NewPawn); // Function Engine.PlayerController.ClientRestart // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24780
	void ClientReset(); // Function Engine.PlayerController.ClientReset // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24760
	void ClientRepObjRef(struct UObject* Object); // Function Engine.PlayerController.ClientRepObjRef // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b246d0
	void ClientReceiveLocalizedMessage(struct UClass* Message, int32_t Switch, struct APlayerState* RelatedPlayerState_2, struct APlayerState* RelatedPlayerState_3, struct UObject* OptionalObject); // Function Engine.PlayerController.ClientReceiveLocalizedMessage // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24550
	void ClientPrestreamTextures(struct AActor* ForcedActor, float ForceDuration, bool bEnableStreaming, int32_t CinematicTextureGroups); // Function Engine.PlayerController.ClientPrestreamTextures // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b24400
	void ClientPrepareMapChange(struct FName LevelName, bool bFirst, bool bLast); // Function Engine.PlayerController.ClientPrepareMapChange // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b242f0
	void ClientPlaySoundAtLocation(struct USoundBase* Sound, struct FVector Location, float VolumeMultiplier, float PitchMultiplier); // Function Engine.PlayerController.ClientPlaySoundAtLocation // Net|Native|Event|Public|HasDefaults|NetClient // @ game+0x2b241a0
	void ClientPlaySound(struct USoundBase* Sound, float VolumeMultiplier, float PitchMultiplier); // Function Engine.PlayerController.ClientPlaySound // Net|Native|Event|Public|NetClient // @ game+0x2b240a0
	void ClientPlayForceFeedback(struct UForceFeedbackEffect* ForceFeedbackEffect, bool bLooping, struct FName Tag); // Function Engine.PlayerController.ClientPlayForceFeedback // Net|Native|Event|Public|NetClient|BlueprintCallable // @ game+0x2b23f90
	void ClientPlayCameraShake(struct UClass* Shake, float Scale, char PlaySpace, struct FRotator UserPlaySpaceRot); // Function Engine.PlayerController.ClientPlayCameraShake // Net|Native|Event|Public|HasDefaults|NetClient|BlueprintCallable // @ game+0x2b23e30
	void ClientPlayCameraAnim(struct UCameraAnim* AnimToPlay, float Scale, float Rate, float BlendInTime, float BlendOutTime, bool bLoop, bool bRandomStartTime, char Space, struct FRotator CustomPlaySpace); // Function Engine.PlayerController.ClientPlayCameraAnim // Net|Native|Event|Public|HasDefaults|NetClient|BlueprintCallable // @ game+0x2b23b90
	void ClientMutePlayer(struct FUniqueNetIdRepl PlayerId); // Function Engine.PlayerController.ClientMutePlayer // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23aa0
	void ClientMessage(struct FString S, struct FName Type, float MsgLifeTime); // Function Engine.PlayerController.ClientMessage // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23970
	void ClientIgnoreMoveInput(bool bIgnore); // Function Engine.PlayerController.ClientIgnoreMoveInput // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b238e0
	void ClientIgnoreLookInput(bool bIgnore); // Function Engine.PlayerController.ClientIgnoreLookInput // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23850
	void ClientGotoState(struct FName NewState); // Function Engine.PlayerController.ClientGotoState // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b237c0
	void ClientGameEnded(struct AActor* EndGameFocus, bool bIsWinner); // Function Engine.PlayerController.ClientGameEnded // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b236f0
	void ClientForceGarbageCollection(); // Function Engine.PlayerController.ClientForceGarbageCollection // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b236d0
	void ClientFlushLevelStreaming(); // Function Engine.PlayerController.ClientFlushLevelStreaming // Final|Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b236b0
	void ClientEndOnlineSession(); // Function Engine.PlayerController.ClientEndOnlineSession // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23690
	void ClientEnableNetworkVoice(bool bEnable); // Function Engine.PlayerController.ClientEnableNetworkVoice // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23600
	void ClientCommitMapChange(); // Function Engine.PlayerController.ClientCommitMapChange // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b235e0
	void ClientClearCameraLensEffects(); // Function Engine.PlayerController.ClientClearCameraLensEffects // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b235c0
	void ClientCapBandwidth(int32_t Cap); // Function Engine.PlayerController.ClientCapBandwidth // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23530
	void ClientCancelPendingMapChange(); // Function Engine.PlayerController.ClientCancelPendingMapChange // Net|NetReliableNative|Event|Public|NetClient // @ game+0x2b23510
	void ClientBlendOutCameraShake(struct UClass* Shake); // Function Engine.PlayerController.ClientBlendOutCameraShake // Net|NetReliableNative|Event|Public|NetClient|BlueprintCallable // @ game+0x2b23480
	void ClientAddTextureStreamingLoc(struct FVector InLoc, float Duration, bool bOverrideLocation); // Function Engine.PlayerController.ClientAddTextureStreamingLoc // Final|Net|NetReliableNative|Event|Public|HasDefaults|NetClient // @ game+0x2b23360
	void ClearAudioListenerOverride(); // Function Engine.PlayerController.ClearAudioListenerOverride // Final|Native|Public|BlueprintCallable // @ game+0x2b23320
	void Camera(struct FName NewMode); // Function Engine.PlayerController.Camera // Exec|Native|Public // @ game+0x2b23260
	void ActivateTouchInterface(struct UTouchInterface* NewTouchInterface); // Function Engine.PlayerController.ActivateTouchInterface // Native|Public|BlueprintCallable // @ game+0x2b22540
};

/*
// Class Engine.Level
// Size: 0x300 (Inherited: 0x28)
struct ULevel : UObject {
	unsigned char UnknownData00[0x78]; // 0x0028(0x0078) MISSED OFFSET
	TArray<class AActor*> AActors; // 0x00A0(0x0010)
	unsigned char UnknownData10[0x18]; // 0x00B0(0x0018) MISSED OFFSET
	struct ULevelActorContainer* ActorCluster; // 0xc8(0x08)
	struct UWorld* OwningWorld; // 0xd0(0x08)
	struct UModel* Model; // 0xd8(0x08)
	struct TArray<struct UModelComponent*> ModelComponents; // 0xe0(0x10)
	struct ALevelScriptActor* LevelScriptActor; // 0xf0(0x08)
	struct ANavigationObjectBase* NavListStart; // 0xf8(0x08)
	struct ANavigationObjectBase* NavListEnd; // 0x100(0x08)
	struct TArray<struct UNavigationDataChunk*> NavDataChunks; // 0x108(0x10)
	float LightmapTotalSize; // 0x118(0x04)
	float ShadowmapTotalSize; // 0x11c(0x04)
	struct TArray<struct FVector> StaticNavigableGeometry; // 0x120(0x10)
	char UnknownData_130[0x130]; // 0x130(0x130)
	bool LevelVisibility; // 0x260(0x01)
	char UnknownData_261[0xf]; // 0x261(0x0f)
	char UnknownData_270_0 : 3; // 0x270(0x01)
	char Blocked : 1; // 0x270(0x01)
	char UnknownData_270_4 : 4; // 0x270(0x01)
	char UnknownData_271[0x67]; // 0x271(0x67)
	struct TArray<struct UAssetUserData*> AssetUserData; // 0x2d8(0x10)
	char UnknownData_2E8[0x18]; // 0x2e8(0x18)
};*/

struct ULevel
{
	char UnkownData[0xa0];
	TArray<class ACharacter*> AActors; // 0x00A0(0x0010)
};

// Class Engine.Player
// Size: 0x48 (Inherited: 0x28)
struct UPlayer {
	char UnknownData00[0x30];
	APlayerController* PlayerController;
};

// Class Engine.LocalPlayer
// Size: 0x210 (Inherited: 0x48)
struct ULocalPlayer : UPlayer {
	char UnknownData_48[0x18]; // 0x48(0x18)
	struct UGameViewportClient* ViewportClient; // 0x60(0x08)
	char UnknownData_68[0x40]; // 0x68(0x40)
	char AspectRatioAxisConstraint; // 0xa8(0x01)
	char UnknownData_A9[0x7]; // 0xa9(0x07)
	struct UClass* PendingLevelPlayerControllerClass; // 0xb0(0x08)
	char bSentSplitJoin : 1; // 0xb8(0x01)
	char UnknownData_B8_1 : 7; // 0xb8(0x01)
	char UnknownData_B9[0x3]; // 0xb9(0x03)
	float MinimumAspectRatio; // 0xbc(0x04)
	char UnknownData_C0[0x150]; // 0xc0(0x150)
};

// Class Engine.GameInstance
// Size: 0x88 (Inherited: 0x28)
struct UGameInstance {
	char UnknownData00[0x38];
	TArray<ULocalPlayer*> LocalPlayers; // 0x38
};

// Class Engine.World
// Size: 0x7e0 (Inherited: 0x28)
struct UWorld : UObject {
	static inline UWorld** GWorld = nullptr;
	char UnknownData_28[0x8]; // 0x28(0x08)
	struct ULevel* PersistentLevel; // 0x30(0x08)
	char pad3[0x20];
	AAthenaGameState* GameState; // 0x58(0x08)
	char pad4[0xF0];
	TArray<ULevel*> Levels; // 0x150(0x10)
	char pad5[0x50];
	ULevel* CurrentLevel; // 0x1b0(0x08)
	char pad6[0x8];
	UGameInstance* GameInstance; // 0x1c0(0x08)
};


// Class CoreUObject.Interface
// Size: 0x28 (Inherited: 0x28)
struct UInterface : UObject {
};

// Class CoreUObject.GCObjectReferencer
// Size: 0x60 (Inherited: 0x28)
struct UGCObjectReferencer : UObject {
	char UnknownData_28[0x38]; // 0x28(0x38)
};

// Class CoreUObject.TextBuffer
// Size: 0x50 (Inherited: 0x28)
struct UTextBuffer : UObject {
	char UnknownData_28[0x28]; // 0x28(0x28)
};

// Class CoreUObject.Field
// Size: 0x30 (Inherited: 0x28)
struct UField : UObject {
	char UnknownData_28[0x8]; // 0x28(0x08)
};

// Class CoreUObject.Struct
// Size: 0x88 (Inherited: 0x30)
struct UStruct : UField {
	UStruct* SuperField;
	char UnknownData_30[0x58]; // 0x30(0x58)
};

// Class CoreUObject.ScriptStruct
// Size: 0x98 (Inherited: 0x88)
struct UScriptStruct : UStruct {
	char UnknownData_88[0x10]; // 0x88(0x10)
};

// ScriptStruct AthenaEngine.SerialisedRpc
// Size: 0x20 (Inherited: 0x00)
struct FSerialisedRpc {
	char UnknownData_0[0x18]; // 0x00(0x18)
	class UScriptStruct* ContentsType; // 0x18(0x08)
};

// Class CoreUObject.Package
// Size: 0x80 (Inherited: 0x28)
struct UPackage : UObject {
	char UnknownData_28[0x58]; // 0x28(0x58)
};

// Class CoreUObject.Class
// Size: 0x1c0 (Inherited: 0x88)
class UClass : public UStruct
{
public:
	using UStruct::UStruct;
	unsigned char                                      UnknownData00[0x138];                                     // 0x0088(0x0138) MISSED OFFSET

	template<typename T>
	inline T* CreateDefaultObject()
	{
		return static_cast<T*>(CreateDefaultObject());
	}


	inline UObject* CreateDefaultObject()
	{
		return GetVFunction<UObject * (*)(UClass*)>(this, 88)(this);
	}
};

// Class Engine.ScriptViewportClient
// Size: 0x30 (Inherited: 0x28)
struct UScriptViewportClient : UObject {
	char UnknownData_28[0x8]; // 0x28(0x08)
};

// Class Engine.GameViewportClient
// Size: 0x250 (Inherited: 0x30)
struct UGameViewportClient : UScriptViewportClient {
	char UnknownData_30[0x8]; // 0x30(0x08)
	struct UConsole* ViewportConsole; // 0x38(0x08)
	char DebugProperties[0x10]; // 0x40(0x10)
	char UnknownData_50[0x30]; // 0x50(0x30)
	struct UWorld* World; // 0x80(0x08)
	struct UGameInstance* GameInstance; // 0x88(0x08)
	char UnknownData_90[0x1c0]; // 0x90(0x1c0)

	void SSSwapControllers(); // Function Engine.GameViewportClient.SSSwapControllers // Exec|Native|Public // @ game+0x2fe5ca0
	void ShowTitleSafeArea(); // Function Engine.GameViewportClient.ShowTitleSafeArea // Exec|Native|Public // @ game+0x2fe5d50
	void SetConsoleTarget(int32_t PlayerIndex); // Function Engine.GameViewportClient.SetConsoleTarget // Exec|Native|Public // @ game+0x2fe5cc0
};

// Class Athena.AthenaGameViewportClient
// Size: 0x260 (Inherited: 0x250)
struct UAthenaGameViewportClient : UGameViewportClient {
	char UnknownData_250[0x10]; // 0x250(0x10)
};

// Class CoreUObject.Function
// Size: 0xb8 (Inherited: 0x88)
struct UFunction : UStruct {
	char UnknownData_88[0x30]; // 0x88(0x30)
};

// Class CoreUObject.DelegateFunction
// Size: 0xb8 (Inherited: 0xb8)
struct UDelegateFunction : UFunction {
};

// Class CoreUObject.SparseDelegateFunction
// Size: 0xc8 (Inherited: 0xb8)
struct USparseDelegateFunction : UDelegateFunction {
	char UnknownData_B8[0x10]; // 0xb8(0x10)
};

// Class CoreUObject.PackageMap
// Size: 0x50 (Inherited: 0x28)
struct UPackageMap : UObject {
	char UnknownData_28[0x28]; // 0x28(0x28)
};

// Class CoreUObject.Enum
// Size: 0x58 (Inherited: 0x30)
struct UEnum : UField {
	char UnknownData_30[0x28]; // 0x30(0x28)
};

// Class CoreUObject.LinkerPlaceholderClass
// Size: 0x310 (Inherited: 0x1c0)
struct ULinkerPlaceholderClass : UClass {
	char UnknownData_1C0[0x150]; // 0x1c0(0x150)
};

// Class CoreUObject.LinkerPlaceholderExportObject
// Size: 0x88 (Inherited: 0x28)
struct ULinkerPlaceholderExportObject : UObject {
	char UnknownData_28[0x60]; // 0x28(0x60)
};

// Class CoreUObject.LinkerPlaceholderFunction
// Size: 0x208 (Inherited: 0xb8)
struct ULinkerPlaceholderFunction : UFunction {
	char UnknownData_B8[0x150]; // 0xb8(0x150)
};

// Class CoreUObject.MetaData
// Size: 0x78 (Inherited: 0x28)
struct UMetaData : UObject {
	char UnknownData_28[0x50]; // 0x28(0x50)
};

// Class CoreUObject.ObjectRedirector
// Size: 0x30 (Inherited: 0x28)
struct UObjectRedirector : UObject {
	char UnknownData_28[0x8]; // 0x28(0x08)
};

// Class CoreUObject.Property
// Size: 0x70 (Inherited: 0x30)
struct UProperty : UField {
	char UnknownData_30[0x40]; // 0x30(0x40)
};

// Class CoreUObject.NumericProperty
// Size: 0x70 (Inherited: 0x70)
struct UNumericProperty : UProperty {
};

// Class CoreUObject.ArrayProperty
// Size: 0x78 (Inherited: 0x70)
struct UArrayProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.ObjectPropertyBase
// Size: 0x78 (Inherited: 0x70)
struct UObjectPropertyBase : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.AssetObjectProperty
// Size: 0x78 (Inherited: 0x78)
struct UAssetObjectProperty : UObjectPropertyBase {
};

// Class CoreUObject.AssetClassProperty
// Size: 0x80 (Inherited: 0x78)
struct UAssetClassProperty : UAssetObjectProperty {
	char UnknownData_78[0x8]; // 0x78(0x08)
};

// Class CoreUObject.BoolProperty
// Size: 0x78 (Inherited: 0x70)
struct UBoolProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.ByteProperty
// Size: 0x78 (Inherited: 0x70)
struct UByteProperty : UNumericProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.ObjectProperty
// Size: 0x78 (Inherited: 0x78)
struct UObjectProperty : UObjectPropertyBase {
};

// Class CoreUObject.ClassProperty
// Size: 0x80 (Inherited: 0x78)
struct UClassProperty : UObjectProperty {
	char UnknownData_78[0x8]; // 0x78(0x08)
};

// Class CoreUObject.DelegateProperty
// Size: 0x78 (Inherited: 0x70)
struct UDelegateProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.DoubleProperty
// Size: 0x70 (Inherited: 0x70)
struct UDoubleProperty : UNumericProperty {
};

// Class CoreUObject.FloatProperty
// Size: 0x70 (Inherited: 0x70)
struct UFloatProperty : UNumericProperty {
};

// Class CoreUObject.IntProperty
// Size: 0x70 (Inherited: 0x70)
struct UIntProperty : UNumericProperty {
};

// Class CoreUObject.Int16Property
// Size: 0x70 (Inherited: 0x70)
struct UInt16Property : UNumericProperty {
};

// Class CoreUObject.Int64Property
// Size: 0x70 (Inherited: 0x70)
struct UInt64Property : UNumericProperty {
};

// Class CoreUObject.Int8Property
// Size: 0x70 (Inherited: 0x70)
struct UInt8Property : UNumericProperty {
};

// Class CoreUObject.InterfaceProperty
// Size: 0x78 (Inherited: 0x70)
struct UInterfaceProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.LazyObjectProperty
// Size: 0x78 (Inherited: 0x78)
struct ULazyObjectProperty : UObjectPropertyBase {
};

// Class CoreUObject.MapProperty
// Size: 0xa8 (Inherited: 0x70)
struct UMapProperty : UProperty {
	char UnknownData_70[0x38]; // 0x70(0x38)
};

// Class CoreUObject.MulticastDelegateProperty
// Size: 0x78 (Inherited: 0x70)
struct UMulticastDelegateProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.MulticastInlineDelegateProperty
// Size: 0x78 (Inherited: 0x78)
struct UMulticastInlineDelegateProperty : UMulticastDelegateProperty {
};

// Class CoreUObject.NameProperty
// Size: 0x70 (Inherited: 0x70)
struct UNameProperty : UProperty {
};

// Class CoreUObject.StrProperty
// Size: 0x70 (Inherited: 0x70)
struct UStrProperty : UProperty {
};

// Class CoreUObject.StructProperty
// Size: 0x78 (Inherited: 0x70)
struct UStructProperty : UProperty {
	char UnknownData_70[0x8]; // 0x70(0x08)
};

// Class CoreUObject.UInt16Property
// Size: 0x70 (Inherited: 0x70)
struct UUInt16Property : UNumericProperty {
};

// Class CoreUObject.UInt32Property
// Size: 0x70 (Inherited: 0x70)
struct UUInt32Property : UNumericProperty {
};

// Class CoreUObject.UInt64Property
// Size: 0x70 (Inherited: 0x70)
struct UUInt64Property : UNumericProperty {
};

// Class CoreUObject.WeakObjectProperty
// Size: 0x78 (Inherited: 0x78)
struct UWeakObjectProperty : UObjectPropertyBase {
};

// Class CoreUObject.TextProperty
// Size: 0x70 (Inherited: 0x70)
struct UTextProperty : UProperty {
};

struct UCrewFunctions {
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Athena.CrewFunctions");
	}
	static bool AreCharactersInSameCrew(ACharacter* Player1, ACharacter* Player2) {
		static auto fn = UObject::FindObject<UFunction>("Function Athena.CrewFunctions.AreCharactersInSameCrew");
		struct
		{
			ACharacter* Player1;
			ACharacter* Player2;
			bool ReturnValue;
		} params;
		params.Player1 = Player1;
		params.Player2 = Player2;
		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}
};

// ScriptStruct Athena.AggressiveGhostShipState
// Size: 0x08 (Inherited: 0x00)
struct FAggressiveGhostShipState {
	bool IsShipVisible; // 0x00(0x01)
	bool IsShipDead; // 0x01(0x01)
	char UnknownData_2[0x2]; // 0x02(0x02)
	float ShipSpeed; // 0x04(0x04)
};

// Class Athena.AggressiveGhostShip
// Size: 0x880 (Inherited: 0x3c8)
struct AAggressiveGhostShip : AActor {
	char UnknownData_3D0[0x158]; // 0x3d0(0x40)
	struct FAggressiveGhostShipState ShipState; // 0x520(0x08)
	char UnknownData_530[0x14]; // 0x528(0x14)
	int32_t NumShotsLeftToKill; // 0x53c(0x04)
	char UnknownData_548[0x340]; // 0x540(0x338)
};

enum class EDrawDebugTrace : uint8_t
{
	EDrawDebugTrace__None = 0,
	EDrawDebugTrace__ForOneFrame = 1,
	EDrawDebugTrace__ForDuration = 2,
	EDrawDebugTrace__Persistent = 3,
	EDrawDebugTrace__EDrawDebugTrace_MAX = 4
};


enum class ETraceTypeQuery : uint8_t
{
	TraceTypeQuery1 = 0,
	TraceTypeQuery2 = 1,
	TraceTypeQuery3 = 2,
	TraceTypeQuery4 = 3,
	TraceTypeQuery5 = 4,
	TraceTypeQuery6 = 5,
	TraceTypeQuery7 = 6,
	TraceTypeQuery8 = 7,
	TraceTypeQuery9 = 8,
	TraceTypeQuery10 = 9,
	TraceTypeQuery11 = 10,
	TraceTypeQuery12 = 11,
	TraceTypeQuery13 = 12,
	TraceTypeQuery14 = 13,
	TraceTypeQuery15 = 14,
	TraceTypeQuery16 = 15,
	TraceTypeQuery17 = 16,
	TraceTypeQuery18 = 17,
	TraceTypeQuery19 = 18,
	TraceTypeQuery20 = 19,
	TraceTypeQuery21 = 20,
	TraceTypeQuery22 = 21,
	TraceTypeQuery23 = 22,
	TraceTypeQuery24 = 23,
	TraceTypeQuery25 = 24,
	TraceTypeQuery26 = 25,
	TraceTypeQuery27 = 26,
	TraceTypeQuery28 = 27,
	TraceTypeQuery29 = 28,
	TraceTypeQuery30 = 29,
	TraceTypeQuery31 = 30,
	TraceTypeQuery32 = 31,
	TraceTypeQuery_MAX = 32,
	ETraceTypeQuery_MAX = 33
};


class UKismetSystemLibrary {
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Engine.KismetSystemLibrary");
	}

	static FRotator DrawDebugCircle(struct UObject* WorldContextObject, struct FVector Center, float Radius, int32_t NumSegments, struct FLinearColor LineColor, float Duration, float Thickness, struct FVector YAxis, struct FVector ZAxis, bool bDrawAxis) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.DrawDebugCircle");

		struct {
			struct UObject* WorldContextObject;
			struct FVector Center;
			float Radius;
			int32_t NumSegments;
			struct FLinearColor LineColor;
			float Duration;
			float Thickness;
			struct FVector YAxis;
			struct FVector ZAxis;
			bool bDrawAxis;
		} params;
		params.WorldContextObject = WorldContextObject;
		params.Center = Center;
		params.Radius = Radius;
		params.NumSegments = NumSegments;
		params.LineColor = LineColor;
		params.Duration = Duration;
		params.Thickness = Thickness;
		params.YAxis = YAxis;
		params.ZAxis = ZAxis;
		params.bDrawAxis = bDrawAxis;

		ProcessEvent(defaultObj, fn, &params);
	}
};


class UKismetMathLibrary {
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Engine.KismetMathLibrary");
	}
	static FRotator NormalizedDeltaRotator(const struct FRotator& A, const struct FRotator& B) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetMathLibrary.NormalizedDeltaRotator");

		struct
		{
			struct FRotator                A;
			struct FRotator                B;
			struct FRotator                ReturnValue;
		} params;

		params.A = A;
		params.B = B;

		ProcessEvent(defaultObj, fn, &params);

		return params.ReturnValue;

	};
	static FRotator FindLookAtRotation(const FVector& Start, const FVector& Target) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetMathLibrary.FindLookAtRotation");

		struct {
			FVector Start;
			FVector Target;
			FRotator ReturnValue;
		} params;
		params.Start = Start;
		params.Target = Target;

		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}

	static FVector Conv_RotatorToVector(const struct FRotator& InRot) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetMathLibrary.Conv_RotatorToVector");

		struct
		{
			struct FRotator                InRot;
			struct FVector                 ReturnValue;
		} params;
		params.InRot = InRot;

		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}

	static bool LineTraceSingle_NEW(class UObject* WorldContextObject, const struct FVector& Start, const struct FVector& End, ETraceTypeQuery TraceChannel, bool bTraceComplex, TArray<class AActor*> ActorsToIgnore, EDrawDebugTrace DrawDebugType, bool bIgnoreSelf, struct FHitResult* OutHit)
	{
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.LineTraceSingle_NEW");

		struct
		{
			class UObject* WorldContextObject;
			struct FVector                 Start;
			struct FVector                 End;
			ETraceTypeQuery				   TraceChannel;
			bool                           bTraceComplex;
			TArray<class AActor*>          ActorsToIgnore;
			EDrawDebugTrace				   DrawDebugType;
			struct FHitResult              OutHit;
			bool                           bIgnoreSelf;
			bool                           ReturnValue;
		} params;

		params.WorldContextObject = WorldContextObject;
		params.Start = Start;
		params.End = End;
		params.TraceChannel = TraceChannel;
		params.bTraceComplex = bTraceComplex;
		params.ActorsToIgnore = ActorsToIgnore;
		params.DrawDebugType = DrawDebugType;
		params.bIgnoreSelf = bIgnoreSelf;

		ProcessEvent(defaultObj, fn, &params);

		if (OutHit != nullptr)
			*OutHit = params.OutHit;

		return params.ReturnValue;
	}

	static void DrawDebugBox(UObject* WorldContextObject, const FVector& Center, const FVector& Extent, const FLinearColor& LineColor, const FRotator& Rotation, float Duration) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.DrawDebugBox");
		struct
		{
			UObject* WorldContextObject = nullptr;
			FVector Center;
			FVector Extent;
			FLinearColor LineColor;
			FRotator Rotation;
			float Duration = INFINITY;
		} params;

		params.WorldContextObject = WorldContextObject;
		params.Center = Center;
		params.Extent = Extent;
		params.LineColor = LineColor;
		params.Rotation = Rotation;
		params.Duration = Duration;
		ProcessEvent(defaultObj, fn, &params);
	}
	static void DrawDebugArrow(UObject* WorldContextObject, const FVector& LineStart, const FVector& LineEnd, float ArrowSize, const FLinearColor& LineColor, float Duration) {
		static auto fn = UObject::FindObject<UFunction>("Function Engine.KismetSystemLibrary.DrawDebugBox");
		struct
		{
			class UObject* WorldContextObject = nullptr;
			struct FVector LineStart;
			struct FVector LineEnd;
			float ArrowSize = 1.f;
			struct FLinearColor LineColor;
			float Duration = 1.f;
		} params;

		params.WorldContextObject = WorldContextObject;
		params.LineStart = LineStart;
		params.LineEnd = LineEnd;
		params.ArrowSize = ArrowSize;
		params.LineColor = LineColor;
		params.Duration = Duration;

		ProcessEvent(defaultObj, fn, &params);
	}
};

enum class EMeleeWeaponMovementSpeed : uint8_t
{
	EMeleeWeaponMovementSpeed__Default = 0,
	EMeleeWeaponMovementSpeed__SlightlySlowed = 1,
	EMeleeWeaponMovementSpeed__Slowed = 2,
	EMeleeWeaponMovementSpeed__EMeleeWeaponMovementSpeed_MAX = 3
};

// Class Athena.MeleeAttackDataAsset
// Size: 0x250 (Inherited: 0x28)
struct UMeleeAttackDataAsset
{
	char pad[0x248];
	float                                              ClampYawRange;                                             // 0x248(0x04)(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
	float                                              ClampYawRate;                                              // 0x023C(0x0004) (Edit, BlueprintVisible, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
};

// Class Athena.MeleeWeaponDataAsset
// Size: 0xb0 (Inherited: 0x28)
struct UMeleeWeaponDataAsset
{
	char pad[0x48];
	class UMeleeAttackDataAsset* HeavyAttack; // 0x48(0x08)
	char pad2[0x28];
	EMeleeWeaponMovementSpeed BlockingMovementSpeed; // 0x78(0x01)
};

// Class Athena.MeleeWeapon
// Size: 0xb40 (Inherited: 0x7e0)
struct AMeleeWeapon
{
	char pad[0x808];
	struct UMeleeWeaponDataAsset* DataAsset; // 0x808(0x08)
};

// ScriptStruct Athena.ProjectileShotParams
// Size: 0x1c (Inherited: 0x00)
struct FProjectileShotParams {
	int Seed; // 0x00(0x04)
	float ProjectileDistributionMaxAngle; // 0x04(0x04)
	int NumberOfProjectiles; // 0x08(0x04)
	float ProjectileMaximumRange; // 0x0c(0x04)
	float ProjectileHitScanMaximumRange; // 0x10(0x04)
	float ProjectileDamage; // 0x14(0x04)
	float ProjectileDamageMultiplierAtMaximumRange; // 0x18(0x04)
};

// ScriptStruct Athena.WeaponProjectileParams
// Size: 0xc0 (Inherited: 0x00)
struct FWeaponProjectileParams {
	float Damage; // 0x00(0x04)
	float DamageMultiplierAtMaximumRange; // 0x04(0x04)
	float LifeTime; // 0x08(0x04)
	float TrailFadeOutTime; // 0x0c(0x04)
	float Velocity; // 0x10(0x04)
	float TimeBeforeApplyingGravity; // 0x14(0x04)
	float DownForceVelocityFractionPerSecond; // 0x18(0x04)
	float VelocityDampeningPerSecond; // 0x1c(0x04)
	struct FLinearColor Color; // 0x20(0x10)
	struct UClass* ProjectileId; // 0x30(0x08)
	char HealthChangeReason; // 0x38(0x01)UWorld
	char UnknownData_39[0x3]; // 0x39(0x03)
	char UnknownData_3C[0x68]; // 0x3C(0x68)
	int SuggestedMaxSimulationIterations; // 0xa4(0x04)
	float SuggestedMinTickTimeSecs; // 0xa8(0x04)
	float SuggestedMaxSimulationTimeStep; // 0xac(0x04)
	float HitScanTrailUpdateModifier; // 0xb0(0x04)
	float HitScanTrailFadeOutTime; // 0xb4(0x04)
	float HitScanTrailGrowthSpeed; // 0xb8(0x04)
	char UnknownData_BC[0x4]; // 0xbc(0x04)
};

// ScriptStruct Athena.ProjectileWeaponParameters
// Size: 0x210 (Inherited: 0x00)
struct FProjectileWeaponParameters {
	int32_t AmmoClipSize; // 0x00(0x04)
	int32_t AmmoCostPerShot; // 0x04(0x04)
	float ChargingDuration; // 0x08(0x04)
	float EquipDuration; // 0x0c(0x04)
	float IntoAimingDuration; // 0x10(0x04)
	float RecoilDuration; // 0x14(0x04)
	float ReloadDuration; // 0x18(0x04)
	struct FProjectileShotParams HipFireProjectileShotParams; // 0x1c(0x1c)
	struct FProjectileShotParams AimDownSightsProjectileShotParams; // 0x38(0x1c)
	int32_t InaccuracySeed; // 0x54(0x04)
	float ProjectileDistributionMaxAngle; // 0x58(0x04)
	int32_t NumberOfProjectiles; // 0x5c(0x04)
	float ProjectileMaximumRange; // 0x60(0x04)
	float ProjectileHitScanMaximumRange; // 0x64(0x04)
	float ProjectileDamage; // 0x68(0x04)
	float ProjectileDamageMultiplierAtMaximumRange; // 0x6c(0x04)
	struct UClass* DamagerType; // 0x70(0x08)
	struct UClass* ProjectileId; // 0x78(0x08)
	struct FWeaponProjectileParams AmmoParams; // 0x80(0xc0)
	bool UsesScope; // 0x140(0x01)
	char UnknownData_141[0x3]; // 0x141(0x03)
	float ZoomedRecoilDurationIncrease; // 0x144(0x04)
	float SecondsUntilZoomStarts; // 0x148(0x04)
	float SecondsUntilPostStarts; // 0x14c(0x04)
	float WeaponFiredAINoiseRange; // 0x150(0x04)
	float MaximumRequestPositionDelta; // 0x154(0x04)
	float MaximumRequestAngleDelta; // 0x158(0x04)
	float TimeoutTolerance; // 0x15c(0x04)
	float AimingMoveSpeedScalar; // 0x160(0x04)
	char AimSensitivitySettingCategory; // 0x164(0x01)
	char UnknownData_165[0x3]; // 0x165(0x03)
	float InAimFOV; // 0x168(0x04)
	float BlendSpeed; // 0x16c(0x04)
	char DryFireSfx[0x8]; // 0x170(0x08)
	char AudioEmitterParameters[0x10]; // 0x180(0x10)
	struct FName RumbleTag; // 0x190(0x08)
	bool KnockbackEnabled; // 0x198(0x01)
	char UnknownData_199[0x3]; // 0x199(0x03)
	char KnockbackParams[0x50]; // 0x19c(0x50)
	bool StunEnabled; // 0x1ec(0x01)
	char UnknownData_1ED[0x3]; // 0x1ed(0x03)
	float StunDuration; // 0x1f0(0x04)
	struct FVector TargetingOffset; // 0x1f4(0x0c)
	bool OverridePlayerCharacterInterpolationConeAngleAndDistanceWhenAiming; // 0x200(0x01)
	char UnknownData_201[0x3]; // 0x201(0x03)
	float OverridePlayerCharacterInterpolationConeAngleAndDistanceDuration; // 0x204(0x04)
	float PlayerCharacterInterpolationConeAngleOverride; // 0x208(0x04)
	float PlayerCharacterInterpolationDistanceOverride; // 0x20c(0x04)
};

// Class Athena.ProjectileWeapon
// Size: 0xb70 (Inherited: 0x7e0)
struct AProjectileWeapon {
	char pad[0x828];
	FProjectileWeaponParameters WeaponParameters; // 0x828(0x210)
	char State; // 0xa38(0x01)
	char UnknownData_A39[0x3]; // 0xa39(0x03)
	int32_t AmmoLeft; // 0xa3c(0x04)
	char AimSensitivityComponent[0x8]; // 0xa40(0x08)
	char UnknownData_A48[0x10]; // 0xa48(0x10)
	char ProjectileWeaponType; // 0xa58(0x01)

	bool CanFire()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.ProjectileWeapon.CanFire");
		bool canfire;
		ProcessEvent(this, fn, &canfire);
		return canfire;
	}


};

// Class Athena.TestProjectileWeapon
// Size: 0xc50 (Inherited: 0xb70)
struct ATestProjectileWeapon : AProjectileWeapon {
	void FireInstantly()
	{
		static auto fn = UObject::FindObject<UFunction>("Function Athena.TestProjectileWeapon.FireInstantly");
		ProcessEvent(this, fn, nullptr);
	}
};


// Class Athena.MapTable
// Size: 0x7b0 (Inherited: 0x3c8)
struct AMapTable
{
	char pad[0x4e8];
	struct TArray<struct FVector2D> MapPins; // 0x4e8(0x10)
	struct TArray<struct FWorldMapShipLocation> TrackedShips; // 0x4f8(0x10)
};

// Class Athena.MapTable
// Size: 0x7b0 (Inherited: 0x3c8)
struct AMapTableESP
{
	char pad[0x498];
	bool DisplayFactionShips; // 0x498(0x01)
};


// Enum Cooking.ECookingSmokeFeedbackLevel
enum class ECookingSmokeFeedbackLevel : uint8_t
{
	ECookingSmokeFeedbackLevel__NotCooking = 0,
	ECookingSmokeFeedbackLevel__Raw = 1,
	ECookingSmokeFeedbackLevel__CookedWarning = 2,
	ECookingSmokeFeedbackLevel__Cooked = 3,
	ECookingSmokeFeedbackLevel__BurnedWarning = 4,
	ECookingSmokeFeedbackLevel__Burned = 5,
	ECookingSmokeFeedbackLevel__ECookingSmokeFeedbackLevel_MAX = 6
};

template<class TEnum>
class TEnumAsByte
{
public:
	inline TEnumAsByte()
	{
	}

	inline TEnumAsByte(TEnum _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit inline TEnumAsByte(int32_t _value)
		: value(static_cast<uint8_t>(_value))
	{
	}

	explicit inline TEnumAsByte(uint8_t _value)
		: value(_value)
	{
	}

	inline operator TEnum() const
	{
		return (TEnum)value;
	}

	inline TEnum GetValue() const
	{
		return (TEnum)value;
	}

private:
	uint8_t value;
};

// ScriptStruct Cooking.CookingClientRepresentation
// Size: 0x68 (Inherited: 0x00)
// 0x00A8
struct FCookingClientRepresentation
{
	bool                                               Cooking;                                                  // 0x0000(0x0001) (ZeroConstructor, IsPlainOldData)
	bool                                               HasItem;                                                  // 0x0001(0x0001) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData00[0x6];                                       // 0x0002(0x0006) MISSED OFFSET
	struct FText                                       CurrentCookingItemDisplayName;                            // 0x0008(0x0038)
	class UClass* CurrentCookingItemCategory;                               // 0x0040(0x0008) (ZeroConstructor, IsPlainOldData)
	TEnumAsByte<ECookingSmokeFeedbackLevel>  		SmokeFeedbackLevel;                                       // 0x0048(0x0001) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData01[0x3];                                       // 0x0049(0x0003) MISSED OFFSET
	float                                              VisibleCookedExtent;                                      // 0x004C(0x0004) (ZeroConstructor, IsPlainOldData)
	char               VisibleMaterialSettings[0x30];                                  // 0x0050(0x0030)
	class UMaterialInstance* OverrideMaterial;                                         // 0x0080(0x0008) (ZeroConstructor, IsPlainOldData)
	struct FVector                                     BurnDownVector;                                           // 0x0088(0x000C) (ZeroConstructor, IsPlainOldData)
	unsigned char                                      UnknownData02[0x4];                                       // 0x0094(0x0004) MISSED OFFSET
	class UAnimationAsset* AnimatedPose;                                             // 0x0098(0x0008) (ZeroConstructor, IsPlainOldData)
	struct FName                                       CurrentCookableTypeName;                                  // 0x00A0(0x0008) (ZeroConstructor, IsPlainOldData)
};

// Class Cooking.CookerComponent
// Size: 0x248 (Inherited: 0xc8)
struct UCookerComponent {
	char UnknownData_C8[0xd0]; // 0x0(0xd0)
	struct TArray<struct FStatus> StatusToApplyToContents; // 0xd0(0x10)
	struct TArray<struct FCookerSmokeFeedbackEntry> VFXFeedback; // 0xe0(0x10)
	struct UStaticMeshMemoryConstraintComponent* CookableStaticMeshComponent; // 0xf0(0x08)
	struct USkeletalMeshMemoryConstraintComponent* CookableSkeletalMeshComponent; // 0xf8(0x08)
	struct FName CookedMaterialParameterName; // 0x100(0x08)
	struct FName BurnDownDirectionParameterName; // 0x108(0x08)
	float PlacementVarianceAngleBound; // 0x110(0x04)
	bool OnByDefault; // 0x114(0x01)
	char UnknownData_115[0x3]; // 0x115(0x03)
	struct UCookingComponentAudioParams* AudioParams; // 0x118(0x08)
	char VfxLocation; // 0x120(0x01)
	char UnknownData_121[0x7]; // 0x121(0x07)
	struct AItemInfo* CurrentlyCookingItem; // 0x128(0x08)
	struct FCookingClientRepresentation CookingState; // 0x130(0x68)
	struct UParticleSystemComponent* SmokeParticleComponent; // 0x198(0x08)
	struct UMaterialInstanceDynamic* VisibleCookableMaterial; // 0x1a0(0x08)
	bool TurnedOn; // 0x1a8(0x01)
	bool OnIsland; // 0x1a9(0x01)
	char UnknownData_1AA[0x9e]; // 0x1aa(0x9e)

	void OnRep_CookingState(struct FCookingClientRepresentation OldRepresentation); // Function Cooking.CookerComponent.OnRep_CookingState // Final|Native|Private|HasOutParms // @ game+0x360d390
};

// Class Cooking.CookingPot
// Size: 0x5e0 (Inherited: 0x400)
struct ACookingPot {
	char UnknownData_400[0x408]; // 0x0(0x400)
	struct UStaticMeshComponent* MeshComponent; // 0x408(0x08)
	struct UActionRulesInteractableComponent* InteractableComponent; // 0x408(0x08)
	struct UCookerComponent* CookerComponent; // 0x410(0x08)
	float HoldToInteractTime; // 0x418(0x04)
	char UnknownData_424[0x4]; // 0x424(0x04)
	struct FText NotWieldingCookableItemTooltip; // 0x420(0x38)
	struct FText WieldingCookableItemTooltip; // 0x458(0x38)
	struct FText TakeItemTooltip; // 0x490(0x38)
	struct FText CannotTakeItemTooltip; // 0x4c8(0x38)
	struct FText MixInItemTooltip; // 0x500(0x38)
	char UnknownData_540[0xa0]; // 0x540(0xa0)
};


// Class Cooking.CookableComponent
// Size: 0x128 (Inherited: 0xc8)
struct UCookableComponent {
	char UnknownData_C8[0xe8]; // 0xc8(0x20)
	struct UClass* NextCookState; // 0xe8(0x08)
	float TimeToNextCookState; // 0xf0(0x04)
	char UnknownData_F4[0x4]; // 0xf4(0x04)
	struct TArray<struct FCookableComponentSmokeFeedbackTimingEntry> SmokeFeedbackLevels; // 0xf8(0x10)
	struct UCurveFloat* VisibleCookedExtentOverTime; // 0x108(0x08)
	float DefaultVisibleCookedExtent; // 0x110(0x04)
	struct FName CookableTypeName; // 0x114(0x08)
	//struct FPlayerStat CookedStat; // 0x11c(0x04)
	//struct FPlayerStat ShipCookedStat; // 0x120(0x04)
	char CookedStat[0x04]; // 0x11c(0x04)
	char ShipCookedStat[0x04]; // 0x120(0x04)
	char CookingState; // 0x124(0x01)
	char InitialCookingState; // 0x125(0x01)
	char RemovedCookingState; // 0x126(0x01)
	char UnknownData_127[0x1]; // 0x127(0x01)
};

// Class Engine.CharacterMovementComponent
// Size: 0x590 (Inherited: 0x160)
struct UCharacterMovementComponent {
	char pad_00[0x160];
	char UnknownData_160[0x18]; // 0x160(0x18)
	struct ACharacter* CharacterOwner; // 0x178(0x08)
	float GravityScale; // 0x180(0x04)
	float MaxStepHeight; // 0x184(0x04)
	float JumpZVelocity; // 0x188(0x04)
	float JumpOffJumpZFactor; // 0x18c(0x04)
	float WalkableFloorAngle; // 0x190(0x04)
	float WalkableFloorZ; // 0x194(0x04)
	char MovementMode; // 0x198(0x01)
	char CustomMovementMode; // 0x199(0x01)
	char UnknownData_19A[0x26]; // 0x19a(0x26)
	float GroundFriction; // 0x1c0(0x04)
	float MaxWalkSpeed; // 0x1c4(0x04)
	float MaxWalkSpeedBackwards; // 0x1c8(0x04)
	float WalkBackwardsMinAngle; // 0x1cc(0x04)
	float WalkBackwardsMaxAngle; // 0x1d0(0x04)
	float MaxWalkSpeedCrouched; // 0x1d4(0x04)
	float MaxSwimSpeed; // 0x1d8(0x04)
	float MaxFlySpeed; // 0x1dc(0x04)
};

// Class Athena.OnlineAthenaPlayerController
// Size: 0x1828 (Inherited: 0x1740)
struct AOnlineAthenaPlayerController {
	char pad_00[0x1740];
	char UnknownData_1740[0x8]; // 0x1740(0x08)
	char LogoutNoteCompletionIdent[0x8]; // 0x1748(0x08)
	char UnknownData_1750[0x19]; // 0x1750(0x19)
	bool IdleDisconnectEnabled; // 0x1769(0x01)
	char UnknownData_176A[0xbe]; // 0x176a(0xbe)
};

// Class Athena.Spyglass
// Size: 0x8d0 (Inherited: 0x7e0)
struct ASpyglass {
	char pad_00[0x7e0];
	char UnknownData_7E0[0x10]; // 0x7e0(0x10)
	char InventoryItem[0x8]; // 0x7f0(0x08)
	char UsableWieldableComponent[0x8]; // 0x7f8(0x08)
	char AimSensitivityComponent[0x8]; // 0x800(0x08)
	char UnknownData_808[0x10]; // 0x808(0x10)
	float NameplateVisibilityRangeExtensionFactorWhileZoomed; // 0x818(0x04)
	float SecondsUntilZoomStarts; // 0x81c(0x04)
	float SecondsUntilPostStarts; // 0x820(0x04)
	float InAimFOV; // 0x824(0x04)
	float BlendSpeed; // 0x828(0x04)
	char AimSpeedScaleParameters[0x8]; // 0x82c(0x08)
	char UnknownData_834[0x4]; // 0x834(0x04)
	char PostProcessComponent[0x8]; /// 0x838(0x08)
	char BlurCurve[0x8]; // 0x840(0x08)
	char UnknownData_848[0x38]; // 0x848(0x38)
	char DynamicMaterial[0x8]; // 0x880(0x08)
	bool TurningOn; // 0x888(0x01)
	char UnknownData_889[0x3]; // 0x889(0x03)
	float BlurTime; // 0x88c(0x04)
	float BlurInDuration; // 0x890(0x04)
	float BlurOutDuration; // 0x894(0x04)
	char Glint[0x8]; // 0x898(0x08)
	char MaterialParen[0x8]; // 0x8a0(0x08)
	char LensNormal[0x8]; // 0x8a8(0x08)
	char LensMask[0x8]; // 0x8b0(0x08)
	char LensTint[0x10]; // 0x8b8(0x10)
	char UnknownData_8C8[0x8]; // 0x8c8(0x08)

	void SpyGlassRaisedFirstPerson(bool IsRaised); // Function Athena.Spyglass.SpyGlassRaisedFirstPerson // Event|Protected|BlueprintEvent // @ game+0x179fa40
	void SetGlintOff(); // Function Athena.Spyglass.SetGlintOff // Native|Public|BlueprintCallable // @ game+0x11489f0
	void ResetSpyglassEffects(); // Function Athena.Spyglass.ResetSpyglassEffects // Final|Native|Public|BlueprintCallable // @ game+0x1148200
};


// ScriptStruct Athena.SailsBillowStateChangeRpc
// Size: 0x18 (Inherited: 0x10)
struct FSailsBillowStateChangeRpc {
	char pad_00[0x10];
	bool AreSailsBillowed; // 0x10(0x01)
};

class UFunctionEx : public UStruct
{
public:
	int FunctionFlags;
	uint16_t RepOffset;
	uint8_t NumParms;
	char pad;
	uint16_t ParmsSize;
	uint16_t ReturnValueOffset;
	uint16_t RPCId;
	uint16_t RPCResponseId;
	UProperty* FirstPropertyToInit;
	UFunctionEx* EventGraphFunction; //0x00A0
	int EventGraphCallOffset;
	char pad_0x00AC[0x4]; //0x00AC
	void* Func; //0x00B0
};

// Class Athena.LightingController
// Size: 0xe60 (Inherited: 0x3c8)
struct ALightingController {
	char pad_0[0x728];
	float FixedTimeOfDay; // 0x728(0x04)
	int32_t FixedDay; // 0x72c(0x04)
	bool IsFixedTimeOfDay; // 0x730(0x01)
	char UnknownData_731[0x7]; // 0x731(0x07)

};

// ScriptStruct PirateGenerator.RadialCoordinate
// Size: 0x08 (Inherited: 0x00)
struct FRadialCoordinate {
	float NormalizedAngle; // 0x00(0x04)
	float RadialDistance; // 0x04(0x04)
};

// ScriptStruct Athena.CarouselPirateDesc
// Size: 0x30 (Inherited: 0x00)
struct FCarouselPirateDesc {
	int32_t Seed; // 0x00(0x04)
	char Gender; // 0x04(0x01)
	char Ethnicity; // 0x05(0x01)
	char UnknownData_6[0x2]; // 0x06(0x02)
	float Age; // 0x08(0x04)
	struct FRadialCoordinate BodyShape; // 0x0c(0x08)
	float Dirtiness; // 0x14(0x04)
	float Wonkiness; // 0x18(0x04)
	char UnknownData_1C[0x4]; // 0x1c(0x04)
	struct TArray<struct UClass*> Outfit; // 0x20(0x10)
};

struct PirateGeneratorLineUpUI {
	char pad_00[0x3e8]; // 0x0 (0x3e8)
	struct TArray<struct FCarouselPirateDesc> CarouselPirateDescs; // 0x3e8(0x10)
};