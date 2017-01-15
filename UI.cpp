#include "DXUT.h"
#include "UI.h"

CDXUTDialog UI::s_UI;
DirectX::XMFLOAT4 UI::s_vOffset(0.f, 0.f, 0.f, 0.f);
DirectX::XMFLOAT4 UI::s_vThreshold(-1.f, -1.f, 2.f, 2.f);

void UI::Init(CDXUTDialogResourceManager* pDialogResourceManager) {
	s_UI.Init(pDialogResourceManager);
	s_UI.SetCallback(UI::EventHandler); int iY = 10;

	s_UI.AddComboBox(IDC_AREA, 0, iY, 200, 30, L'R');
	s_UI.GetComboBox(IDC_AREA)->AddItem(L"(R)ange: All colors", (void*)0);
	s_UI.GetComboBox(IDC_AREA)->AddItem(L"(R)ange:        Red", (void*)1);
	s_UI.GetComboBox(IDC_AREA)->AddItem(L"(R)ange:     Yellow", (void*)2);
	s_UI.GetComboBox(IDC_AREA)->AddItem(L"(R)ange:      Green", (void*)3);
	s_UI.GetComboBox(IDC_AREA)->AddItem(L"(R)ange:       Cyan", (void*)4);
	s_UI.GetComboBox(IDC_AREA)->AddItem(L"(R)ange:       Blue", (void*)5);
	s_UI.GetComboBox(IDC_AREA)->AddItem(L"(R)ange:    Magenta", (void*)6);

	s_UI.AddEditBox(IDE_HUE, TEXT("0"), 150, iY += 168, 50, 30);
	s_UI.AddStatic(IDT_HUE, TEXT("Hue: "), 0, iY, 150, 20);
	s_UI.AddSlider(IDS_HUE, 0, iY += 24, 200, 20, -180, 180, 0);

	s_UI.AddEditBox(IDE_SATURATION, TEXT("0"), 150, iY += 24, 50, 30);
	s_UI.AddStatic(IDT_SATURATION, TEXT("Saturation: "), 0, iY, 150, 20);
	s_UI.AddSlider(IDS_SATURATION, 0, iY += 24, 200, 20, -100, 100, 0);

	s_UI.AddEditBox(IDE_INTENSITY, TEXT("0"), 150, iY += 24, 50, 30);
	s_UI.AddStatic(IDT_INTENSITY, TEXT("Lightness: "), 0, iY, 150, 20);
	s_UI.AddSlider(IDS_INTENSITY, 0, iY += 24, 200, 20, -100, 100, 0);
}

void UI::Reset(const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) {
	s_UI.SetLocation(pBackBufferSurfaceDesc->Width - 200,
		pBackBufferSurfaceDesc->Height - 350);
	s_UI.SetSize(170, 300);
}

void CALLBACK UI::EventHandler(UINT nEvent, int nControlID,
	CDXUTControl* pControl, void*) {
	wchar_t str[8];
	int value = 0, id = IDT_HUE;
	if (nEvent == EVENT_SLIDER_VALUE_CHANGED) {
		value = ((CDXUTSlider*)pControl)->GetValue();
		switch (nControlID)
		{
		case IDS_HUE:
			id = IDE_HUE;
			s_vOffset.x = value / 360.f;
			break;
		case IDS_SATURATION:
			id = IDE_SATURATION;
			s_vOffset.y = value / 100.f;
			break;
		case IDS_INTENSITY:
			id = IDE_INTENSITY;
			s_vOffset.z = value / 100.f;
			break;
		}
		swprintf_s(str, 8, L"%d", value);
		s_UI.GetEditBox(id)->SetText(str);
	}
	else if (nEvent == EVENT_EDITBOX_CHANGE) {
		value = _wtoi(((CDXUTEditBox*)pControl)->GetText());
		switch (nControlID)
		{
		case IDE_HUE:
			id = IDS_HUE;
			if (value < -180) value = -180;
			else if (value > 180) value = 180;
			s_vOffset.x = value / 360.f;
			break;
		case IDE_SATURATION:
			id = IDS_SATURATION;
			if (value < -100) value = -100;
			else if (value > 100) value = 100;
			s_vOffset.y = value / 100.f;
			break;
		case IDE_INTENSITY:
			id = IDS_INTENSITY;
			if (value < -100) value = -100;
			else if (value > 100) value = 100;
			s_vOffset.z = value / 100.f;
			break;
		}
		swprintf_s(str, 8, L"%d", value);
		s_UI.GetSlider(id)->SetValue(value);
		((CDXUTEditBox*)pControl)->SetText(str);
	}
	else if (nEvent == EVENT_COMBOBOX_SELECTION_CHANGED) {
		switch ((size_t)((CDXUTComboBox*)pControl)->GetSelectedData()) {
		case 0:
			s_vThreshold.x = s_vThreshold.y = -1.f;
			s_vThreshold.z = s_vThreshold.w = 2.f;
			break;
		case 1:
			s_vThreshold.x = 315.f / 360; s_vThreshold.y = 345.f / 360;
			s_vThreshold.z = 15.f / 360; s_vThreshold.w = 45.f / 360;
			break;
		case 2:
			s_vThreshold.x = 15.f / 360; s_vThreshold.y = 45.f / 360;
			s_vThreshold.z = 75.f / 360; s_vThreshold.w = 105.f / 360;
			break;
		case 3:
			s_vThreshold.x = 75.f / 360; s_vThreshold.y = 105.f / 360;
			s_vThreshold.z = 135.f / 360; s_vThreshold.w = 165.f / 360;
			break;
		case 4:
			s_vThreshold.x = 135.f / 360; s_vThreshold.y = 165.f / 360;
			s_vThreshold.z = 195.f / 360; s_vThreshold.w = 225.f / 360;
			break;
		case 5:
			s_vThreshold.x = 195.f / 360; s_vThreshold.y = 225.f / 360;
			s_vThreshold.z = 255.f / 360; s_vThreshold.w = 285.f / 360;
			break;
		case 6:
			s_vThreshold.x = 255.f / 360; s_vThreshold.y = 285.f / 360;
			s_vThreshold.z = 315.f / 360; s_vThreshold.w = 345.f / 360;
			break;
		}
	}
}
