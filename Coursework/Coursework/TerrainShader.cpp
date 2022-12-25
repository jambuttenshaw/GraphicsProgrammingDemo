#include "TerrainShader.h"

#include "SceneLight.h"
#include "Material.h"
#include "GlobalLighting.h"
#include "ShadowCubemap.h"
#include "TerrainMesh.h"


TerrainShader::TerrainShader(ID3D11Device* device, GlobalLighting* globalLighting)
	: m_Device(device), m_GlobalLighting(globalLighting)
{
	InitShader();
}


TerrainShader::~TerrainShader()
{
	if (m_VertexShader) m_VertexShader->Release();
	if (m_HullShader) m_HullShader->Release();
	if (m_DomainShader) m_DomainShader->Release();
	if (m_PixelShader) m_PixelShader->Release();

	if (m_InputLayout) m_InputLayout->Release();
	
	if (m_VSMatrixBuffer) m_VSMatrixBuffer->Release();
	if (m_DSMatrixBuffer) m_DSMatrixBuffer->Release();
	if (m_DSLightBuffer) m_DSLightBuffer->Release();
	if (m_PSLightBuffer) m_PSLightBuffer->Release();
	if (m_TerrainBuffer) m_TerrainBuffer->Release();
	if (m_MaterialBuffer) m_MaterialBuffer->Release();
	if (m_TessellationBuffer) m_TessellationBuffer->Release();

	if (m_HeightmapSampleState) m_HeightmapSampleState->Release();
	if (m_MaterialSampler) m_MaterialSampler->Release();
	if (m_ShadowSampler) m_ShadowSampler->Release();
	if (m_PointSampler) m_PointSampler->Release();
}

void TerrainShader::InitShader()
{
	// load and compile shaders
	LoadVS(L"terrain_vs.cso");
	LoadHS(L"terrain_hs.cso");
	LoadDS(L"terrain_ds.cso");
	LoadPS(L"terrain_ps.cso");

	// create constant buffers
	ShaderUtility::CreateBuffer(m_Device, sizeof(VSMatrixBufferType), &m_VSMatrixBuffer);
	ShaderUtility::CreateBuffer(m_Device, sizeof(TessellationBufferType), &m_TessellationBuffer);
	ShaderUtility::CreateBuffer(m_Device, sizeof(DSMatrixBufferType), &m_DSMatrixBuffer);
	ShaderUtility::CreateBuffer(m_Device, sizeof(ShaderUtility::VSLightBufferType), &m_DSLightBuffer);
	ShaderUtility::CreateBuffer(m_Device, sizeof(ShaderUtility::PSLightBufferType), &m_PSLightBuffer);
	ShaderUtility::CreateBuffer(m_Device, sizeof(ShaderUtility::MaterialBufferType), &m_MaterialBuffer);
	ShaderUtility::CreateBuffer(m_Device, sizeof(TerrainBufferType), &m_TerrainBuffer);

	// create sampler state
	D3D11_SAMPLER_DESC heightmapSamplerDesc;
	heightmapSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	heightmapSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	heightmapSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	heightmapSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	heightmapSamplerDesc.MipLODBias = 0.0f;
	heightmapSamplerDesc.MaxAnisotropy = 1;
	heightmapSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	heightmapSamplerDesc.MinLOD = 0;
	heightmapSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_Device->CreateSamplerState(&heightmapSamplerDesc, &m_HeightmapSampleState);

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_Device->CreateSamplerState(&samplerDesc, &m_MaterialSampler);

	D3D11_SAMPLER_DESC shadowSamplerDesc;
	shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.MinLOD = 0;
	shadowSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	shadowSamplerDesc.MipLODBias = 0;
	shadowSamplerDesc.MaxAnisotropy = 1;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[1] = 1.0f;
	shadowSamplerDesc.BorderColor[2] = 1.0f;
	shadowSamplerDesc.BorderColor[3] = 1.0f;
	shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	m_Device->CreateSamplerState(&shadowSamplerDesc, &m_ShadowSampler);

	D3D11_SAMPLER_DESC pointSamplerDesc;
	pointSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	pointSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	pointSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	pointSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	pointSamplerDesc.MipLODBias = 0.0f;
	pointSamplerDesc.MaxAnisotropy = 1;
	pointSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	pointSamplerDesc.MinLOD = 0;
	pointSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_Device->CreateSamplerState(&pointSamplerDesc, &m_PointSampler);
}

void TerrainShader::LoadVS(const wchar_t* vs)
{
	ID3DBlob* vertexShaderBuffer = nullptr;

	// Reads compiled shader into buffer (bytecode).
	HRESULT result = D3DReadFileToBlob(vs, &vertexShaderBuffer);
	assert(result == S_OK && "Failed to load shader");

	// Create the vertex shader from the buffer.
	m_Device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_VertexShader);

	// Create the vertex input layout description.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	// Get a count of the elements in the layout.
	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// Create the vertex input layout.
	m_Device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_InputLayout);

	vertexShaderBuffer->Release();
}

void TerrainShader::LoadHS(const wchar_t* hs)
{
	ID3DBlob* hullShaderBuffer = nullptr;

	// Reads compiled shader into buffer (bytecode).
	HRESULT result = D3DReadFileToBlob(hs, &hullShaderBuffer);
	assert(result == S_OK && "Failed to load shader");

	// Create the shader from the buffer.
	m_Device->CreateHullShader(hullShaderBuffer->GetBufferPointer(), hullShaderBuffer->GetBufferSize(), NULL, &m_HullShader);
	hullShaderBuffer->Release();
}

void TerrainShader::LoadDS(const wchar_t* ds)
{
	ID3DBlob* domainShaderBuffer = nullptr;

	// Reads compiled shader into buffer (bytecode).
	HRESULT result = D3DReadFileToBlob(ds, &domainShaderBuffer);
	assert(result == S_OK && "Failed to load shader");

	// Create the shader from the buffer.
	m_Device->CreateDomainShader(domainShaderBuffer->GetBufferPointer(), domainShaderBuffer->GetBufferSize(), NULL, &m_DomainShader);
	domainShaderBuffer->Release();
}

void TerrainShader::LoadPS(const wchar_t* ps)
{
	ID3DBlob* pixelShaderBuffer = nullptr;

	// Reads compiled shader into buffer (bytecode).
	HRESULT result = D3DReadFileToBlob(ps, &pixelShaderBuffer);
	assert(result == S_OK && "Failed to load shader");

	// Create the shader from the buffer.
	m_Device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_PixelShader);
	pixelShaderBuffer->Release();
}


void TerrainShader::SetShaderParameters(ID3D11DeviceContext* deviceContext,
	const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix,
	TerrainMesh* terrainMesh,
	size_t lightCount, SceneLight** lights, Camera* camera, Material* mat)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	
	ResourceBuffer tex2DBuffer, texCubeBuffer;

	// update data in buffers
	{
		// Transpose the matrices to prepare them for the shader.
		result = deviceContext->Map(m_VSMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		VSMatrixBufferType* dataPtr = (VSMatrixBufferType*)mappedResource.pData;
		dataPtr->world = XMMatrixTranspose(worldMatrix);
		deviceContext->Unmap(m_VSMatrixBuffer, 0);
	}
	{
		deviceContext->Map(m_TessellationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		TessellationBufferType* dataPtr = (TessellationBufferType*)mappedResource.pData;
		dataPtr->minMaxDistance = m_MinMaxDistance;
		dataPtr->minMaxHeightDeviation = m_MinMaxHeightDeviation;
		dataPtr->minMaxLOD = m_MinMaxLOD;
		dataPtr->distanceLODBlending = m_DistanceLODBlending;
		dataPtr->padding = 0.0f;
		dataPtr->cameraPos = camera->getPosition();
		dataPtr->size = terrainMesh->GetSize();
		deviceContext->Unmap(m_TessellationBuffer, 0);
	}
	{
		result = deviceContext->Map(m_DSMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		DSMatrixBufferType* dataPtr = (DSMatrixBufferType*)mappedResource.pData;
		dataPtr->view = XMMatrixTranspose(viewMatrix);
		dataPtr->projection = XMMatrixTranspose(projectionMatrix);
		deviceContext->Unmap(m_DSMatrixBuffer, 0);
	}
	{
		result = deviceContext->Map(m_DSLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		ShaderUtility::VSLightBufferType* dataPtr = (ShaderUtility::VSLightBufferType*)mappedResource.pData;
		ShaderUtility::ConstructVSLightBuffer(dataPtr, lights, lightCount, camera);
		deviceContext->Unmap(m_DSLightBuffer, 0);
	}
	{
		deviceContext->Map(m_PSLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		ShaderUtility::PSLightBufferType* dataPtr = (ShaderUtility::PSLightBufferType*)mappedResource.pData;
		ShaderUtility::ConstructPSLightBuffer(dataPtr, lights, lightCount, m_GlobalLighting, &tex2DBuffer, &texCubeBuffer);
		deviceContext->Unmap(m_PSLightBuffer, 0);
	}
	{
		deviceContext->Map(m_MaterialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		ShaderUtility::MaterialBufferType* dataPtr = (ShaderUtility::MaterialBufferType*)mappedResource.pData;
		ShaderUtility::ConstructMaterialData(&dataPtr->material, mat, &tex2DBuffer);
		deviceContext->Unmap(m_MaterialBuffer, 0);
	}
	{
		deviceContext->Map(m_TerrainBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		TerrainBufferType* dataPtr = (TerrainBufferType*)mappedResource.pData;
		
		dataPtr->heightmapIndex = tex2DBuffer.AddResource(terrainMesh->GetHeightmapSRV());

		dataPtr->flatThreshold = m_FlatThreshold;
		dataPtr->cliffThreshold = m_CliffThreshold;
		dataPtr->steepnessSmoothing = m_SteepnessSmoothing;
		deviceContext->Unmap(m_TerrainBuffer, 0);
	}

	deviceContext->VSSetConstantBuffers(0, 1, &m_VSMatrixBuffer);

	deviceContext->HSSetConstantBuffers(0, 1, &m_TessellationBuffer);
	auto preprocessedHeightmap = terrainMesh->GetPreprocessSRV();
	deviceContext->HSSetShaderResources(0, 1, &preprocessedHeightmap);
	deviceContext->HSSetSamplers(0, 1, &m_PointSampler);

	ID3D11Buffer* dsCBs[] = { m_DSMatrixBuffer, m_DSLightBuffer };
	deviceContext->DSSetConstantBuffers(0, 2, dsCBs);
	auto heightmap = terrainMesh->GetHeightmapSRV();
	deviceContext->DSSetShaderResources(0, 1, &heightmap);
	deviceContext->DSSetSamplers(0, 1, &m_HeightmapSampleState);

	ID3D11Buffer* psCBs[] = { m_PSLightBuffer, m_MaterialBuffer, m_TerrainBuffer };
	deviceContext->PSSetConstantBuffers(0, 3, psCBs);

	deviceContext->PSSetShaderResources(0, RESOURCE_BUFFER_SIZE, tex2DBuffer.GetResourcePtr());
	deviceContext->PSSetShaderResources(RESOURCE_BUFFER_SIZE, RESOURCE_BUFFER_SIZE, texCubeBuffer.GetResourcePtr());

	ID3D11SamplerState* psSamplers[] = { m_GlobalLighting->GetBRDFIntegrationSampler(), m_GlobalLighting->GetCubemapSampler(), m_MaterialSampler, m_ShadowSampler, m_HeightmapSampleState };
	deviceContext->PSSetSamplers(0, 5, psSamplers);
}

void TerrainShader::Render(ID3D11DeviceContext* deviceContext, unsigned int indexCount)
{
	deviceContext->IASetInputLayout(m_InputLayout);

	deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
	deviceContext->HSSetShader(m_HullShader, nullptr, 0);
	deviceContext->DSSetShader(m_DomainShader, nullptr, 0);
	deviceContext->GSSetShader(nullptr, nullptr, 0);
	deviceContext->PSSetShader(m_PixelShader, nullptr, 0);

	deviceContext->DrawIndexed(indexCount, 0, 0);

	deviceContext->VSSetShader(nullptr, nullptr, 0);
	deviceContext->HSSetShader(nullptr, nullptr, 0);
	deviceContext->DSSetShader(nullptr, nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);
}

void TerrainShader::GUI()
{
	ImGui::SliderFloat("Flat Threshold", &m_FlatThreshold, 0.0f, m_CliffThreshold);
	ImGui::SliderFloat("Cliff Threshold", &m_CliffThreshold, m_FlatThreshold, 1.0f);
	ImGui::SliderFloat("Steepness Smoothing", &m_SteepnessSmoothing, 0.0f, 0.2f);

	ImGui::Text("LOD");
	ImGui::SliderFloat("Min Distance", &m_MinMaxDistance.x, 0.0f, m_MinMaxDistance.y);
	ImGui::SliderFloat("Max Distance", &m_MinMaxDistance.y, m_MinMaxDistance.x, 100.0f);
	ImGui::SliderFloat("Min Height Deviation", &m_MinMaxHeightDeviation.x, 0.0f, m_MinMaxHeightDeviation.y);
	ImGui::SliderFloat("Max Height Deviation", &m_MinMaxHeightDeviation.y, m_MinMaxHeightDeviation.x, 4.0f);
	ImGui::SliderFloat("Min LOD", &m_MinMaxLOD.x, 1.0f, m_MinMaxLOD.y);
	ImGui::SliderFloat("Max LOD", &m_MinMaxLOD.y, m_MinMaxLOD.x, 64.0f);
}

nlohmann::json TerrainShader::Serialize() const
{
	nlohmann::json serialized;

	serialized["flatThreshold"] = m_FlatThreshold;
	serialized["cliffThreshold"] = m_CliffThreshold;
	serialized["steepnessSmoothing"] = m_SteepnessSmoothing;

	return serialized;
}

void TerrainShader::LoadFromJson(const nlohmann::json& data)
{
	if (data.contains("flatThreshold")) m_FlatThreshold = data["flatThreshold"];
	if (data.contains("cliffThreshold")) m_CliffThreshold = data["cliffThreshold"];
	if (data.contains("steepnessSmoothing")) m_SteepnessSmoothing = data["steepnessSmoothing"];
}
