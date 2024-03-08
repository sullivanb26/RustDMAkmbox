#include "pch.h"
#include "Globals.h"
#include "memory.h"
#include "OcclusionCulling.h"
#include "MainCamera.h"
#include "ConvarGraphics.h"
#include "ConvarAdmin.h"
#include "ConsoleSystem.h"
#include "LocalPlayer.h"
#include "BaseNetworkable.h"
#include "BasePlayer.h"
#include "TODSky.h"
#include "BaseProjectile.h"
#include "CheatFunction.h"
#include "Init.h"
#include "GUI.h"
#include "Configinstance.h"
#include "kmbox_interface.hpp"
#include <dwmapi.h>
std::shared_ptr<BasePlayer> BaseLocalPlayer = nullptr;
std::shared_ptr<MainCamera> Camera = nullptr;
std::shared_ptr<ConsoleSystem> Console = nullptr;
std::shared_ptr<TODSky> Sky = nullptr;
// each time we reinitialize localplayer
void PerServerVariables()
{
	std::shared_ptr <LocalPlayer> localplayer = std::make_shared <LocalPlayer>();
	auto handle = TargetProcess.CreateScatterHandle();
	BaseLocalPlayer = std::make_shared <BasePlayer>(localplayer->GetBasePlayer(),handle);
	TargetProcess.ExecuteReadScatter(handle);
	TargetProcess.CloseScatterHandle(handle);
	BaseLocalPlayer->InitializePlayerList();
	handle = TargetProcess.CreateScatterHandle();
	BaseLocalPlayer->CacheStage1(handle);
	TargetProcess.ExecuteReadScatter(handle);
	TargetProcess.CloseScatterHandle(handle);
	Camera = std::make_shared <MainCamera>();
	Sky = std::make_shared<TODSky>();
}
void SetupCvars()
{

	std::shared_ptr<OcclusionCulling> occlusionculling = std::make_shared<OcclusionCulling>();
	if (ConfigInstance.Misc.AdminESP & ConfigInstance.Misc.UnsafeFeat)
	{
		occlusionculling->WriteDebugSettings(DebugFilter::Dynamic);
		occlusionculling->WriteLayerMask(131072);
	}
	else if (ConfigInstance.Misc.UnsafeFeat)
	{
		occlusionculling->WriteDebugSettings(DebugFilter::Off);
		occlusionculling->WriteLayerMask(0);
	}
	std::shared_ptr<ConvarAdmin> convaradmin = std::make_shared<ConvarAdmin>();
	if (ConfigInstance.Misc.RemoveWaterEffect & ConfigInstance.Misc.UnsafeFeat)
		convaradmin->ClearVisionInWater(true);
	if (ConfigInstance.Misc.ChangeTime & ConfigInstance.Misc.UnsafeFeat)
		convaradmin->SetAdminTime(ConfigInstance.Misc.Time);
	else if (ConfigInstance.Misc.UnsafeFeat)
		convaradmin->SetAdminTime(-1);
	std::shared_ptr<ConvarGraphics> graphics = std::make_shared<ConvarGraphics>();
	if (ConfigInstance.Misc.ChangeFov & ConfigInstance.Misc.UnsafeFeat)
		graphics->WriteFOV(ConfigInstance.Misc.Fov);

}
std::shared_ptr<CheatFunction> CachePlayers = std::make_shared<CheatFunction>(2000, []() {
		BaseLocalPlayer->CachePlayers();
	});
std::shared_ptr<CheatFunction> UpdateMovement = std::make_shared<CheatFunction>(38, []() {
	if (ConfigInstance.Misc.SpiderMan & ConfigInstance.Misc.UnsafeFeat)
	{
		auto handle = TargetProcess.CreateScatterHandle();
		BaseLocalPlayer->GetBaseMovement()->WriteGroundAngleNew(handle, 0.f);
		BaseLocalPlayer->GetBaseMovement()->WriteMaxAngleWalking(handle, 100.f);
		BaseLocalPlayer->GetBaseMovement()->WriteGroundAngle(handle, 0.f);
		TargetProcess.ExecuteScatterWrite(handle);
		TargetProcess.CloseScatterHandle(handle);
	}
	});
std::shared_ptr<CheatFunction> UpdateLocalPlayer = std::make_shared<CheatFunction>(300, []() {

	if (ConfigInstance.Misc.NoRecoil & ConfigInstance.Misc.UnsafeFeat)
	{
		BaseLocalPlayer->SetupBeltContainerList();
	}

	auto handle = TargetProcess.CreateScatterHandle();
	BaseLocalPlayer->UpdateActiveItemID(handle);
	BaseLocalPlayer->UpdateActiveFlag(handle);
	TargetProcess.ExecuteReadScatter(handle);
	TargetProcess.CloseScatterHandle(handle);

	if (ConfigInstance.Misc.NoRecoil & ConfigInstance.Misc.UnsafeFeat)
	{
		std::shared_ptr <Item> helditem = BaseLocalPlayer->GetActiveItem();
		if (helditem != nullptr)
		{
			std::shared_ptr <BaseProjectile> weapon = helditem->GetBaseProjectile();
			if (weapon->IsValidWeapon())
			{
				handle = TargetProcess.CreateScatterHandle();
				weapon->WriteRecoilPitch(handle,helditem->GetItemID(),ConfigInstance.Misc.RecoilX);
				weapon->WriteRecoilYaw(handle,helditem->GetItemID(), ConfigInstance.Misc.RecoilY);
				TargetProcess.ExecuteScatterWrite(handle);
				TargetProcess.CloseScatterHandle(handle);
			}

		}

	}
	if (ConfigInstance.Misc.NoRecoilKMbox)
	{
		std::shared_ptr <Item> helditem = BaseLocalPlayer->GetActiveItem();
		if (helditem != nullptr)
		{
			std::shared_ptr <BaseProjectile> weapon = helditem->GetBaseProjectile();
			if (weapon->IsValidWeapon())
			{
				// TO DO
				// * Create weapon selection from held weaponid
				//   - Will do via case switch possibly
				// * Track weapon recoil using km.move()

				// handle = TargetProcess.CreateScatterHandle();
				// weapon->WriteRecoilPitch(handle,helditem->GetItemID(),ConfigInstance.Misc.RecoilX);
				// weapon->WriteRecoilYaw(handle,helditem->GetItemID(), ConfigInstance.Misc.RecoilY);
				// TargetProcess.ExecuteScatterWrite(handle);
				// TargetProcess.CloseScatterHandle(handle);
			}

		}

	}
	
	if (ConfigInstance.Misc.AdminFlag & ConfigInstance.Misc.UnsafeFeat)
	{
		if ((BaseLocalPlayer->GetActiveFlag() & (int)4) != (int)4)
		{
			if (Console == nullptr)
			{
				Console = std::make_shared<ConsoleSystem>();

			}
			BaseLocalPlayer->WriteActiveFlag(BaseLocalPlayer->GetActiveFlag() + 4);
		}
	}
	});
std::shared_ptr<CheatFunction> SkyManager = std::make_shared<CheatFunction>(7, []() {
	auto handle = TargetProcess.CreateScatterHandle();
	if (ConfigInstance.Misc.BrightNights & ConfigInstance.Misc.UnsafeFeat)
	{
		Sky->WriteNightLightIntensity(handle, 25.0f);
		Sky->WriteNightAmbientMultiplier(handle, 4.0f);
	}

		if (ConfigInstance.Misc.BrightCaves & ConfigInstance.Misc.UnsafeFeat)
		{
			Sky->WriteDayAmbientMultiplier(handle, 2.0f);

		}
		TargetProcess.ExecuteScatterWrite(handle);
		TargetProcess.CloseScatterHandle(handle);
	

	});

void Caching()
{
	if (BaseLocalPlayer->GetPlayerListSize() == 0)
		return;
	CachePlayers->Execute();
	UpdateLocalPlayer->Execute();
	SkyManager->Execute();
	UpdateMovement->Execute();
}
void Intialize()
{
	PerServerVariables();
	while (!BaseLocalPlayer->IsPlayerValid())
	{
		Sleep(4000);

		Intialize(); // wait till localplayer is valid.
	}
	SetupCvars();
	CachePlayers->Execute();
	
}

inline void km_move(int X, int Y) {
	std::string command = "km.move(" + std::to_string(X) + "," + std::to_string(Y) + ")\r\n";
	send_command(hSerial, command.c_str());
}

inline void km_click() {
	std::string command = "km.left(" + std::to_string(1)  + ")\r\n"; // left mouse button down
	Sleep(10); // to stop it from crashing idk
	std::string command1 = "km.left(" + std::to_string(0) + ")\r\n"; // left mouse button up
	send_command(hSerial, command.c_str());
	send_command(hSerial, command1.c_str());
}

void main()
{
	if (!TargetProcess.Init("RustClient.exe"))
	{
		printf("Failed to initialize process\n");
		return;
	}
	TargetProcess.GetBaseAddress("GameAssembly.dll");
	TargetProcess.FixCr3();
	Intialize();
}
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	InputWndProc(hWnd, message, wParam, lParam);
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hWnd;
	WNDCLASSEX wc;
	AllocConsole();
	FILE* fDummy;
	freopen_s(&fDummy, LIT("CONIN$"), LIT("r"), stdin);
	freopen_s(&fDummy, LIT("CONOUT$"), LIT("w"), stderr);
	freopen_s(&fDummy, LIT("CONOUT$"), LIT("w"), stdout);
	printf(LIT("Debugging Window:\n"));
	SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
	main();
	ZeroMemory(&wc, sizeof(WNDCLASSEX));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = L"GUI Framework";
	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(WS_EX_APPWINDOW, wc.lpszClassName, L"GUI Framework",
		WS_POPUP,
		0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return -1;


	SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 255, LWA_ALPHA);

	ShowWindow(hWnd, nCmdShow);

	InitD2D(hWnd);
	CreateGUI();
	MSG msg;
	SetProcessDPIAware();
	SetInput();
	while (TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				break;
		}
		Caching();
		RenderFrame();

	}
	CleanD2D();
	return msg.wParam;
}

