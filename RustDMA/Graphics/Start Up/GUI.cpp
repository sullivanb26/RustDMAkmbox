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
int SelectedTab = 1;
int TabCount = 0;
int KeyBindClipBoard = 0;
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

		auto mainTab = std::make_shared<Tab>(L"Main", 5, 55, &SelectedTab, 50, 20);
		{
			auto safeLabel = std::make_shared<Label>(L"Safe Features", 10, 10);
			mainTab->Push(safeLabel);
			auto enableplayeresp = std::make_shared<Toggle>(10, 30, LIT(L"Enable Player ESP"), &ConfigInstance.PlayerESP.Enable);
			mainTab->Push(enableplayeresp);
			auto playercolour = std::make_shared<ColourPicker>(150, 30, &ConfigInstance.PlayerESP.Colour);
			mainTab->Push(playercolour);
			auto playername = std::make_shared<Toggle>(10, 50, LIT(L"Player Name"), &ConfigInstance.PlayerESP.Name);
			mainTab->Push(playername);
			auto playerdistance = std::make_shared<Toggle>(10, 70, LIT(L"Player Distance"), &ConfigInstance.PlayerESP.Distance);
			mainTab->Push(playerdistance);
			auto playermaxdistance = std::make_shared<Slider<int>>(10, 90, 150, LIT(L"Max Distance"), LIT(L"m"), 0, 1000, &ConfigInstance.PlayerESP.MaxDistance);
			mainTab->Push(playermaxdistance);
			auto recoilKM = std::make_shared<Toggle>(10, 110, LIT(L"No Recoil KMBox"), &ConfigInstance.Misc.NoRecoilKMbox);
			mainTab->Push(recoilKM);
			auto recoilxKM = std::make_shared<Slider<int>>(10, 130, 150, LIT(L"Recoil X"), LIT(L"%"), 0, 100, &ConfigInstance.Misc.RecoilXKMbox);
			mainTab->Push(recoilxKM);
			auto recoilyKM = std::make_shared<Slider<int>>(10, 150, 150, LIT(L"Recoil Y"), LIT(L"%"), 0, 100, &ConfigInstance.Misc.RecoilYKMbox);
			mainTab->Push(recoilyKM);


			auto unsafeLabel = std::make_shared<Label>(L"Unsafe Features", 180, 10);
			mainTab->Push(unsafeLabel);
			auto unsafeFeatures = std::make_shared<Toggle>(180, 35, LIT(L"Enable Unsafe Features?"), &ConfigInstance.Misc.UnsafeFeat);
			mainTab->Push(unsafeFeatures);
			auto adminesp = std::make_shared<Toggle>(180, 60, LIT(L"Admin Box ESP"), &ConfigInstance.Misc.AdminESP);
			adminesp->SetValueChangedEvent([]()
				{
					std::shared_ptr<OcclusionCulling> occlusionculling = std::make_shared<OcclusionCulling>();
					if (ConfigInstance.Misc.AdminESP && ConfigInstance.Misc.UnsafeFeat)
					{
						occlusionculling->WriteDebugSettings(DebugFilter::Dynamic);
						occlusionculling->WriteLayerMask(131072);
					}
					else
					{
						occlusionculling->WriteDebugSettings(DebugFilter::Off);
						occlusionculling->WriteLayerMask(0);
					}
				});
			mainTab->Push(adminesp);
			auto watereffect = std::make_shared<Toggle>(180, 80, LIT(L"Remove Water Effect"), &ConfigInstance.Misc.RemoveWaterEffect);
			watereffect->SetValueChangedEvent([]()
				{
					std::shared_ptr<ConvarAdmin> convaradmin = std::make_shared<ConvarAdmin>();
					if (ConfigInstance.Misc.RemoveWaterEffect && ConfigInstance.Misc.UnsafeFeat)
						convaradmin->ClearVisionInWater(true);
				});
			mainTab->Push(watereffect);
			auto adminflag = std::make_shared<Toggle>(180, 100, LIT(L"Admin Flag"), &ConfigInstance.Misc.AdminFlag);
			mainTab->Push(adminflag);
			auto changetime = std::make_shared<Toggle>(180, 120, LIT(L"Change Time"), &ConfigInstance.Misc.ChangeTime);
			changetime->SetValueChangedEvent([]()
				{
					std::shared_ptr<ConvarAdmin> convaradmin = std::make_shared<ConvarAdmin>();
					if (ConfigInstance.Misc.ChangeTime && ConfigInstance.Misc.UnsafeFeat)
						convaradmin->SetAdminTime(ConfigInstance.Misc.Time);
					else
						convaradmin->SetAdminTime(-1);
				});
			mainTab->Push(changetime);
			auto time = std::make_shared<Slider<int>>(180, 140, 150, LIT(L"Time"), LIT(L"°"), 0, 24, &ConfigInstance.Misc.Time);
			time->SetValueChangedEvent([]()
				{
					std::shared_ptr<ConvarAdmin> convaradmin = std::make_shared<ConvarAdmin>();
					if (ConfigInstance.Misc.ChangeTime && ConfigInstance.Misc.UnsafeFeat)
						convaradmin->SetAdminTime(ConfigInstance.Misc.Time);
					else
						convaradmin->SetAdminTime(-1);
				});
			mainTab->Push(time);
			auto changefov = std::make_shared<Toggle>(180, 165, LIT(L"Change FOV"), &ConfigInstance.Misc.ChangeFov);
			changefov->SetValueChangedEvent([]()
				{
					std::shared_ptr<ConvarGraphics> graphics = std::make_shared<ConvarGraphics>();
					if (ConfigInstance.Misc.ChangeFov && ConfigInstance.Misc.UnsafeFeat)
						graphics->WriteFOV(ConfigInstance.Misc.Fov);
				});
			mainTab->Push(changefov);
			auto fovamount = std::make_shared<Slider<int>>(180, 185, 150, LIT(L"FOV Amount"), LIT(L"°"), 0, 150, &ConfigInstance.Misc.Fov);
			fovamount->SetValueChangedEvent([]()
				{
					std::shared_ptr<ConvarGraphics> graphics = std::make_shared<ConvarGraphics>();
					if (ConfigInstance.Misc.ChangeFov && ConfigInstance.Misc.UnsafeFeat)
						graphics->WriteFOV(ConfigInstance.Misc.Fov);
				});
			mainTab->Push(fovamount);
			auto brightnights = std::make_shared<Toggle>(180, 210, LIT(L"Bright Nights"), &ConfigInstance.Misc.BrightNights);
			mainTab->Push(brightnights);
			auto brightcaves = std::make_shared<Toggle>(180, 230, LIT(L"Bright Caves"), &ConfigInstance.Misc.BrightCaves);
			mainTab->Push(brightcaves);
			auto norecoil = std::make_shared<Toggle>(180, 250, LIT(L"No Recoil"), &ConfigInstance.Misc.NoRecoil);
			mainTab->Push(norecoil);
			auto recoilx = std::make_shared<Slider<int>>(180, 270, 150, LIT(L"Recoil X"), LIT(L"%"), 0, 100, &ConfigInstance.Misc.RecoilX);
			mainTab->Push(recoilx);
			auto recoily = std::make_shared<Slider<int>>(180, 295, 150, LIT(L"Recoil Y"), LIT(L"%"), 0, 100, &ConfigInstance.Misc.RecoilY);
			mainTab->Push(recoily);
			auto spiderman = std::make_shared<Toggle>(180, 315, LIT(L"Spiderman"), &ConfigInstance.Misc.SpiderMan);
			mainTab->Push(spiderman);
		}
		auto configTab = std::make_shared<Tab>(L"Config", 65, 55, &SelectedTab, 50, 20);
		{
		}
		tabcontroller->Push(mainTab);
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
