#pragma once

#include <d3d11.h>
#include <directxmath.h>


class RenderTarget
{
public:
	RenderTarget(ID3D11Device* device, unsigned int width, unsigned int height);
	~RenderTarget();

	void Clear(ID3D11DeviceContext* deviceContext, const DirectX::XMFLOAT4& colour);
	void Set(ID3D11DeviceContext* deviceContext);

	inline ID3D11ShaderResourceView* GetColourSRV() const { return m_ColourSRV; }
	inline ID3D11ShaderResourceView* GetDepthSRV() const { return m_DepthSRV; }

	inline unsigned int GetWidth() const { return m_Width; }
	inline unsigned int GetHeight() const { return m_Height; }

private:
	unsigned int m_Width = 0, m_Height = 0;

	ID3D11RenderTargetView* m_RenderTargetView = nullptr;
	ID3D11DepthStencilView* m_DepthStencilView = nullptr;

	ID3D11ShaderResourceView* m_ColourSRV = nullptr;
	ID3D11ShaderResourceView* m_DepthSRV = nullptr;

};
