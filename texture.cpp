#include "DXUT.h"
#include "texture.h"
#include "WICTextureLoader.h"

HRESULT Texture::Init(ID3D11Device* pd3dDevice) {
	/**/
	if (!fileSelected) {
		OPENFILENAME ofn;
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.lpstrFile = m_pSrcFile;
		ofn.lpstrFile[0] = 0;
		ofn.lpstrFilter = TEXT("Í¼Æ¬ÎÄ¼ş\0*.jpg;*.bmp;*.png;*.tif\0");
		ofn.lpstrInitialDir = TEXT(".\\");
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if (!GetOpenFileName(&ofn)) return S_OK;
		fileSelected = true;
	}
	/**
	swprintf_s(m_pSrcFile, 256, L"test.png");
	/**/

	HRESULT hr;
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

HRESULT Texture::Render(ID3D11DeviceContext* pd3dImmediateContext) {
	pd3dImmediateContext->IASetVertexBuffers(0, 1, &m_pVB, &m_stride, &m_offset);
	pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	pd3dImmediateContext->PSSetShaderResources(0, 1, &m_pTextureRV);
	pd3dImmediateContext->Draw(4, 0);
	return S_OK;
}