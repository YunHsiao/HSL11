#pragma once
#include "DXUTgui.h"

#define IDC_AREA				10
#define IDS_HUE					1
#define IDS_SATURATION          2
#define IDS_INTENSITY           3
#define IDE_HUE					4
#define IDE_SATURATION          5
#define IDE_INTENSITY           6
#define IDT_HUE					7
#define IDT_SATURATION          8
#define IDT_INTENSITY           9

class UI {
public:
	void Init(CDXUTDialogResourceManager* pManager);
	void Reset(const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	HRESULT OnRender(float fElapsedTime) { return s_UI.OnRender(fElapsedTime); }
	bool MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return s_UI.MsgProc(hWnd, uMsg, wParam, lParam);
	}
	DirectX::XMFLOAT4 getOffset() { return s_vOffset; }
	DirectX::XMFLOAT4 getThreshold() { return s_vThreshold; }
private:
	static void CALLBACK EventHandler(UINT nEvent, int nControlID,
		CDXUTControl* pControl, void* pUserContext);
	static CDXUTDialog s_UI;
	static DirectX::XMFLOAT4 s_vOffset, s_vThreshold;
};