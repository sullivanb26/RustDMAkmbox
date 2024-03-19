#include "Pch.h"
#include "Globals.h"
#include "BasePlayer.h"
#include <vector>

BasePlayer::BasePlayer(uint64_t address,VMMDLL_SCATTER_HANDLE handle)
{
	this->Class = address;
	if (address == 0)
		return; // invalid
	
	TargetProcess.AddScatterReadRequest(handle,Class + BaseMovementOffset, reinterpret_cast<void*>(&BaseMovementOffset),sizeof(uint64_t));
	TargetProcess.AddScatterReadRequest(handle, Class + ActiveItemIDOffset, reinterpret_cast<void*>(&ActiveItemID), sizeof(uint32_t));
	TargetProcess.AddScatterReadRequest(handle, Class + PlayerInventory, reinterpret_cast<void*>(&PlayerInventory), sizeof(uint64_t));
	TargetProcess.AddScatterReadRequest(handle, Class + PlayerModel, reinterpret_cast<void*>(&PlayerModel), sizeof(uint64_t));
	TargetProcess.AddScatterReadRequest(handle, Class + DisplayName, reinterpret_cast<void*>(&DisplayName), sizeof(uint64_t));

}
void BasePlayer::InitializePlayerList()
{
	uint64_t staticclass = TargetProcess.Read<uint64_t>(TargetProcess.GetBaseAddress(LIT("GameAssembly.dll")) + StaticClass);
	uint64_t staticfield = TargetProcess.Read<uint64_t>(staticclass + 0xb8); // access static fields
	uint64_t playerlist = TargetProcess.Read<uint64_t>(staticfield + VisiblePlayerList);
	VisiblePlayerList = TargetProcess.Read<uint64_t>(playerlist + 0x28);

}
int BasePlayer::GetPlayerListSize()
{
	return PlayerListSize;
}


struct TransformAccessReadOnly
{
	ULONGLONG	pTransformData;
	int			index;
};
struct TransformData
{
	ULONGLONG pTransformArray;
	ULONGLONG pTransformIndices;
};
struct Vector4 
{
	double x, y, z, w;
};
struct Matrix34
{
	Vector4 vec0;
	Vector4 vec1;
	Vector4 vec2;
};
Vector3 BasePlayer::getBonePosition(uint64_t bone_entity)
{
	uint64_t transform = TargetProcess.Read<uint64_t>(bone_entity + 0x10);
	__m128 result;

	const __m128 mulVec0 = { -2.000, 2.000, -2.000, 0.000 };
	const __m128 mulVec1 = { 2.000, -2.000, -2.000, 0.000 };
	const __m128 mulVec2 = { -2.000, -2.000, 2.000, 0.000 };

	TransformAccessReadOnly pTransformAccessReadOnly = TargetProcess.Read<TransformAccessReadOnly>(transform + 0x38);
	unsigned int index = TargetProcess.Read<unsigned int>(transform + 0x40);
	TransformData transformData = TargetProcess.Read<TransformData>(pTransformAccessReadOnly.pTransformData + 0x18);

	if (transformData.pTransformArray && transformData.pTransformIndices)
	{
		result = TargetProcess.Read<__m128>(transformData.pTransformArray + (uint64_t)0x30 * index);
		int transformIndex = TargetProcess.Read<int>(transformData.pTransformIndices + (uint64_t)0x4 * index);
		int pSafe = 0;
		while (transformIndex >= 0 && pSafe++ < 200)
		{
			Matrix34 matrix34 = TargetProcess.Read<Matrix34>(transformData.pTransformArray + (uint64_t)0x30 * transformIndex);

			__m128 xxxx = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x00));	// xxxx
			__m128 yyyy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x55));	// yyyy
			__m128 zwxy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x8E));	// zwxy
			__m128 wzyw = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0xDB));	// wzyw
			__m128 zzzz = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0xAA));	// zzzz
			__m128 yxwy = _mm_castsi128_ps(_mm_shuffle_epi32(*(__m128i*)(&matrix34.vec1), 0x71));	// yxwy
			__m128 tmp7 = _mm_mul_ps(*(__m128*)(&matrix34.vec2), result);

			result = _mm_add_ps(
				_mm_add_ps(
					_mm_add_ps(
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(xxxx, mulVec1), zwxy),
								_mm_mul_ps(_mm_mul_ps(yyyy, mulVec2), wzyw)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0xAA))),
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(zzzz, mulVec2), wzyw),
								_mm_mul_ps(_mm_mul_ps(xxxx, mulVec0), yxwy)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x55)))),
					_mm_add_ps(
						_mm_mul_ps(
							_mm_sub_ps(
								_mm_mul_ps(_mm_mul_ps(yyyy, mulVec0), yxwy),
								_mm_mul_ps(_mm_mul_ps(zzzz, mulVec1), zwxy)),
							_mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(tmp7), 0x00))),
						tmp7)), *(__m128*)(&matrix34.vec0));

			transformIndex = TargetProcess.Read<int>(transformData.pTransformIndices + (uint64_t)0x4 * transformIndex);
		}
	}

	return Vector3(result.m128_f32[0], result.m128_f32[1], result.m128_f32[2]);
}

void BasePlayer::CacheStage1(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle, PlayerModel + Position, reinterpret_cast<void*>(&TransformPosition), sizeof(Vector3));
	TargetProcess.AddScatterReadRequest(handle, DisplayName + 0x14, reinterpret_cast<void*>(&PlayerName), sizeof(PlayerName));
	TargetProcess.AddScatterReadRequest(handle, PlayerModel + IsNPCOffset, reinterpret_cast<void*>(&NPC), sizeof(bool));
	TargetProcess.AddScatterReadRequest(handle, PlayerInventory + ContainerBelt, reinterpret_cast<void*>(&ContainerBelt),sizeof(uint64_t));
	this->BaseMovementInstance = std::make_shared<BaseMovement>(BaseMovementOffset);

	
}
void BasePlayer::CachePlayers()
{
	if (VisiblePlayerList == 0)
	{
		PlayerListSize = 0;
		PlayerList.clear();
		return;
	}
	std::vector< std::shared_ptr<BasePlayer>> templayerlist;
	auto handle = TargetProcess.CreateScatterHandle();
	uint32_t size;
	TargetProcess.AddScatterReadRequest(handle, VisiblePlayerList + 0x10, reinterpret_cast<void*>(&size), sizeof(uint32_t));
	uint64_t buffer;
	TargetProcess.AddScatterReadRequest(handle, VisiblePlayerList + 0x18, reinterpret_cast<void*>(&buffer), sizeof(uint64_t));
	TargetProcess.ExecuteScatterRead(handle);
	TargetProcess.CloseScatterHandle(handle);

	PlayerListSize = size;
	if (size == 0 || buffer == 0)
		return;
	std::vector<uint64_t> playerlist;
	playerlist.resize(size);
	handle = TargetProcess.CreateScatterHandle();
	for (int i = 0; i < size; i++)
	{
		TargetProcess.AddScatterReadRequest(handle, buffer + (0x20 + (i * 8)), reinterpret_cast<void*>(&playerlist[i]), sizeof(uint64_t));
	}
	TargetProcess.ExecuteScatterRead(handle);
	TargetProcess.CloseScatterHandle(handle);

	handle = TargetProcess.CreateScatterHandle();
	for (int i = 0; i < size; i++)
	{
		if (playerlist[i] == NULL)
			continue;
		templayerlist.push_back(std::make_shared<BasePlayer>(playerlist[i], handle));

	}
	TargetProcess.ExecuteScatterRead(handle);
	TargetProcess.CloseScatterHandle(handle);

	handle = TargetProcess.CreateScatterHandle();
	for (int i = 0; i < templayerlist.size(); i++)
	{
		std::shared_ptr<BasePlayer> player = templayerlist[i];
		player->CacheStage1(handle);
		
	}
	TargetProcess.ExecuteScatterRead(handle);
	TargetProcess.CloseScatterHandle(handle);
	PlayerList = templayerlist;
}
BasePlayer::~BasePlayer()
{
}
PlayerFlags BasePlayer::GetPlayerFlag()
{
	if (!IsPlayerValid())
		return PlayerFlags::Connected;
	PlayerFlags flag = TargetProcess.Read<PlayerFlags>(Class + PlayerFlag);
	return flag;
}
void BasePlayer::WritePlayerFlag(PlayerFlags flag)
{
	if (!IsPlayerValid())
		return;
	if(!TargetProcess.Write<PlayerFlags>(Class + PlayerFlag,flag))
			printf("[BasePlayer] Failed to write PlayerFlag\n");
}
uint32_t BasePlayer::GetActiveItemID()
{
	return ActiveItemID;
}
// call this in the local player loop to keep the value updated as it changes depending on the item
void BasePlayer::UpdateActiveItemID(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle,Class + ActiveItemIDOffset, reinterpret_cast<void*>(&ActiveItemID), sizeof(uint64_t));
}
std::shared_ptr<BaseMovement> BasePlayer::GetBaseMovement()
{
	return BaseMovementInstance;
}
void BasePlayer::SetupBeltContainerList()
{
	if (!IsPlayerValid())
		return;
	BeltContainerList.clear();
	uint64_t itemlist = TargetProcess.Read<uint64_t>(ContainerBelt + ItemList); // yeah you need to reread this constantly, if you don't hell breaks loose. 
	auto handle = TargetProcess.CreateScatterHandle();
	uint64_t items = 0;
	TargetProcess.AddScatterReadRequest(handle, itemlist + ItemListContents, reinterpret_cast<void*>(&items), sizeof(uint64_t));
	uint32_t itemsize = 0;
	TargetProcess.AddScatterReadRequest(handle, itemlist + ItemListSize, reinterpret_cast<void*>(&itemsize), sizeof(uint32_t));
	TargetProcess.ExecuteScatterRead(handle);
	TargetProcess.CloseScatterHandle(handle);
	BeltContainerList.resize(itemsize);	

	std::vector<uint64_t> objectpointrs;
	objectpointrs.resize(itemsize);


	handle = TargetProcess.CreateScatterHandle();
	for (int i = 0; i < itemsize; i++)
	{
		TargetProcess.AddScatterReadRequest(handle, items + 0x20 + (i * 0x8), reinterpret_cast<void*>(&objectpointrs[i]), sizeof(uint64_t));
	}
	TargetProcess.ExecuteScatterRead(handle);
	TargetProcess.CloseScatterHandle(handle);
	for (int i = 0; i < itemsize; i++)
	{
		if (objectpointrs[i] == NULL)
			continue;
		BeltContainerList.push_back(std::make_shared<Item>(objectpointrs[i]));
	}
}
// it appears that we cant get the item class correctly, no idea where the issue lies. 
std::shared_ptr<Item> BasePlayer::GetActiveItem()
{
	if (ActiveItemID == 0)
		return nullptr;
	if (!IsPlayerValid())
		return nullptr;
	std::shared_ptr<Item> founditem = nullptr;
	for (std::shared_ptr<Item> item : BeltContainerList)
	{
		if (item == NULL)
			continue; // no wasting reads and writes on null pointers

		int activeweaponid = item->GetItemID();

		if (ActiveItemID == activeweaponid)
		{

			founditem =  item;
			break;
		}
		
	}
	return founditem;
	
}

bool BasePlayer::IsPlayerValid()
{
	return Class != 0 && PlayerInventory != 0;
}

bool BasePlayer::IsSleeping()
{
	if (!IsPlayerValid())
		return false;
	return (ActiveFlag & (int)16) == (int)16;
}
bool BasePlayer::IsNPC()
{
	if (!IsPlayerValid())
		return false;
	return NPC;
}

std::wstring BasePlayer::GetName()
{
	if(!IsNPC())
	return std::wstring(PlayerName);
	else
		return std::wstring(LIT(L"Scientist"));
}
std::vector<std::shared_ptr<BasePlayer>> BasePlayer::GetPlayerList()
{
	return PlayerList;
}
uint64_t BasePlayer::GetClass()
{
	return Class;
}
void BasePlayer::UpdatePosition(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle, PlayerModel + Position, reinterpret_cast<void*>(&TransformPosition), sizeof(Vector3));
}
void BasePlayer::UpdateBonePositions(VMMDLL_SCATTER_HANDLE handle)
{
	BonePositionsVector.clear();
	auto entity_model = TargetProcess.Read<uint64_t>(BaseEntity + model);
	auto bone_transforms = TargetProcess.Read<uint64_t>(entity_model + boneTransforms);
	for (int bone_num = 0; bone_num < 13; bone_num++) {
		auto entity_bone = TargetProcess.Read<uint64_t>(bone_transforms + (0x20 + (bone_num * 0x8)));
		auto bone = TargetProcess.Read<uint64_t>(entity_bone + 0x10);
		BonePositionsVector.push_back(getBonePosition(bone));
	}
}
void BasePlayer::UpdateDestroyed(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle, Class + DestroyedOffset, reinterpret_cast<void*>(&Destroyed), sizeof(bool));
}
Vector3 BasePlayer::GetPosition()
{
	return TransformPosition;
}
Vector3 BasePlayer::GetBonePositions(int bone_index)
{
	return BonePositionsVector[bone_index];
}
void BasePlayer::UpdateActiveFlag(VMMDLL_SCATTER_HANDLE handle)
{
	TargetProcess.AddScatterReadRequest(handle, Class + PlayerFlag, reinterpret_cast<void*>(&ActiveFlag), sizeof(int));
}
void BasePlayer::WriteActiveFlag(int flag)
{
	if (!IsPlayerValid())
		return;
	if (!TargetProcess.Write<int>(Class + PlayerFlag, flag))
		printf("[BasePlayer] Failed to write flag\n");
}
int BasePlayer::GetActiveFlag()
{
	return ActiveFlag;
}