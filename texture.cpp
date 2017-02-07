#include "DXUT.h"
#include "texture.h"
#include "WICTextureLoader.h"
#include "SDKmisc.h"

Texture::Texture() 
: fileSelected(false)
, m_pVertexShader11(nullptr)
, m_pPixelShader11(nullptr)
, m_pLayout11(nullptr)
, m_pSamLinear(nullptr)
, m_pcbVS(nullptr)
, m_pcbPS(nullptr)
{}

void Texture::OnD3D11DestroyDevice() {
	SAFE_RELEASE(m_pTextureRV);
	SAFE_RELEASE(m_pVB);

	SAFE_RELEASE(m_pVertexShader11);
	SAFE_RELEASE(m_pPixelShader11);
	SAFE_RELEASE(m_pLayout11);
	SAFE_RELEASE(m_pSamLinear);

	SAFE_RELEASE(m_pcbVS);
	SAFE_RELEASE(m_pcbPS);
}

HRESULT Texture::Init(ID3D11Device* pd3dDevice) {
	/**/
	if (!fileSelected) {
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = m_pSrcFile;
		ofn.lpstrFile[0] = 0;
		ofn.lpstrFilter = TEXT("Í¼Æ¬ÎÄ¼þ\0*.jpg;*.bmp;*.png;*.tif\0");
		ofn.lpstrInitialDir = TEXT(".\\");
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if (!GetOpenFileName(&ofn)) return S_OK;
		fileSelected = true;
	}
	/**
	swprintf_s(m_pSrcFile, MAX_PATH, L"media\\lena.png");
	/**/

	HRESULT hr;
	m_pd3dDevice = pd3dDevice;
	V_RETURN(InitShader());

	// Create constant buffers
	D3D11_BUFFER_DESC cbDesc;
	ZeroMemory(&cbDesc, sizeof(cbDesc));
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	cbDesc.ByteWidth = sizeof(CBuffer_VS);
	V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, nullptr, &m_pcbVS));
	DXUT_SetDebugName(m_pcbVS, "CB_VS");

	cbDesc.ByteWidth = sizeof(CBuffer_PS);
	V_RETURN(pd3dDevice->CreateBuffer(&cbDesc, nullptr, &m_pcbPS));
	DXUT_SetDebugName(m_pcbPS, "CB_PS");

	V_RETURN(DirectX::CreateWICTextureFromFileEx(pd3dDevice, nullptr, m_pSrcFile, 0,
		D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, true,
		nullptr, &m_pTextureRV));
	
	ID3D11Texture2D *texture2d = 0;
	ID3D11Resource *res;
	D3D11_TEXTURE2D_DESC desc;
	m_pTextureRV->GetResource(&res);
	V_RETURN(res->QueryInterface(&texture2d));
	texture2d->GetDesc(&desc);
	float y = (float)desc.Height / desc.Width;
	SAFE_RELEASE(res);
	SAFE_RELEASE(texture2d);


	SimpleVertex vertices[] =
	{
		{ DirectX::XMFLOAT3(-1.f, -y, 0.f), DirectX::XMFLOAT2(0.f, 1.f) },
		{ DirectX::XMFLOAT3(-1.f,  y, 0.f), DirectX::XMFLOAT2(0.f, 0.f) },
		{ DirectX::XMFLOAT3( 1.f, -y, 0.f), DirectX::XMFLOAT2(1.f, 1.f) },
		{ DirectX::XMFLOAT3( 1.f,  y, 0.f), DirectX::XMFLOAT2(1.f, 0.f) }
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;
	V_RETURN(pd3dDevice->CreateBuffer(&bd, &InitData, &m_pVB));
	m_stride = sizeof(SimpleVertex);
	m_offset = 0;
	return S_OK;
}

HRESULT Texture::InitShader()
{
	HRESULT hr;
	SAFE_RELEASE(m_pVertexShader11);
	SAFE_RELEASE(m_pPixelShader11);
	SAFE_RELEASE(m_pLayout11);
	SAFE_RELEASE(m_pSamLinear);

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
	V_RETURN(m_pd3dDevice->CreateVertexShader(pVertexShaderBuffer->GetBufferPointer(),
		pVertexShaderBuffer->GetBufferSize(), nullptr, &m_pVertexShader11));
	DXUT_SetDebugName(g_pVertexShader11, "RenderSceneVS");

	V_RETURN(m_pd3dDevice->CreatePixelShader(pPixelShaderBuffer->GetBufferPointer(),
		pPixelShaderBuffer->GetBufferSize(), nullptr, &m_pPixelShader11));
	DXUT_SetDebugName(g_pPixelShader11, "RenderScenePS");

	// Create a layout for the object data
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	V_RETURN(m_pd3dDevice->CreateInputLayout(layout, ARRAYSIZE(layout), pVertexShaderBuffer->GetBufferPointer(),
		pVertexShaderBuffer->GetBufferSize(), &m_pLayout11));
	DXUT_SetDebugName(m_pLayout11, "Primary");

	// No longer need the shader blobs
	SAFE_RELEASE(pVertexShaderBuffer);
	SAFE_RELEASE(pPixelShaderBuffer);

	// Create state objects
	D3D11_SAMPLER_DESC samDesc;
	ZeroMemory(&samDesc, sizeof(samDesc));
	samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samDesc.MaxAnisotropy = 1;
	samDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samDesc.MaxLOD = D3D11_FLOAT32_MAX;
	V_RETURN(m_pd3dDevice->CreateSamplerState(&samDesc, &m_pSamLinear));
	DXUT_SetDebugName(g_pSamLinear, "Linear");

	return S_OK;
}

HRESULT Texture::Render(ID3D11DeviceContext* pd3dImmediateContext, DirectX::XMMATRIX& mWorldViewProjection,
	DirectX::XMFLOAT4& offset, DirectX::XMFLOAT4& threshold) {
	// Set the constant buffers
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE MappedResource;

	V(pd3dImmediateContext->Map(m_pcbVS, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	auto pVS = reinterpret_cast<CBuffer_VS*>(MappedResource.pData);
	XMStoreFloat4x4(&pVS->m_mWorldViewProjection, XMMatrixTranspose(mWorldViewProjection));
	pd3dImmediateContext->Unmap(m_pcbVS, 0);
	pd3dImmediateContext->VSSetConstantBuffers(0, 1, &m_pcbVS);

	V(pd3dImmediateContext->Map(m_pcbPS, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	auto pPS = reinterpret_cast<CBuffer_PS*>(MappedResource.pData);
	pPS->m_offset = offset;
	pPS->m_threshold = threshold;
	pd3dImmediateContext->Unmap(m_pcbPS, 0);
	pd3dImmediateContext->PSSetConstantBuffers(1, 1, &m_pcbPS);

	pd3dImmediateContext->IASetInputLayout(m_pLayout11);
	pd3dImmediateContext->VSSetShader(m_pVertexShader11, nullptr, 0);
	pd3dImmediateContext->PSSetShader(m_pPixelShader11, nullptr, 0);
	pd3dImmediateContext->PSSetSamplers(0, 1, &m_pSamLinear);

	pd3dImmediateContext->IASetVertexBuffers(0, 1, &m_pVB, &m_stride, &m_offset);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pd3dImmediateContext->PSSetShaderResources(0, 1, &m_pTextureRV);
	pd3dImmediateContext->Draw(4, 0);
	return S_OK;
}
