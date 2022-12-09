#include "Cubemap.h"

#include "stb_image.h"


XMFLOAT3 Cubemap::s_FaceNormals[6] = {
		{ 1.0f, 0.0f, 0.0f },
		{ -1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, -1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f },
		{ 0.0f, 0.0f, -1.0f }
};
XMFLOAT3 Cubemap::s_FaceTangents[6] = {
	{ 0.0f, 0.0f, -1.0f },
	{ 0.0f, 0.0f, 1.0f },
	{ 1.0f, 0.0f, 0.0f },
	{ 1.0f, 0.0f, 0.0f },
	{ 1.0f, 0.0f, 0.0f },
	{ -1.0f, 0.0f, 0.0f }
};
XMFLOAT3 Cubemap::s_FaceBitangents[6] = {
	{ 0.0f, -1.0f, 0.0f },
	{ 0.0f, -1.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f },
	{ 0.0f, 0.0f, -1.0f },
	{ 0.0f, -1.0f, 0.0f },
	{ 0.0f, -1.0f, 0.0f }
};


Cubemap::Cubemap(ID3D11Device* device, unsigned int size, unsigned int mipLevels, DXGI_FORMAT format, DXGI_FORMAT srvFormat)
{
	m_ReadOnly = false;
	m_HasMips = mipLevels != 1;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = size;
	texDesc.Height = size;
	texDesc.MipLevels = mipLevels;
	texDesc.ArraySize = 6;
	texDesc.Format = format;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = srvFormat;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = m_HasMips ? -1 : texDesc.MipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;

	HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &m_CubemapTexture);
	assert(hr == S_OK);

	hr = device->CreateShaderResourceView(m_CubemapTexture, &srvDesc, &m_SRV);
	assert(hr == S_OK);

	{
		if (m_HasMips)
		{
			for (unsigned int mip = 0; mip < mipLevels; mip++) CreateUAVs(device, mip);
		}
		else
		{
			CreateUAVs(device);
		}
	}
}

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
	for (auto& uav : m_UAVs)
	{
		if (uav) uav->Release();
	}
}

ID3D11UnorderedAccessView* Cubemap::GetUAV(int face, int mip) const
{
	assert(!m_ReadOnly && "Cubemap is read only!");
	
	if (m_HasMips)
	{
		return m_UAVs[(6 * mip) + face];
	}
	else
	{
		return m_UAVs[face];
	}
}

void Cubemap::Load(ID3D11Device* device, const char* faces[6])
{
	unsigned char* faceData[6];
	int width, height, channels;

	bool success = true;
	for (int i = 0; i < 6; i++)
	{
		faceData[i] = stbi_load(faces[i], &width, &height, &channels, 4);

		if (!faceData[i] || channels < 4)
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
			pData[i].SysMemPitch = width * channels;
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

void Cubemap::CreateUAVs(ID3D11Device* device, unsigned int mipSlice)
{
	for (int i = 0; i < 6; i++)
	{
		ID3D11UnorderedAccessView* uav = nullptr;

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.MipSlice = mipSlice;
		uavDesc.Texture2DArray.ArraySize = 1;
		uavDesc.Texture2DArray.FirstArraySlice = i;
		HRESULT hr = device->CreateUnorderedAccessView(m_CubemapTexture, &uavDesc, &uav);
		assert(hr == S_OK);

		m_UAVs.push_back(uav);
	}
}
