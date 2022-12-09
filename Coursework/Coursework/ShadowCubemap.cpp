#include "ShadowCubeMap.h"

ShadowCubemap::ShadowCubemap(ID3D11Device* device, unsigned int resolution)
	: Cubemap(device, resolution, 1, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT)
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
	}
}

ShadowCubemap::~ShadowCubemap()
{
	for (int i = 0; i < 6; i++)
		m_DSVs[i]->Release();
}
