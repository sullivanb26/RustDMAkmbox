#include "pch.h"
#include "Globals.h"
#include "Item.h"

Item::Item(uint64_t address)
{
	printf("[Item] Initialized\n");
	Class = address;
	auto handle = TargetProcess.CreateScatterHandle();
	TargetProcess.QueueScatterReadEx(handle, Class + ItemID,reinterpret_cast<void*>(&ItemID),sizeof(uint64_t));
	TargetProcess.QueueScatterReadEx(handle, Class + HeldEntity, reinterpret_cast<void*>(&HeldEntity), sizeof(uint64_t));
	TargetProcess.ExecuteScatterRead(handle);
	printf("[Item] ItemID: 0x%llX\n", ItemID);
	printf("[Item] HeldEntity: 0x%llX\n", HeldEntity);

}
Item::~Item()
{
	delete BaseProjectileInstance;
}
uint64_t Item::GetItemID()
{
	return ItemID;
}

uint64_t Item::GetHeldEntity()
{
	return HeldEntity;
}

BaseProjectile* Item::GetBaseProjectile()
{
	if (HeldEntity == 0)
		return nullptr;
	BaseProjectile* projectile = new BaseProjectile(HeldEntity);
}