#include "UnlitTerrainShader.h"


UnlitTerrainShader::UnlitTerrainShader(ID3D11Device* device)
	: m_Device(device)
{
	InitShader();
}


UnlitTerrainShader::~UnlitTerrainShader()
{
	if (m_VertexShader) m_VertexShader->Release();
	if (m_HullShader) m_HullShader->Release();
	if (m_DomainShader) m_DomainShader->Release();

	if (m_InputLayout) m_InputLayout->Release();

	if (m_VSMatrixBuffer) m_VSMatrixBuffer->Release();
	if (m_DSMatrixBuffer) m_DSMatrixBuffer->Release();
	if (m_TessellationBuffer) m_TessellationBuffer->Release();

	if (m_HeightmapSampleState) m_HeightmapSampleState->Release();
}

void UnlitTerrainShader::InitShader()
{
	// load and compile shaders
	LoadVS(L"terrain_vs.cso");
	LoadHS(L"terrain_hs.cso");
	LoadDS(L"unlitterrain_ds.cso");

	// create constant buffers
	CreateBuffer(sizeof(VSMatrixBufferType), &m_VSMatrixBuffer);
	CreateBuffer(sizeof(TessellationBufferType), &m_TessellationBuffer);
	CreateBuffer(sizeof(DSMatrixBufferType), &m_DSMatrixBuffer);

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
}

void UnlitTerrainShader::LoadVS(const wchar_t* vs)
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

void UnlitTerrainShader::LoadHS(const wchar_t* hs)
{
	ID3DBlob* hullShaderBuffer = nullptr;

	// Reads compiled shader into buffer (bytecode).
	HRESULT result = D3DReadFileToBlob(hs, &hullShaderBuffer);
	assert(result == S_OK && "Failed to load shader");

	// Create the shader from the buffer.
	m_Device->CreateHullShader(hullShaderBuffer->GetBufferPointer(), hullShaderBuffer->GetBufferSize(), NULL, &m_HullShader);
	hullShaderBuffer->Release();
}

void UnlitTerrainShader::LoadDS(const wchar_t* ds)
{
	ID3DBlob* domainShaderBuffer = nullptr;

	// Reads compiled shader into buffer (bytecode).
	HRESULT result = D3DReadFileToBlob(ds, &domainShaderBuffer);
	assert(result == S_OK && "Failed to load shader");

	// Create the shader from the buffer.
	m_Device->CreateDomainShader(domainShaderBuffer->GetBufferPointer(), domainShaderBuffer->GetBufferSize(), NULL, &m_DomainShader);
	domainShaderBuffer->Release();
}

void UnlitTerrainShader::CreateBuffer(UINT byteWidth, ID3D11Buffer** ppBuffer)
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


void UnlitTerrainShader::SetShaderParameters(ID3D11DeviceContext* deviceContext,
	const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
	ID3D11ShaderResourceView* heightmap, Camera* camera, XMFLOAT2 minMaxDist, XMFLOAT2 minMaxLOD)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;


	// update data in buffers
	{
		// Transpose the matrices to prepare them for the shader.
		result = deviceContext->Map(m_VSMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		VSMatrixBufferType* dataPtr = (VSMatrixBufferType*)mappedResource.pData;
		dataPtr->world = XMMatrixTranspose(world);
		deviceContext->Unmap(m_VSMatrixBuffer, 0);
	}
	{
		deviceContext->Map(m_TessellationBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		TessellationBufferType* dataPtr = (TessellationBufferType*)mappedResource.pData;
		dataPtr->minMaxDistance = minMaxDist;
		dataPtr->minMaxLOD = minMaxLOD;
		dataPtr->cameraPos = camera->getPosition();
		dataPtr->padding = 0.0f;
		deviceContext->Unmap(m_TessellationBuffer, 0);
	}
	{
		result = deviceContext->Map(m_DSMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		DSMatrixBufferType* dataPtr = (DSMatrixBufferType*)mappedResource.pData;
		dataPtr->view = XMMatrixTranspose(view);
		dataPtr->projection = XMMatrixTranspose(projection);
		deviceContext->Unmap(m_DSMatrixBuffer, 0);
	}

	deviceContext->VSSetConstantBuffers(0, 1, &m_VSMatrixBuffer);

	deviceContext->HSSetConstantBuffers(0, 1, &m_TessellationBuffer);

	deviceContext->DSSetConstantBuffers(0, 1, &m_DSMatrixBuffer);
	deviceContext->DSSetShaderResources(0, 1, &heightmap);
	deviceContext->DSSetSamplers(0, 1, &m_HeightmapSampleState);
}

void UnlitTerrainShader::Render(ID3D11DeviceContext* deviceContext, unsigned int indexCount)
{
	deviceContext->IASetInputLayout(m_InputLayout);

	deviceContext->VSSetShader(m_VertexShader, nullptr, 0);
	deviceContext->HSSetShader(m_HullShader, nullptr, 0);
	deviceContext->DSSetShader(m_DomainShader, nullptr, 0);
	deviceContext->GSSetShader(nullptr, nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);

	deviceContext->DrawIndexed(indexCount, 0, 0);

	deviceContext->VSSetShader(nullptr, nullptr, 0);
	deviceContext->HSSetShader(nullptr, nullptr, 0);
	deviceContext->DSSetShader(nullptr, nullptr, 0);
}
