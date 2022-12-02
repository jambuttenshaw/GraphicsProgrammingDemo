#include "Cubemap.h"

#include "stb_image.h"
#include <cassert>

Cubemap::Cubemap(ID3D11Device* device, const char* right, const char* left, const char* top, const char* bottom, const char* front, const char* back)
{
	const char* faces[6] = { right, left, top, bottom, front, back };
	Load(device, faces);
}

Cubemap::Cubemap(ID3D11Device* device, const char* faces[6])
{
	Load(device, faces);
}

Cubemap::~Cubemap()
{
	m_SRV->Release();
	m_CubemapTexture->Release();
}

void Cubemap::Load(ID3D11Device* device, const char* faces[6])
{
	unsigned char* faceData[6];
	int width, height;

	bool success = true;
	for (int i = 0; i < 6; i++)
	{
		int channels;
		faceData[i] = stbi_load(faces[i], &width, &height, &channels, 4);

		if (!faceData[i] || channels != 4)
		{
			assert(false && "Failed to load cubemap");

			success = false;

			// free any data that had been loaded before this point
			for (int j = 0; j < i; j++)
				stbi_image_free(faceData[j]);

			break;
		}
	}

	if (success)
	{
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 6;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
		srvDesc.TextureCube.MostDetailedMip = 0;

		D3D11_SUBRESOURCE_DATA pData[6];
		for (int i = 0; i < 6; i++)
		{
			pData[i].pSysMem = faceData[i];
			// each row takes up (4 * width) bytes, 'width' pixels of 4 channels at 1 byte per channel
			pData[i].SysMemPitch = width * 4;
			pData[i].SysMemSlicePitch = 0;
		}

		HRESULT hr = device->CreateTexture2D(&texDesc, &pData[0], &m_CubemapTexture);
		assert(hr == S_OK);

		hr = device->CreateShaderResourceView(m_CubemapTexture, &srvDesc, &m_SRV);
		assert(hr == S_OK);

		// free loaded image data
		for (int i = 0; i < 6; i++)
			stbi_image_free(faceData[i]);
	}
	else
	{
		// error
		assert(false && "Failed to load cubemap");
	}
}
