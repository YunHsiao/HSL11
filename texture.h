#pragma once

struct SimpleVertex {
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 Tex;
};

// Constant buffers
#pragma pack(push,1)
struct CBuffer_VS
{
	DirectX::XMFLOAT4X4 m_mWorldViewProjection;
};
struct CBuffer_PS
{
	DirectX::XMFLOAT4 m_offset;
	DirectX::XMFLOAT4 m_threshold;
};
#pragma pack(pop)

class Texture {
public:
	Texture();
	HRESULT Init(ID3D11Device* pd3dDevice);
	void OnD3D11DestroyDevice();
	HRESULT Render(ID3D11DeviceContext* pImmediateContext, DirectX::XMMATRIX& mWorldViewProjection, 
		DirectX::XMFLOAT4& offset, DirectX::XMFLOAT4& threshold);
	HRESULT InitShader();
	HRESULT InitTexture();
private:
	TCHAR m_pSrcFile[MAX_PATH];
	ID3D11ShaderResourceView* m_pTextureRV;
	ID3D11Buffer *m_pVB;
	UINT m_stride, m_offset;

	ID3D11Device* m_pd3dDevice;
	ID3D11VertexShader* m_pVertexShader11;
	ID3D11PixelShader* m_pPixelShader11;
	ID3D11InputLayout* m_pLayout11;
	ID3D11SamplerState* m_pSamLinear;

	ID3D11Buffer* m_pcbVS;
	ID3D11Buffer* m_pcbPS;
};