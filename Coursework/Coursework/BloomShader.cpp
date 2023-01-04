#include "BloomShader.h"

#include <d3dcompiler.h>

#include "imGUI/imgui.h"


BloomShader::BloomShader(ID3D11Device* device, unsigned int width, unsigned int height, unsigned int levels)
	: m_Width(width), m_Height(height), m_LevelCount(levels)
{
	assert(levels > 1 && "Must have at least 2 levels!");

	// load shaders
	LoadCS(device, L"bloombrightpass_cs.cso", &m_BrightpassShader);
	LoadCS(device, L"bloomcombine_cs.cso", &m_CombineShader);
	LoadCS(device, L"horizontalguass_cs.cso", &m_HorizontalBlurShader);
	LoadCS(device, L"verticalguass_cs.cso", &m_VerticalBlurShader);

	HRESULT hr;

	// create textures and views
	for (int level = 0; level < m_LevelCount; level++)
	{
		BloomLevel bloomLevel;

		ID3D11Texture2D* tex1 = nullptr;
		ID3D11Texture2D* tex2 = nullptr;

		unsigned int currentWidth  = static_cast<unsigned int>( m_Width * powf(0.5f, static_cast<float>(level)));
		unsigned int currentHeight = static_cast<unsigned int>(m_Height * powf(0.5f, static_cast<float>(level)));

		D3D11_TEXTURE2D_DESC texDesc;
		texDesc.Width = currentWidth;
		texDesc.Height = currentHeight;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		hr = device->CreateTexture2D(&texDesc, nullptr, &tex1);
		assert(hr == S_OK);
		hr = device->CreateTexture2D(&texDesc, nullptr, &tex2);
		assert(hr == S_OK);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		hr = device->CreateShaderResourceView(tex1, &srvDesc, &bloomLevel.srv1);
		assert(hr == S_OK);
		hr = device->CreateShaderResourceView(tex2, &srvDesc, &bloomLevel.srv2);
		assert(hr == S_OK);

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		hr = device->CreateUnorderedAccessView(tex1, &uavDesc, &bloomLevel.uav1);
		assert(hr == S_OK);
		hr = device->CreateUnorderedAccessView(tex2, &uavDesc, &bloomLevel.uav2);
		assert(hr == S_OK);

		m_Levels.push_back(bloomLevel);

		if (currentWidth <= 1 || currentHeight <= 1)
		{
			m_LevelCount = level;
			break;
		}
	}

	// create buffers
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(CSBufferType);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	hr = device->CreateBuffer(&cbDesc, nullptr, &m_CSBuffer);
	assert(hr == S_OK);

	// create sampler
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = device->CreateSamplerState(&samplerDesc, &m_TrilinearSampler);
	assert(hr == S_OK);
}

BloomShader::~BloomShader()
{
	m_BrightpassShader->Release();
	m_CombineShader->Release();
	m_HorizontalBlurShader->Release();
	m_VerticalBlurShader->Release();

	m_CSBuffer->Release();
	m_TrilinearSampler->Release();

	for (auto& level : m_Levels)
		level.Release();
}

void BloomShader::SettingsGUI()
{
	ImGui::DragFloat("Threshold", &m_Threshold, 0.005f);
}

void BloomShader::Run(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* input)
{
	// run brightpass filter on input
	CSBufferType params{ {m_Threshold, 0.0f, 0.0f, 0.0f} };
	RunCS(deviceContext, m_BrightpassShader, input, m_Levels[0].uav1, 0, &params, sizeof(params));


	// blur and downscale
	for (int level = 1; level < m_LevelCount; level++)
	{
		// horizontal blur
		RunCS(deviceContext, m_HorizontalBlurShader, m_Levels[level - 1].srv1, m_Levels[level].uav2, level, &params, sizeof(params));
		// vertical blur
		RunCS(deviceContext, m_VerticalBlurShader, m_Levels[level].srv2, m_Levels[level].uav1, level, &params, sizeof(params));
	}

	// combine each level with the level below it
	for (int level = m_LevelCount - 1; level > 0; level--)
	{
		RunCS(deviceContext, m_CombineShader, m_Levels[level].srv1, m_Levels[level - 1].uav1, level - 1, &params, sizeof(params));
	}
}


void BloomShader::LoadCS(ID3D11Device* device, const wchar_t* filename, ID3D11ComputeShader** ppCS)
{
	// load compute shader from file
	ID3D10Blob* computeShaderBuffer;

	// Reads compiled shader into buffer (bytecode).
	HRESULT hr = D3DReadFileToBlob(filename, &computeShaderBuffer);
	assert(hr == S_OK && "Failed to load shader");

	// Create the compute shader from the buffer.
	hr = device->CreateComputeShader(computeShaderBuffer->GetBufferPointer(), computeShaderBuffer->GetBufferSize(), NULL, ppCS);
	assert(hr == S_OK);

	computeShaderBuffer->Release();
}

void BloomShader::RunCS(ID3D11DeviceContext* deviceContext, ID3D11ComputeShader* shader, ID3D11ShaderResourceView* input, ID3D11UnorderedAccessView* output, int outputLevel, void* csBufferData, size_t csBufferDataSize)
{
	// run initial reduction shader on input
	deviceContext->CSSetShader(shader, nullptr, 0);

	deviceContext->CSSetShaderResources(0, 1, &input);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &output, nullptr);
	deviceContext->CSSetSamplers(0, 1, &m_TrilinearSampler);

	// update parameters CB
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(m_CSBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, csBufferData, csBufferDataSize);
	deviceContext->Unmap(m_CSBuffer, 0);
	deviceContext->CSSetConstantBuffers(0, 1, &m_CSBuffer);

	// calculate groups
	float levelWidth = m_Width * powf(0.5f, static_cast<float>(outputLevel));
	float levelHeight = m_Height * powf(0.5f, static_cast<float>(outputLevel));
	unsigned int groupsX = static_cast<unsigned int>(ceilf(levelWidth / 8.0f));
	unsigned int groupsY = static_cast<unsigned int>(ceilf(levelHeight / 8.0f));

	// dispatch CS
	deviceContext->Dispatch(groupsX, groupsY, 1);

	// unbind resources
	ID3D11Buffer* nullCB = nullptr;
	ID3D11ShaderResourceView* nullSRV = nullptr;
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	deviceContext->CSSetConstantBuffers(0, 1, &nullCB);
	deviceContext->CSSetShaderResources(0, 1, &nullSRV);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
}
