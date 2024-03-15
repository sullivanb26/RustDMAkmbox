#pragma once
#include "Pch.h"
#include "Misc.h"
#include "PlayerEspConfig.h"

class ConfigInstances
{
public:
	MiscConfig Misc = MiscConfig(LIT("Misc"));
	PlayerEspConfig PlayerESP = PlayerEspConfig(LIT("PlayerESP"));

	json ToJson(int configNum)
	{
		json jsoned;
		jsoned.merge_patch(Misc.ToJson());
		jsoned.merge_patch(PlayerESP.ToJson());
		std::string mergePath = "C:\\" + std::to_string(configNum) + "cnfg.json";
		std::ofstream config(mergePath);
		config << jsoned;
	}

	void FromJson(int configNum)
	{
		std::string mergePath = "C:\\" + std::to_string(configNum) + "cnfg.json";
		std::ifstream config(mergePath);
		json jsoned = json::parse(config);
		Misc.FromJson(jsoned);
		PlayerESP.FromJson(jsoned);
	}
};
inline ConfigInstances ConfigInstance;