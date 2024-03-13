#include "Pch.h"
#include "ConvarGraphics.h"
#include "Globals.h"

ConvarGraphics::ConvarGraphics()
{
	printf("[ConvarGraphics] Initialized\n");
	uint64_t graphics = TargetProcess.Read<uint64_t>(TargetProcess.GetBaseAddress(LIT("GameAssembly.dll")) + Class); // Get Class Start Address
	printf("[ConvarGraphics] ConvarGraphics: 0x%llX\n", graphics);
	this->StaticField = TargetProcess.Read<uint64_t>(graphics + StaticField); // Set Static Padding
	printf("[ConvarGraphics] Static Fields: 0x%llX\n", StaticField);
}
void ConvarGraphics::WriteFOV(float fov)
{
	if(!TargetProcess.Write<float>(StaticField + FOV, fov))
	printf("[ConvarGraphics] Failed to write FOV\n");
}

double ConvarGraphics::ReadFOV()
{
	uint64_t fovOfuint = TargetProcess.Read<uint64_t>(StaticField + FOV);
	double fovOf = (double) fovOfuint; // May need to strip hex tag
	printf("[ConvarGraphics] Fov: 0x%llX\n", fovOf);
	return fovOf;
}