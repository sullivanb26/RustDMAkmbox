#include "pch.h"
#include "GUI.h"
#include "entity.h"
#include "Form.h"
#include "Button.h"
#include "ColourPicker.h"
#include "Label.h"
#include "tab.h"
#include "TabController.h"
#include "Toggle.h"
#include "Slider.h"
#include "DropDown.h"
#include "ComboBox.h"
#include "KeyBind.h"
#include "TabListBox.h"
#include "TabListBoxController.h"
#include "TextBox.h"
#include "ConfigInstance.h"
#include "OcclusionCulling.h"
#include "ConvarAdmin.h"
#include "ConvarGraphics.h"
#include "ConfigInstance.h"
int SelectedTab = 1;
int TabCount = 0;
int KeyBindClipBoard = 0;
int ConfigNumber = 0;
EntityVector MenuEntity;
bool MenuOpen = true;
D2D1::ColorF ColourPickerClipBoard = Colour(255, 255, 255);
D2D1::ColorF ColourPick = Colour(0, 150, 255, 255);

void CreateGUI()
{
	MenuEntity = std::make_shared<Container>();
	auto form = std::make_shared<Form>(100, 100.0f, 420, 350, 2, 30, LIT(L"Rust DMA"), false);
	{
		auto tabcontroller = std::make_shared<TabController>();
		form->Push(tabcontroller);

		auto safeTab = std::make_shared<Tab>(L"Safe", 5, 55, &SelectedTab, 50, 20);
		{
			auto enableplayeresp = std::make_shared<Toggle>(10, 10, LIT(L"Enable Player ESP"), &ConfigInstance.PlayerESP.Enable);
			safeTab->Push(enableplayeresp);
			auto playercolour = std::make_shared<ColourPicker>(150, 10, &ConfigInstance.PlayerESP.Colour);
			safeTab->Push(playercolour);
			auto playername = std::make_shared<Toggle>(10, 30, LIT(L"Player Name"), &ConfigInstance.PlayerESP.Name);
			safeTab->Push(playername);
			auto playerbox = std::make_shared<Toggle>(10, 50, LIT(L"Box"), &ConfigInstance.PlayerESP.Box);
			safeTab->Push(playerbox);
			auto playerboxwidth = std::make_shared<Slider<int>>(10, 70, 150, LIT(L"Width"), LIT(L"px"), 1, 10, &ConfigInstance.PlayerESP.Width);
			safeTab->Push(playerboxwidth);
			auto playerdistance = std::make_shared<Toggle>(10, 90, LIT(L"Player Distance"), &ConfigInstance.PlayerESP.Distance);
			safeTab->Push(playerdistance);
			auto playermaxdistance = std::make_shared<Slider<int>>(10, 110, 150, LIT(L"Max Distance"), LIT(L"m"), 0, 1000, &ConfigInstance.PlayerESP.MaxDistance);
			safeTab->Push(playermaxdistance);
			auto recoilKM = std::make_shared<Toggle>(10, 130, LIT(L"No Recoil KMBox"), &ConfigInstance.Misc.NoRecoilKMbox);
			safeTab->Push(recoilKM);
			auto recoilxKM = std::make_shared<Slider<int>>(10, 150, 150, LIT(L"Recoil X"), LIT(L"%"), 0, 100, &ConfigInstance.Misc.RecoilXKMbox);
			safeTab->Push(recoilxKM);
			auto recoilyKM = std::make_shared<Slider<int>>(10, 170, 150, LIT(L"Recoil Y"), LIT(L"%"), 0, 100, &ConfigInstance.Misc.RecoilYKMbox);
			safeTab->Push(recoilyKM);
			auto aimKM = std::make_shared<Toggle>(10, 190, LIT(L"KMBox Aimbot"), &ConfigInstance.Misc.aimKMbox);
			safeTab->Push(aimKM);
			auto aimKey = std::make_shared<KeyBind>(10, 210, L"KeyBind", &ConfigInstance.Misc.aimKeyKM);
			safeTab->Push(aimKey);
			auto aimSmoothKM = std::make_shared<Slider<int>>(10, 230, 150, LIT(L"Smoothing"), LIT(L"%"), 0, 100, &ConfigInstance.Misc.smoothKM);
			safeTab->Push(aimSmoothKM);
			auto aimFOVKM = std::make_shared<Slider<int>>(10, 250, 150, LIT(L"FOV"), LIT(L"%"), 0, 180, &ConfigInstance.Misc.fovKM);
			safeTab->Push(aimFOVKM);
			auto fovCircle = std::make_shared<Toggle>(10, 270, LIT(L"FOV Circle"), &ConfigInstance.Misc.fovCircle);
			safeTab->Push(fovCircle);
		}
		auto unsafeTab = std::make_shared<Tab>(L"Unsafe", 65, 55, &SelectedTab, 50, 20);
		{
			auto unsafeFeatures = std::make_shared<Toggle>(10, 10, LIT(L"Enable Unsafe Features?"), &ConfigInstance.Misc.UnsafeFeat);
			unsafeTab->Push(unsafeFeatures);
			auto adminesp = std::make_shared<Toggle>(10, 35, LIT(L"Admin Box ESP"), &ConfigInstance.Misc.AdminESP);
			adminesp->SetValueChangedEvent([]()
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
				});
			unsafeTab->Push(adminesp);
			auto watereffect = std::make_shared<Toggle>(10, 55, LIT(L"Remove Water Effect"), &ConfigInstance.Misc.RemoveWaterEffect);
			watereffect->SetValueChangedEvent([]()
				{
					std::shared_ptr<ConvarAdmin> convaradmin = std::make_shared<ConvarAdmin>();
					if (ConfigInstance.Misc.RemoveWaterEffect && ConfigInstance.Misc.UnsafeFeat)
						convaradmin->ClearVisionInWater(true);
				});
			unsafeTab->Push(watereffect);
			auto adminflag = std::make_shared<Toggle>(10, 75, LIT(L"Admin Flag"), &ConfigInstance.Misc.AdminFlag);
			unsafeTab->Push(adminflag);
			auto changetime = std::make_shared<Toggle>(10, 95, LIT(L"Change Time"), &ConfigInstance.Misc.ChangeTime);
			changetime->SetValueChangedEvent([]()
				{
					std::shared_ptr<ConvarAdmin> convaradmin = std::make_shared<ConvarAdmin>();
					if (ConfigInstance.Misc.ChangeTime && ConfigInstance.Misc.UnsafeFeat)
						convaradmin->SetAdminTime(ConfigInstance.Misc.Time);
					else
						convaradmin->SetAdminTime(-1);
				});
			unsafeTab->Push(changetime);
			auto time = std::make_shared<Slider<int>>(10, 115, 150, LIT(L"Time"), LIT(L"°"), 0, 24, &ConfigInstance.Misc.Time);
			time->SetValueChangedEvent([]()
				{
					std::shared_ptr<ConvarAdmin> convaradmin = std::make_shared<ConvarAdmin>();
					if (ConfigInstance.Misc.ChangeTime && ConfigInstance.Misc.UnsafeFeat)
						convaradmin->SetAdminTime(ConfigInstance.Misc.Time);
					else if (ConfigInstance.Misc.UnsafeFeat)
						convaradmin->SetAdminTime(-1);
				});
			unsafeTab->Push(time);
			auto changefov = std::make_shared<Toggle>(10, 140, LIT(L"Change FOV"), &ConfigInstance.Misc.ChangeFov);
			changefov->SetValueChangedEvent([]()
				{
					std::shared_ptr<ConvarGraphics> graphics = std::make_shared<ConvarGraphics>();
					if (ConfigInstance.Misc.ChangeFov && ConfigInstance.Misc.UnsafeFeat)
						graphics->WriteFOV(ConfigInstance.Misc.Fov);
				});
			unsafeTab->Push(changefov);
			auto fovamount = std::make_shared<Slider<int>>(10, 160, 150, LIT(L"FOV Amount"), LIT(L"°"), 0, 150, &ConfigInstance.Misc.Fov);
			fovamount->SetValueChangedEvent([]()
				{
					std::shared_ptr<ConvarGraphics> graphics = std::make_shared<ConvarGraphics>();
					if (ConfigInstance.Misc.ChangeFov && ConfigInstance.Misc.UnsafeFeat)
						graphics->WriteFOV(ConfigInstance.Misc.Fov);
				});
			unsafeTab->Push(fovamount);
			auto brightnights = std::make_shared<Toggle>(10, 185, LIT(L"Bright Nights"), &ConfigInstance.Misc.BrightNights);
			unsafeTab->Push(brightnights);
			auto brightcaves = std::make_shared<Toggle>(10, 205, LIT(L"Bright Caves"), &ConfigInstance.Misc.BrightCaves);
			unsafeTab->Push(brightcaves);
			auto norecoil = std::make_shared<Toggle>(10, 225, LIT(L"No Recoil"), &ConfigInstance.Misc.NoRecoil);
			unsafeTab->Push(norecoil);
			auto recoilx = std::make_shared<Slider<int>>(10, 245, 150, LIT(L"Recoil X"), LIT(L"%"), 0, 100, &ConfigInstance.Misc.RecoilX);
			unsafeTab->Push(recoilx);
			auto recoily = std::make_shared<Slider<int>>(10, 270, 150, LIT(L"Recoil Y"), LIT(L"%"), 0, 100, &ConfigInstance.Misc.RecoilY);
			unsafeTab->Push(recoily);
			auto spiderman = std::make_shared<Toggle>(10, 290, LIT(L"Spiderman"), &ConfigInstance.Misc.SpiderMan);
			unsafeTab->Push(spiderman);
		}
		auto configTab = std::make_shared<Tab>(L"Config", 125, 55, &SelectedTab, 50, 20);
		{
			std::list<std::wstring> configValues = {L"1", L"2", L"3", L"4"};
			auto configDropdown = std::make_shared<DropDown>(10, 30, L"Config", &ConfigNumber, configValues);
			configTab->Push(configDropdown);
			auto loadConfig = std::make_shared<Button>(10, 50, L"Load", []{
				ConfigInstances.FromJson(&ConfigNumber);
			});
			configTab->Push(loadConfig);
			auto saveConfig = std::make_shared<Button>(10, 70, L"Save", []{
				ConfigInstances.ToJson(&ConfigNumber);
			});
			configTab->Push(saveConfig);
		}
		tabcontroller->Push(safeTab);
		tabcontroller->Push(unsafeTab);
		tabcontroller->Push(configTab);
	}

	MenuEntity->Push(form);
	MenuEntity->Draw();
	MenuEntity->Update();
}

void SetFormPriority()
{
	// This sorts the host container (containerptr) which contains forms, as long as a form isn't parented to another form then this will allow it to draw over when clicked.
	// I swear to god if i need to make this work for forms inside forms for some odd reason in the future then i am going to throw a monitor out the window.
	std::sort(MenuEntity->GetContainer().begin(), MenuEntity->GetContainer().end(),
		[](child a, child b) { return b->GetLastClick() < a->GetLastClick(); }
	);
}

float LastOpen = 0;

void Render()
{
	if (IsKeyClicked(VK_INSERT) && LastOpen < clock() * 0.00001f)
	{
		LastOpen = (clock() * 0.00001f) + 0.002f;
		MenuOpen = !MenuOpen;
	}

	MenuEntity->Draw();
	MenuEntity->GetContainer()[0]->Update(); // only allow stretching,dragging and other update stuff if it is the main form, prevents dragging and sizing the wrong forms.
	SetFormPriority();
}
