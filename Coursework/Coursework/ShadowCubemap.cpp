#include "ShadowCubeMap.h"

ShadowCubemap::ShadowCubemap(ID3D11Device* device, unsigned int resolution)
	: Cubemap(device, resolution, true, 1, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT, D3D11_BIND_DEPTH_STENCIL)
{
	HRESULT hr;

	// create a RTV for each face
	for (int i = 0; i < 6; i++)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.ArraySize = 1;
		dsvDesc.Texture2DArray.FirstArraySlice = i;
		dsvDesc.Texture2DArray.MipSlice = 0;
		dsvDesc.Flags = 0;
		hr = device->CreateDepthStencilView(m_CubemapTexture, &dsvDesc, &m_DSVs[i]);
		assert(hr == S_OK);
	}

	m_Viewport.Width =  static_cast<float>(resolution);
	m_Viewport.Height = static_cast<float>(resolution);
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;
	m_Viewport.TopLeftX = 0.0f;
	m_Viewport.TopLeftY = 0.0f;
}

ShadowCubemap::~ShadowCubemap()
{
	for (int i = 0; i < 6; i++)
		m_DSVs[i]->Release();
}

void ShadowCubemap::BindDSV(ID3D11DeviceContext* deviceContext, int face)
{
	deviceContext->RSSetViewports(1, &m_Viewport);
	
	ID3D11RenderTargetView* nullRTV = nullptr;
	ID3D11DepthStencilView* dsv = GetDSV(face);

	deviceContext->OMSetRenderTargets(1, &nullRTV, dsv);
	deviceContext->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
