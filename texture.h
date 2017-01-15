#pragma once

struct SimpleVertex {
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 Tex;
};

class Texture {
public:
	Texture() : fileSelected(false) {}
	HRESULT Init(ID3D11Device* pd3dDevice);
	void OnD3D11DestroyDevice() { SAFE_RELEASE(m_pTextureRV); SAFE_RELEASE(m_pVB); }
	HRESULT Render(ID3D11DeviceContext* pImmediateContext);
private:
	bool fileSelected;
	TCHAR m_pSrcFile[256];
	ID3D11ShaderResourceView* m_pTextureRV;
	ID3D11Buffer *m_pVB;
	UINT m_stride, m_offset;
};