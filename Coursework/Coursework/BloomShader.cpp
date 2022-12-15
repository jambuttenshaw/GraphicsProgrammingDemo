#include "BloomShader.h"

#include <d3dcompiler.h>

#include "imGUI/imgui.h"


BloomShader::BloomShader(ID3D11Device* device, unsigned int width, unsigned int height, unsigned int levels)
	: m_Width(width), m_Height(height), m_Levels(levels)
{
	assert(levels > 1 && "Must have at least 2 levels!");

	// load shaders
	LoadCS(device, L"bloom_cs.cso", &m_Shader);

	HRESULT hr;

	// create textures and views
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = levels;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	hr = device->CreateTexture2D(&texDesc, nullptr, &m_BloomTex);
	assert(hr == S_OK);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = levels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(m_BloomTex, &srvDesc, &m_SRV);
	assert(hr == S_OK);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	for (unsigned int i = 0; i < levels - 1; i++)
	{
		uavDesc.Texture2D.MipSlice = i;

		ID3D11UnorderedAccessView* uav;
		hr = device->CreateUnorderedAccessView(m_BloomTex, &uavDesc, &uav);
		assert(hr == S_OK);
		m_UAVs.push_back(uav);
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
	m_Shader->Release();

	m_BloomTex->Release();

	m_SRV->Release();
	for (auto uav : m_UAVs) uav->Release();

	m_CSBuffer->Release();
	m_TrilinearSampler->Release();
}

void BloomShader::SettingsGUI()
{
	ImGui::DragFloat("Strength", &m_Strength, 0.01f);
	ImGui::DragFloat("Threshold", &m_Threshold, 0.005f);
	ImGui::DragFloat("Smoothing", &m_Smoothing, 0.001f);
}

void BloomShader::Run(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* input)
{
	// run initial reduction shader on input
	deviceContext->CSSetShader(m_Shader, nullptr, 0);

	deviceContext->CSSetShaderResources(0, 1, &input);
	ID3D11UnorderedAccessView* uav = GetUAV(0);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);
	deviceContext->CSSetSamplers(0, 1, &m_TrilinearSampler);

	// update parameters CB
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(m_CSBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	CSBufferType* dataPtr = (CSBufferType*)mappedResource.pData;
	dataPtr->threshold = m_Threshold;
	dataPtr->smoothing = m_Smoothing;
	deviceContext->Unmap(m_CSBuffer, 0);
	deviceContext->CSSetConstantBuffers(0, 1, &m_CSBuffer);

	// calculate groups
	unsigned int groupsX = static_cast<unsigned int>(ceilf(m_Width / 8.0f));
	unsigned int groupsY = static_cast<unsigned int>(ceilf(m_Height / 8.0f));

	// dispatch CS
	deviceContext->Dispatch(groupsX, groupsY, 1);

	// unbind resources
	ID3D11Buffer* nullCB = nullptr;
	ID3D11ShaderResourceView* nullSRV = nullptr;
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	deviceContext->CSSetConstantBuffers(0, 1, &nullCB);
	deviceContext->CSSetShaderResources(0, 1, &nullSRV);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

	// generate mip chain to downsample and blur
	deviceContext->GenerateMips(m_SRV);
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
