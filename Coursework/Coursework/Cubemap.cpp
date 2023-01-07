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


Cubemap::Cubemap(ID3D11Device* device, unsigned int size, bool readOnly, unsigned int mipLevels, DXGI_FORMAT format, DXGI_FORMAT srvFormat, UINT bindFlags)
{
	// this constructor creates an empty cubemap that will be written to on the gpu

	m_ReadOnly = readOnly;
	m_HasMips = mipLevels != 1;
	m_TextureFormat = format;
	m_SRVFormat = srvFormat;

	// create texture
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = size;
	texDesc.Height = size;
	texDesc.MipLevels = mipLevels;
	texDesc.ArraySize = 6;
	texDesc.Format = m_TextureFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | ((!m_ReadOnly) ? D3D11_BIND_UNORDERED_ACCESS : 0) | bindFlags;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	// create srv
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = m_SRVFormat;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = m_HasMips ? -1 : texDesc.MipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;

	HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &m_CubemapTexture);
	assert(hr == S_OK);

	hr = device->CreateShaderResourceView(m_CubemapTexture, &srvDesc, &m_SRV);
	assert(hr == S_OK);

	// create a uav for each face (and mips if they are desired)
	if (!m_ReadOnly)
	{
		if (m_HasMips)
		{
			for (unsigned int mip = 0; mip < mipLevels; mip++) CreateUAVs(device, mip);
		}
		else
			CreateUAVs(device);
	}

	// create an srv for each face
	CreateFaceSRVs(device);
}

Cubemap::Cubemap(ID3D11Device* device, const char* right, const char* left, const char* top, const char* bottom, const char* front, const char* back)
{
	// this constructor loads a cubemap from 6 files
	const char* faces[6] = { right, left, top, bottom, front, back };
	Load(device, faces);
}

Cubemap::Cubemap(ID3D11Device* device, const char* faces[6])
{
	// this constructor loads a cubemap from 6 files
	Load(device, faces);
}

Cubemap::~Cubemap()
{
	m_CubemapTexture->Release();
	m_SRV->Release();
	for (auto& srv : m_FaceSRVs)
	{
		if (srv) srv->Release();
	} 
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
	// load image data from files
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
		// create texture
		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 6;
		texDesc.Format = m_TextureFormat;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		
		// setup data to initialise texture with
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

		// create srv for the cubemap
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = m_SRVFormat;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
		srvDesc.TextureCube.MostDetailedMip = 0;

		hr = device->CreateShaderResourceView(m_CubemapTexture, &srvDesc, &m_SRV);
		assert(hr == S_OK);

		// create srv for each face
		CreateFaceSRVs(device);

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

void Cubemap::CreateFaceSRVs(ID3D11Device* device)
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC texDesc;
	m_CubemapTexture->GetDesc(&texDesc);

	for (int i = 0; i < 6; i++)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = m_SRVFormat;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.ArraySize = 1;
		srvDesc.Texture2DArray.FirstArraySlice = i;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = texDesc.MipLevels;

		hr = device->CreateShaderResourceView(m_CubemapTexture, &srvDesc, &m_FaceSRVs[i]);
		assert(hr == S_OK);
	}
}
