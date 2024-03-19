#include "pch.h"
#include "globals.h"
#include "PlayerEsp.h"
#include "Visuals.h"
#include "Drawing.h"
#include "ConfigInstance.h"

std::shared_ptr<CheatFunction> UpdatePlayers = std::make_shared<CheatFunction>(20, []() {
	auto handle = TargetProcess.CreateScatterHandle();
	BaseLocalPlayer->UpdatePosition(handle);
	for (std::shared_ptr<BasePlayer> player : BaseLocalPlayer->GetPlayerList())
	{
		if (!player->IsPlayerValid())
			continue;
		if(player->GetClass() == BaseLocalPlayer->GetClass())
			continue;
		if (player->IsSleeping())
			continue;
		player->UpdatePosition(handle);
		player->UpdateBonePositions(handle);
		player->UpdateDestroyed(handle);
		player->UpdateActiveFlag(handle);
	}
	TargetProcess.ExecuteReadScatter(handle);
	TargetProcess.CloseScatterHandle(handle);
	});

void DrawPlayers()
{
	if(ConfigInstance.Misc.fovCircle) {
		int width = GetSystemMetrics(SM_CYSCREEN) * ConfigInstance.Misc.fovKM/180;
		OutlineCircle(GetSystemMetrics(SM_CXSCREEN)/2, GetSystemMetrics(SM_CYSCREEN)/2, width/2, 3.14f, D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f));
	}
	if(ConfigInstance.PlayerESP.Enable) {
	for (std::shared_ptr<BasePlayer> player : BaseLocalPlayer->GetPlayerList())
	{
		if (!player->IsPlayerValid())
			continue;
		if (player->GetClass() == BaseLocalPlayer->GetClass())
			continue;
		if (player->IsSleeping())
			continue;
	
		Vector3 position = player->GetPosition();
		if (position == Vector3(0, 0, 0))
			continue;
		Vector2 screenpos = WorldToScreen(position);
		if (screenpos.x == 0 && screenpos.y == 0)
			continue;
		
		int distance = (int)Vector3::Distance(position, BaseLocalPlayer->GetPosition());
		if(distance > ConfigInstance.PlayerESP.MaxDistance)
			continue;
		std::wstring name = ConfigInstance.PlayerESP.Name ? player->GetName() : LIT(L"");
		std::wstring distancestr = ConfigInstance.PlayerESP.Distance ? LIT(L"[") + std::to_wstring(distance) + LIT(L"m]") : LIT(L"");
		DrawText(screenpos.x, screenpos.y, name + distancestr, LIT("Verdana"), 11, ConfigInstance.PlayerESP.Colour, FontAlignment::Centre);
		if(ConfigInstance.PlayerESP.Box) {
			Vector3 headPosition = player->GetBonePositions(3); // Need to get head bone position update # to head bone
			if (headPosition == Vector3(0, 0, 0))
				continue;
			Vector2 headScreenpos = WorldToScreen(headPosition);
			if (headScreenpos.x == 0 && headScreenpos.y == 0)
				continue;
			int heightOf = screenpos.y-headScreenpos.y;
			int widthOf = heightOf/2;
			Vector2 boxpos;
			boxpos.x = screenpos.x-(widthOf/2);
			boxpos.y = screenpos.y;
			OutlineRectangle(boxpos.x, boxpos.y, heightOf, widthOf, ConfigInstance.PlayerESP.Width, ConfigInstance.PlayerESP.Colour);
		}
	}

	}
}
