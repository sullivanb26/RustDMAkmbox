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
#include <windows.h>
#include <vector>
#include <array>
std::shared_ptr<BasePlayer> BaseLocalPlayer = nullptr;
std::shared_ptr<MainCamera> Camera = nullptr;
std::shared_ptr<ConsoleSystem> Console = nullptr;
std::shared_ptr<TODSky> Sky = nullptr;
inline void km_move(int X, int Y) {
	std::string command = "km.move(" + std::to_string(X) + "," + std::to_string(Y) + ")\r\n";
	send_command(hSerial, command.c_str());
}

inline void km_click() {
	std::string command = "km.left(" + std::to_string(1) + ")\r\n"; // left mouse button down
	Sleep(10); // to stop it from crashing idk
	std::string command1 = "km.left(" + std::to_string(0) + ")\r\n"; // left mouse button up
	send_command(hSerial, command.c_str());
	send_command(hSerial, command1.c_str());
}

inline bool km_left() {
	std::string command = "km.left()\r\n";
	return receive_data(hSerial, command);
}
inline bool km_right() {
	std::string command = "km.right()\r\n";
	return receive_data(hSerial, command);
}
inline bool km_isDown(string key) {
	std::string command = "km.isdown('" + key + "')\r\n";
	return receive_data(hSerial, command);
}

// each time we reinitialize localplayer
void PerServerVariables()
{
	std::shared_ptr <LocalPlayer> localplayer = std::make_shared <LocalPlayer>();
	auto handle = TargetProcess.CreateScatterHandle();
	BaseLocalPlayer = std::make_shared <BasePlayer>(localplayer->GetBasePlayer(), handle);
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
	if (ConfigInstance.Misc.AdminESP && ConfigInstance.Misc.UnsafeFeat)
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
	if (ConfigInstance.Misc.RemoveWaterEffect && ConfigInstance.Misc.UnsafeFeat)
		convaradmin->ClearVisionInWater(true);
	if (ConfigInstance.Misc.ChangeTime && ConfigInstance.Misc.UnsafeFeat)
		convaradmin->SetAdminTime(ConfigInstance.Misc.Time);
	else if (ConfigInstance.Misc.UnsafeFeat)
		convaradmin->SetAdminTime(-1);
	std::shared_ptr<ConvarGraphics> graphics = std::make_shared<ConvarGraphics>();
	if (ConfigInstance.Misc.ChangeFov && ConfigInstance.Misc.UnsafeFeat)
		graphics->WriteFOV(ConfigInstance.Misc.Fov);

}
std::shared_ptr<CheatFunction> CachePlayers = std::make_shared<CheatFunction>(2000, []() {
	BaseLocalPlayer->CachePlayers();
	});
std::shared_ptr<CheatFunction> UpdateMovement = std::make_shared<CheatFunction>(38, []() {
	if (ConfigInstance.Misc.SpiderMan && ConfigInstance.Misc.UnsafeFeat)
	{
		auto handle = TargetProcess.CreateScatterHandle();
		BaseLocalPlayer->GetBaseMovement()->WriteGroundAngleNew(handle, 0.f);
		BaseLocalPlayer->GetBaseMovement()->WriteMaxAngleWalking(handle, 100.f);
		BaseLocalPlayer->GetBaseMovement()->WriteGroundAngle(handle, 0.f);
		TargetProcess.ExecuteScatterWrite(handle);
		TargetProcess.CloseScatterHandle(handle);
	}
	});
const std::array<double, 4> atta_multi = {
	/* None */		1.0,
	/* 8x */		4.76,
	/* holo */	1.2,
	/* handmade */		0.8
};
const std::array<string, 12> weapon_name = {
	/* None */		"None",
	/* AK-47 */		"AK-47",
	/* LR-300 */	"LR-300",
	/* M249 */		"M249",
	/* HMLMG */		"HMLMG",
	/* MP5 */		"MP5",
	/* Thompson */	"Thompson",
	/* Custom */	"Custom",
	/* Python */	"Python",
	/* Semi */	"Semi"
};
const std::array<double, 12> weapon_delays = {
	/* None */		0.0,
	/* AK-47 */		133.33,
	/* LR-300 */	120.0,
	/* M249 */		100.0,
	/* HMLMG */		100.0,
	/* MP5 */		89.0,
	/* Thompson */	113.0,
	/* Custom */	90.0,
	/* Python */	125.0,
	/* Semi */	175.0
};
const std::vector<std::vector<std::array<double, 2>>>recoil_tables = {
	/* None */		{ { 0.0 } },
	/* AK-47 */		{ {0.000000,-2.257792},{0.323242,-2.300758},{0.649593,-2.299759},{0.848786,-2.259034},{1.075408,-2.323947},{1.268491,-2.215956},{1.330963,-2.236556},{1.336833,-2.218203},{1.505516,-2.143454},{1.504423,-2.233091},{1.442116,-2.270194},{1.478543,-2.204318},{1.392874,-2.165817},{1.480824,-2.177887},{1.597069,-2.270915},{1.449996,-2.145893},{1.369179,-2.270450},{1.582363,-2.298334},{1.516872,-2.235066},{1.498249,-2.238401},{1.465769,-2.331642},{1.564812,-2.242621},{1.517519,-2.303052},{1.422433,-2.211946},{1.553195,-2.248043},{1.510463,-2.285327},{1.553878,-2.240047},{1.520380,-2.221839},{1.553878,-2.240047},{1.553195,-2.248043} },
	/* LR-300 */	{ {0.000000,-2.052616},{0.055584,-1.897695},{-0.247226,-1.863222},{-0.243871,-1.940010},{0.095727,-1.966751},{0.107707,-1.885520},{0.324888,-1.946722},{-0.181137,-1.880342},{0.162399,-1.820107},{-0.292076,-1.994940},{0.064575,-1.837156},{-0.126699,-1.887880},{-0.090568,-1.832799},{0.065338,-1.807480},{-0.197343,-1.705888},{-0.216561,-1.785949},{0.042567,-1.806371},{-0.065534,-1.757623},{0.086380,-1.904010},{-0.097326,-1.969296},{-0.213034,-1.850288},{-0.017790,-1.730867},{-0.045577,-1.783686},{-0.053309,-1.886260},{0.055072,-1.793076},{-0.091874,-1.921906},{-0.033719,-1.796160},{0.266464,-1.993952},{0.079090,-1.921165} },
	/* M249 */	{{0, -1.49}, {0.39375, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.720, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}, {0.0, -1.4900}},
	/* HMLMG */	{{0, -1.4}, {-0.39, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}, {-0.73, -1.4}},
	/* MP5 */	{ {0.125361,-1.052446},{-0.099548,-0.931548},{0.027825,-0.954094},{-0.013715,-0.851504},{-0.007947,-1.070579},{0.096096,-1.018017},{-0.045937,-0.794216},{0.034316,-1.112618},{-0.003968,-0.930040},{-0.009403,-0.888503},{0.140813,-0.970807},{-0.015052,-1.046551},{0.095699,-0.860475},{-0.269643,-1.038896},{0.000285,-0.840478},{0.018413,-1.038126},{0.099191,-0.851701},{0.199659,-0.893041},{-0.082660,-1.069278},{0.006826,-0.881493},{0.091709,-1.150956},{-0.108677,-0.965513},{0.169612,-1.099499},{-0.038244,-1.120084},{-0.085513,-0.876956},{0.136279,-1.047589},{0.196392,-1.039977},{-0.152513,-1.209291},{-0.214510,-0.956648},{0.034276,-0.095177} },
	/* Thompson */	{ {-0.114413,-0.680635},{0.008686,-0.676598},{0.010312,-0.682837},{0.064825,-0.691345},{0.104075,-0.655618},{-0.088118,-0.660429},{0.089906,-0.675183},{0.037071,-0.632623},{0.178465,-0.634737},{0.034654,-0.669443},{-0.082658,-0.664826},{0.025550,-0.636631},{0.082414,-0.647118},{-0.123305,-0.662104},{0.028164,-0.662354},{-0.117346,-0.693475},{-0.268777,-0.661123},{-0.053086,-0.677493},{0.04238,-0.647038}, {0.04238,-0.647038} },
	/* Custom */	{ {-0.114414,-0.680635},{0.008685,-0.676597},{0.010312,-0.682837},{0.064825,-0.691344},{0.104075,-0.655617},{-0.088118,-0.660429},{0.089906,-0.675183},{0.037071,-0.632623},{0.178466,-0.634737},{0.034653,-0.669444},{-0.082658,-0.664827},{0.025551,-0.636631},{0.082413,-0.647118},{-0.123305,-0.662104},{0.028164,-0.662354},{-0.117345,-0.693474},{-0.268777,-0.661122},{-0.053086,-0.677493},{0.004238,-0.647037},{0.014169,-0.551440},{-0.009907,-0.552079},{0.044076,-0.577694},{-0.043187,-0.549581} },
	/* Semi */	{ {0, -1.4} },
	/* Python */	{ {0, -5.8} }
};
const std::vector<std::array<double, 2>>ak47 = { {0.000000,-2.257792},{0.323242,-2.300758},{0.649593,-2.299759},{0.848786,-2.259034},{1.075408,-2.323947},{1.268491,-2.215956},{1.330963,-2.236556},{1.336833,-2.218203},{1.505516,-2.143454},{1.504423,-2.233091},{1.442116,-2.270194},{1.478543,-2.204318},{1.392874,-2.165817},{1.480824,-2.177887},{1.597069,-2.270915},{1.449996,-2.145893},{1.369179,-2.270450},{1.582363,-2.298334},{1.516872,-2.235066},{1.498249,-2.238401},{1.465769,-2.331642},{1.564812,-2.242621},{1.517519,-2.303052},{1.422433,-2.211946},{1.553195,-2.248043},{1.510463,-2.285327},{1.553878,-2.240047},{1.520380,-2.221839},{1.553878,-2.240047},{1.553195,-2.248043} };
const double akwait = 133.33;
std::shared_ptr<CheatFunction> UpdateLocalPlayer = std::make_shared<CheatFunction>(80, []() {

	if (ConfigInstance.Misc.NoRecoilKMbox || (ConfigInstance.Misc.NoRecoil && ConfigInstance.Misc.UnsafeFeat))
	{
		BaseLocalPlayer->SetupBeltContainerList();
	}

	auto handle = TargetProcess.CreateScatterHandle();
	BaseLocalPlayer->UpdateActiveItemID(handle);
	BaseLocalPlayer->UpdateActiveFlag(handle);
	TargetProcess.ExecuteReadScatter(handle);
	TargetProcess.CloseScatterHandle(handle);

	if (ConfigInstance.Misc.NoRecoilKMbox || (ConfigInstance.Misc.NoRecoil && ConfigInstance.Misc.UnsafeFeat))
	{
		std::shared_ptr <Item> helditem = BaseLocalPlayer->GetActiveItem();
		// LOG("\n helditem %d", helditem);
		if (helditem != nullptr)
		{
			//LOG("\n helditem %d", helditem);
			LOG("\n itemID %d", helditem->GetItemID());
			std::shared_ptr <BaseProjectile> weapon = helditem->GetBaseProjectile();
			if (weapon->IsValidWeapon())
			{	
				if (ConfigInstance.Misc.NoRecoil && ConfigInstance.Misc.UnsafeFeat) {
					handle = TargetProcess.CreateScatterHandle();
					weapon->WriteRecoilPitch(handle, helditem->GetItemID(), ConfigInstance.Misc.RecoilX);
					weapon->WriteRecoilYaw(handle, helditem->GetItemID(), ConfigInstance.Misc.RecoilY);
					TargetProcess.ExecuteScatterWrite(handle);
					TargetProcess.CloseScatterHandle(handle);
				} else if (ConfigInstance.Misc.NoRecoilKMbox) {
					int weaponNum = 0;
					switch (helditem->GetItemID())
					{
					case 54612: //ak
						weaponNum = 1;
						break;

					case 55772: //lr
						weaponNum = 2;
						break;

					case 58539: //m249
						weaponNum = 3;
						break;

					case 58542: //hmlmg
						weaponNum = 4;
						break;

					case 58549: //mp5
						weaponNum = 5;
						break;

					case 58568: //thompson
						weaponNum = 6;
						break;
					default:
						weaponNum = 0;
						break;
					}
					LOG("\n %s", weapon_name[weaponNum].c_str());
					double ADSsens = 0.66;
					double recoilXPer = 0.80;
					double recoilYPer = 0.60;
					//double recoilXPer = ConfigInstance.Misc.RecoilXKMbox;
					//double recoilYPer = ConfigInstance.Misc.RecoilYKMbox;
					double fov = 90.0;
					int scopeNum = 0;
					int isFiring = km_left();
					int isAiming = km_right();
					if (isFiring == 1 && isAiming == 1) {
						for (int i = 0; i < recoil_tables[weaponNum].size(); i++) {
							float angleX = recoil_tables[weaponNum][i][0];
							float angleY = recoil_tables[weaponNum][i][1];
							printf("\n\nangleX:%f, angleY:%f", angleX, angleY);
							double xMoveABS;
							double yMoveABS;
							xMoveABS = (angleX * atta_multi[scopeNum]) / (-0.03 * (ADSsens) * 3.0 * (fov / 100.0));
							yMoveABS = (angleY * atta_multi[scopeNum]) / (-0.03 * (ADSsens) * 3.0 * (fov / 100.0));
							LOG("\nabsX:%f, absY:%f", xMoveABS, yMoveABS);
							double xMoveREL = xMoveABS * recoilXPer;
							double yMoveREL = yMoveABS * recoilYPer;
							LOG("\nrelX:%f, relY:%f", xMoveREL, yMoveREL);
							int isFiring = km_left();
							int isAiming = km_right();
							if (isFiring == 1 && isAiming == 1) {
								Sleep(akwait);
								km_move(xMoveREL, yMoveREL);
							}
							else if (isFiring == 0)
								break;
						}
					}
				}
			}

		}

	}

	if (ConfigInstance.Misc.AdminFlag && ConfigInstance.Misc.UnsafeFeat)
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
	if (ConfigInstance.Misc.BrightNights && ConfigInstance.Misc.UnsafeFeat)
	{
		Sky->WriteNightLightIntensity(handle, 25.0f);
		Sky->WriteNightAmbientMultiplier(handle, 4.0f);
	}

	if (ConfigInstance.Misc.BrightCaves && ConfigInstance.Misc.UnsafeFeat)
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

void main()
{
	string port = find_port("USB-SERIAL CH340"); // name of the kmbox port without ( COM )
	if (port.empty()) {
		std::cout << "\n	[!] no port found..";
		return;
	}
	if (!open_port(hSerial, port.c_str(), CBR_115200)) { // CBR_1115200 is the baud rate
		std::cout << "\n	[!] opening the port failed!";
		return;
	}
	printf("\n[+] Connected to the kmbox");
	printf("\n[+] Proceeding to Rust client \n");
	if (!TargetProcess.Init("RustClient.exe"))
	{
		printf("\n[!] Failed to initialize process");
		return;
	}
	TargetProcess.GetBaseAddress("GameAssembly.dll");
	TargetProcess.FixCr3();
	printf("\n[!] Found game");
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

