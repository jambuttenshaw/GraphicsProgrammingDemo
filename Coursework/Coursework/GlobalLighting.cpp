#include "GlobalLighting.h"

#include <d3dcompiler.h>

#include <cmath>

#include "imGUI/imgui.h"


GlobalLighting::GlobalLighting(ID3D11Device* device)
	: m_Device(device)
{
	LoadShader(L"irradiance_cs.cso", &m_IrradianceMapShader);
	CreateBuffer(sizeof(IrradianceMapBufferType), &m_IrradianceMapShaderBuffer);

	LoadShader(L"prefilteredenvironment_cs.cso", &m_PEMShader);
	CreateBuffer(sizeof(PEMBufferType), &m_PEMShaderBuffer);

	LoadShader(L"brdfintegration_cs.cso", &m_BRDFIntegrationShader);

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_Device->CreateSamplerState(&samplerDesc, &m_CubemapSampler);

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	m_Device->CreateSamplerState(&samplerDesc, &m_BRDFIntegrationSampler);
}

GlobalLighting::~GlobalLighting()
{
	m_IrradianceMapShader->Release();
	m_IrradianceMapShaderBuffer->Release();
	m_PEMShader->Release();
	m_PEMShaderBuffer->Release();
	m_BRDFIntegrationShader->Release();

	if (m_IrradianceMap) delete m_IrradianceMap;
	m_CubemapSampler->Release();
	m_BRDFIntegrationSampler->Release();

	if (m_BRDFIntegrationMap) m_BRDFIntegrationMap->Release();
	if (m_BRDFIntegrationMapSRV) m_BRDFIntegrationMapSRV->Release();
	if (m_BRDFIntegrationMapUAV) m_BRDFIntegrationMapUAV->Release();

	if (m_PrefilteredEnvironmentMap) delete m_PrefilteredEnvironmentMap;
}

void GlobalLighting::SettingsGUI()
{
	ImGui::Checkbox("Enable IBL", &m_EnableIBL);
}

void GlobalLighting::SetAndProcessEnvironmentMap(ID3D11DeviceContext* deviceContext, Cubemap* environment)
{
	m_EnvironmentMap = environment;

	CreateIrradianceMap(deviceContext);
	CreateBRDFIntegrationMap(deviceContext);
	CreatePrefilteredEnvironmentMap(deviceContext);
}

void GlobalLighting::LoadShader(const wchar_t* file, ID3D11ComputeShader** shader)
{
	// load compute shader from file
	ID3D10Blob* computeShaderBuffer;

	// Reads compiled shader into buffer (bytecode).
	HRESULT result = D3DReadFileToBlob(file, &computeShaderBuffer);
	assert(result == S_OK && "Failed to load shader");

	// Create the compute shader from the buffer.
	m_Device->CreateComputeShader(computeShaderBuffer->GetBufferPointer(), computeShaderBuffer->GetBufferSize(), NULL, shader);

	computeShaderBuffer->Release();
}

void GlobalLighting::CreateBuffer(UINT byteWidth, ID3D11Buffer** ppBuffer)
{
	assert(byteWidth % 16 == 0 && "Constant buffer byte width must be multiple of 16!");

	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	m_Device->CreateBuffer(&desc, NULL, ppBuffer);
}

void GlobalLighting::CreateIrradianceMap(ID3D11DeviceContext* deviceContext)
{
	// create irradiance map
	if (m_IrradianceMap) delete m_IrradianceMap;
	m_IrradianceMap = new Cubemap(m_Device, m_IrradianceMapResolutuion, false);

	ID3D11ShaderResourceView* environmentMapSRV = m_EnvironmentMap->GetSRV();
	deviceContext->CSSetShaderResources(0, 1, &environmentMapSRV);
	deviceContext->CSSetSamplers(0, 1, &m_CubemapSampler);
	deviceContext->CSSetConstantBuffers(0, 1, &m_IrradianceMapShaderBuffer);
	deviceContext->CSSetShader(m_IrradianceMapShader, nullptr, 0);

	// assume thread groups consist of 8x8x1 threads
	unsigned int groupCount = (m_IrradianceMapResolutuion + 7) / 8; // (fast ceiling of integer division)

	for (int face = 0; face < 6; face++)
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		deviceContext->Map(m_IrradianceMapShaderBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		IrradianceMapBufferType* data = (IrradianceMapBufferType*)mappedResource.pData;
		data->faceNormal = Cubemap::GetFaceNormal(face);
		data->faceTangent = Cubemap::GetFaceTangent(face);
		data->faceBitangent = Cubemap::GetFaceBitangent(face);
		deviceContext->Unmap(m_IrradianceMapShaderBuffer, 0);

		ID3D11UnorderedAccessView* uav = m_IrradianceMap->GetUAV(face);
		deviceContext->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

		deviceContext->Dispatch(groupCount, groupCount, 1);
	}
	// unbind resources
	deviceContext->CSSetShader(nullptr, nullptr, 0);

	ID3D11UnorderedAccessView* nullUAV = nullptr;
	deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	ID3D11ShaderResourceView* nullSRV = nullptr;
	deviceContext->CSSetShaderResources(0, 1, &nullSRV);
	ID3D11SamplerState* nullSampler = nullptr;
	deviceContext->CSSetSamplers(0, 1, &nullSampler);
	ID3D11Buffer* nullBuffer = nullptr;
	deviceContext->CSSetConstantBuffers(0, 1, &nullBuffer);
}

void GlobalLighting::CreateBRDFIntegrationMap(ID3D11DeviceContext* deviceContext)
{
	if (m_BRDFIntegrationMap) m_BRDFIntegrationMap->Release();
	if (m_BRDFIntegrationMapSRV) m_BRDFIntegrationMapSRV->Release();
	if (m_BRDFIntegrationMapUAV) m_BRDFIntegrationMapUAV->Release();

	// create texture
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width =  m_BRDFIntegrationMapResolution;
	texDesc.Height = m_BRDFIntegrationMapResolution;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	
	HRESULT hr = m_Device->CreateTexture2D(&texDesc, nullptr, &m_BRDFIntegrationMap);
	assert(hr == S_OK);

	// create srv
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	
	hr = m_Device->CreateShaderResourceView(m_BRDFIntegrationMap, &srvDesc, &m_BRDFIntegrationMapSRV);
	assert(hr == S_OK);

	// create uav
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	hr = m_Device->CreateUnorderedAccessView(m_BRDFIntegrationMap, &uavDesc, &m_BRDFIntegrationMapUAV);
	assert(hr == S_OK);

	// set cs resources
	deviceContext->CSSetShader(m_BRDFIntegrationShader, nullptr, 0);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &m_BRDFIntegrationMapUAV, nullptr);

	unsigned int groupCount = (m_BRDFIntegrationMapResolution + 7) / 8; // (fast ceiling of integer division)

	// run cs
	deviceContext->Dispatch(groupCount, groupCount, 1);

	deviceContext->CSSetShader(nullptr, nullptr, 0);
	ID3D11UnorderedAccessView* nullUAV = nullptr;
	deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
}

void GlobalLighting::CreatePrefilteredEnvironmentMap(ID3D11DeviceContext* deviceContext)
{
	// create prefiltered environment map
	if (m_PrefilteredEnvironmentMap) delete m_PrefilteredEnvironmentMap;
	m_PrefilteredEnvironmentMap = new Cubemap(m_Device, m_PEMResolution, false, m_PEMRoughnessBins);

	// set shader and shader resources that are constant for all faces and mips
	ID3D11ShaderResourceView* environmentMapSRV = m_EnvironmentMap->GetSRV();
	deviceContext->CSSetShaderResources(0, 1, &environmentMapSRV);
	deviceContext->CSSetSamplers(0, 1, &m_CubemapSampler);
	deviceContext->CSSetConstantBuffers(0, 1, &m_PEMShaderBuffer);
	deviceContext->CSSetShader(m_PEMShader, nullptr, 0);

	// different levels of roughness of reflections are stored in the maps mip chain, so we need to generate mips for this cubemap
	for (unsigned int mip = 0; mip < m_PEMRoughnessBins; mip++)
	{
		// calculate width and height of this mip
		unsigned int mipSize = static_cast<unsigned int>(static_cast<double>(m_PEMResolution) * std::pow(0.5, mip));

		// assume thread groups consist of 8x8x1 threads
		unsigned int groupCount = (mipSize + 7) / 8; // (fast ceiling of integer division)

		float mipRoughness = (float)mip / (float)(m_PEMRoughnessBins - 1);

		for (int face = 0; face < 6; face++)
		{
			// update data in buffer
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			deviceContext->Map(m_PEMShaderBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			PEMBufferType* data = (PEMBufferType*)mappedResource.pData;
			data->faceNormal = Cubemap::GetFaceNormal(face);
			data->faceTangent = Cubemap::GetFaceTangent(face);
			data->faceBitangent = Cubemap::GetFaceBitangent(face);
			data->roughness = mipRoughness;
			deviceContext->Unmap(m_PEMShaderBuffer, 0);

			// get uav for this mip and face
			ID3D11UnorderedAccessView* uav = m_PrefilteredEnvironmentMap->GetUAV(face, mip);
			deviceContext->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

			deviceContext->Dispatch(groupCount, groupCount, 1);
		}
	}
	// unbind resources
	deviceContext->CSSetShader(nullptr, nullptr, 0);

	ID3D11UnorderedAccessView* nullUAV = nullptr;
	deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
	ID3D11ShaderResourceView* nullSRV = nullptr;
	deviceContext->CSSetShaderResources(0, 1, &nullSRV);
	ID3D11SamplerState* nullSampler = nullptr;
	deviceContext->CSSetSamplers(0, 1, &nullSampler);
	ID3D11Buffer* nullBuffer = nullptr;
	deviceContext->CSSetConstantBuffers(0, 1, &nullBuffer);
}
