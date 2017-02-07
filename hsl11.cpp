//--------------------------------------------------------------------------------------
// File: hsl11.cpp
//
// Starting point for new Direct3D 11 Win32 desktop samples.  For a more minimal starting
// point, use the EmptyProject11 sample instead.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTgui.h"
#include "DXUTmisc.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"
#include "resource.h"
#include "texture.h"
#include "UI.h"

#pragma warning( disable : 4100 )

using namespace DirectX;

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
CModelViewerCamera          g_Camera;               // A model viewing camera
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg             g_SettingsDlg;          // Device settings dialog
CDXUTTextHelper*            g_pTxtHelper = nullptr;
CDXUTDialog                 g_HUD;                  // dialog for standard controls
ID3D11Device*				g_pd3dDevice;

// Direct3D 11 resources
ID3D11VertexShader*         g_pVertexShader11 = nullptr;
ID3D11PixelShader*          g_pPixelShader11 = nullptr;
ID3D11InputLayout*          g_pLayout11 = nullptr;
ID3D11SamplerState*         g_pSamLinear = nullptr;

Texture						g_photo;
UI							g_UI;

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------
#pragma pack(push,1)
struct CBuffer_VS
{
    XMFLOAT4X4  m_mWorldViewProjection;
};
struct CBuffer_PS
{
	XMFLOAT4    m_offset;
	XMFLOAT4    m_threshold;
};
#pragma pack(pop)

ID3D11Buffer* g_pcbVS = nullptr;
ID3D11Buffer* g_pcbPS = nullptr;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           2
#define IDC_CHANGEDEVICE        3
#define IDC_TOGGLEWARP          4
#define IDC_RELOADSHADER		5

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );

bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output,
                                       const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                 float fElapsedTime, void* pUserContext );

void InitApp();
void RenderText();


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#ifdef _DEBUG
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // DXUT will create and use the best device
    // that is available on the system depending on which D3D callbacks are set below

    // Set DXUT callbacks
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );

    InitApp();
    DXUTInit( true, true, nullptr ); // Parse the command line, show msgboxes on error, no extra command line params
    DXUTSetCursorSettings( true, true );
    DXUTCreateWindow( L"HSL" );

    // Only require 10-level hardware, change to D3D_FEATURE_LEVEL_11_0 to require 11-class hardware
    // Switch to D3D_FEATURE_LEVEL_9_x for 10level9 hardware
    DXUTCreateDevice( D3D_FEATURE_LEVEL_10_0, true, 800, 600 );

    DXUTMainLoop(); // Enter into the DXUT render loop

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    g_SettingsDlg.Init( &g_DialogResourceManager );
    g_HUD.Init( &g_DialogResourceManager );
    g_UI.Init( &g_DialogResourceManager );

    g_HUD.SetCallback( OnGUIEvent );
    int iY = 30;
    int iYo = 26;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 0, iY, 170, 22 );
    g_HUD.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", 0, iY += iYo, 170, 22, VK_F2 );
    g_HUD.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", 0, iY += iYo, 170, 22, VK_F3 );
    g_HUD.AddButton( IDC_TOGGLEWARP, L"Toggle WARP (F4)", 0, iY += iYo, 170, 22, VK_F4 );
    g_HUD.AddButton( IDC_RELOADSHADER, L"Reload Shader (F5)", 0, iY += iYo, 170, 22, VK_F5 );
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text.
//--------------------------------------------------------------------------------------
void RenderText()
{
    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos( 5, 5 );
    g_pTxtHelper->SetForegroundColor( Colors::Yellow );
    g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );
    g_pTxtHelper->End();
}


//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output,
                                       const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}

HRESULT InitShader( ID3D11Device* pd3dDevice )
{
	HRESULT hr;
	SAFE_RELEASE(g_pVertexShader11);
	SAFE_RELEASE(g_pPixelShader11);
	SAFE_RELEASE(g_pLayout11);

	// Read the HLSL file
	// You should use the lowest possible shader profile for your shader to enable various feature levels. These
	// shaders are simple enough to work well within the lowest possible profile, and will run on all feature levels

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	ID3DBlob* pVertexShaderBuffer = nullptr;
	V_RETURN(DXUTCompileFromFile(L"hsl.hlsl", nullptr, "RenderSceneVS", "vs_4_0_level_9_1", dwShaderFlags, 0,
		&pVertexShaderBuffer));

	ID3DBlob* pPixelShaderBuffer = nullptr;
	V_RETURN(DXUTCompileFromFile(L"hsl.hlsl", nullptr, "RenderScenePS", "ps_4_0_level_9_3", dwShaderFlags, 0,
		&pPixelShaderBuffer));

	// Create the shaders
	V_RETURN(pd3dDevice->CreateVertexShader(pVertexShaderBuffer->GetBufferPointer(),
		pVertexShaderBuffer->GetBufferSize(), nullptr, &g_pVertexShader11));
	DXUT_SetDebugName(g_pVertexShader11, "RenderSceneVS");

	V_RETURN(pd3dDevice->CreatePixelShader(pPixelShaderBuffer->GetBufferPointer(),
		pPixelShaderBuffer->GetBufferSize(), nullptr, &g_pPixelShader11));
	DXUT_SetDebugName(g_pPixelShader11, "RenderScenePS");

	// Create a layout for the object data
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	V_RETURN(pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVertexShaderBuffer->GetBufferPointer(),
		pVertexShaderBuffer->GetBufferSize(), &g_pLayout11));
	DXUT_SetDebugName(g_pLayout11, "Primary");

	// No longer need the shader blobs
	SAFE_RELEASE(pVertexShaderBuffer);
	SAFE_RELEASE(pPixelShaderBuffer);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc,
                                     void* pUserContext )
{
    HRESULT hr;
	g_pd3dDevice = pd3dDevice;

    auto pd3dImmediateContext = DXUTGetD3D11DeviceContext();
    V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
    V_RETURN( g_SettingsDlg.OnD3D11CreateDevice( pd3dDevice ) );
    g_pTxtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15 );

	V_RETURN( InitShader( pd3dDevice ) );

    // Create state objects
    D3D11_SAMPLER_DESC samDesc;
    ZeroMemory( &samDesc, sizeof(samDesc) );
    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.MaxAnisotropy = 1;
    samDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samDesc.MaxLOD = D3D11_FLOAT32_MAX;
    V_RETURN( pd3dDevice->CreateSamplerState( &samDesc, &g_pSamLinear ) );
    DXUT_SetDebugName( g_pSamLinear, "Linear" );

    // Create constant buffers
    D3D11_BUFFER_DESC cbDesc;
    ZeroMemory( &cbDesc, sizeof(cbDesc) );
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	cbDesc.ByteWidth = sizeof(CBuffer_VS);
	V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, nullptr, &g_pcbVS));
	DXUT_SetDebugName(g_pcbVS, "CB_VS");

	cbDesc.ByteWidth = sizeof(CBuffer_PS);
	V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, nullptr, &g_pcbPS));
	DXUT_SetDebugName(g_pcbPS, "CB_PS");

    // Create other render resources here
	V_RETURN(g_photo.Init(pd3dDevice));

    // Setup the camera's view parameters
    static const XMVECTORF32 s_vecEye = { 0.0f, 0.0f, -5.0f, 0.f };
    g_Camera.SetViewParams( s_vecEye, g_XMZero );

    g_HUD.GetButton( IDC_TOGGLEWARP )->SetEnabled( true );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                         const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_SettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams( XM_PI / 4, fAspectRatio, 0.1f, 1000.0f );
    g_Camera.SetWindow( pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height );
    g_Camera.SetButtonMasks( MOUSE_LEFT_BUTTON, MOUSE_WHEEL, MOUSE_MIDDLE_BUTTON );

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_HUD.SetSize( 170, 170 );

	g_UI.Reset(pBackBufferSurfaceDesc);

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime,
                                 float fElapsedTime, void* pUserContext )
{
    // If the settings dialog is being shown, then render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }       

    auto pRTV = DXUTGetD3D11RenderTargetView();
    pd3dImmediateContext->ClearRenderTargetView( pRTV, Colors::MidnightBlue );

    // Clear the depth stencil
    auto pDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

    // Get the projection & view matrix from the camera class
    XMMATRIX mWorld = g_Camera.GetWorldMatrix();
    XMMATRIX mView = g_Camera.GetViewMatrix();
    XMMATRIX mProj = g_Camera.GetProjMatrix();
    XMMATRIX mWorldViewProjection = mWorld * mView * mProj;

    // Set the constant buffers
    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE MappedResource;

    V( pd3dImmediateContext->Map( g_pcbVS, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) );
    auto pVS = reinterpret_cast<CBuffer_VS*>( MappedResource.pData );
	XMStoreFloat4x4(&pVS->m_mWorldViewProjection, XMMatrixTranspose(mWorldViewProjection));
    pd3dImmediateContext->Unmap( g_pcbVS, 0 );
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &g_pcbVS);

	V(pd3dImmediateContext->Map(g_pcbPS, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	auto pPS = reinterpret_cast<CBuffer_PS*>(MappedResource.pData);
	pPS->m_offset = g_UI.getOffset();
	pPS->m_threshold = g_UI.getThreshold();
	pd3dImmediateContext->Unmap(g_pcbPS, 0);
	pd3dImmediateContext->PSSetConstantBuffers(1, 1, &g_pcbPS);

    // Set render resources
    pd3dImmediateContext->IASetInputLayout( g_pLayout11 );
    pd3dImmediateContext->VSSetShader( g_pVertexShader11, nullptr, 0 );
    pd3dImmediateContext->PSSetShader( g_pPixelShader11, nullptr, 0 );
    pd3dImmediateContext->PSSetSamplers( 0, 1, &g_pSamLinear );

    // Render objects here...
	g_photo.Render(pd3dImmediateContext);

    DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );
    g_HUD.OnRender( fElapsedTime );
    g_UI.OnRender( fElapsedTime );
    RenderText();
    DXUT_EndPerfEvent();

    static ULONGLONG timefirst = GetTickCount64();
    if ( GetTickCount64() - timefirst > 5000 )
    {    
        OutputDebugString( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
        OutputDebugString( L"\n" );
        timefirst = GetTickCount64();
    }
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11DestroyDevice();
    g_SettingsDlg.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_DELETE( g_pTxtHelper );

    SAFE_RELEASE( g_pVertexShader11 );
    SAFE_RELEASE( g_pPixelShader11 );
    SAFE_RELEASE( g_pLayout11 );
    SAFE_RELEASE( g_pSamLinear );

    // Delete additional render resources here...
	g_photo.OnD3D11DestroyDevice();
	SAFE_RELEASE(g_pcbVS);
	SAFE_RELEASE(g_pcbPS);
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene.  This is called regardless of which D3D API is used
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_UI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
			DXUTToggleFullScreen();
            break;
        case IDC_TOGGLEREF:
            DXUTToggleREF();
            break;
        case IDC_TOGGLEWARP:
            DXUTToggleWARP();
			break;
		case IDC_RELOADSHADER:
			InitShader( g_pd3dDevice );
			break;
        case IDC_CHANGEDEVICE:
            g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() );
			break;
    }
}
