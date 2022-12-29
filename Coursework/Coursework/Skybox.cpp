#include "Skybox.h"

#include "Cubemap.h"


Skybox::Skybox(ID3D11Device* device, Cubemap* cubemap)
	: m_Device(device), m_Cubemap(cubemap)
{
	m_CubeMesh = new CubeMesh(device, nullptr, 1);

	// load shaders
	LoadVS();
	LoadPS();

	// create vs cbuffer
	D3D11_BUFFER_DESC vsBufferDesc;
	vsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vsBufferDesc.ByteWidth = sizeof(VSBufferType);
	vsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	vsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vsBufferDesc.MiscFlags = 0;
	vsBufferDesc.StructureByteStride = 0;
	m_Device->CreateBuffer(&vsBufferDesc, NULL, &m_VSBuffer);

	// setup cubemap sampler state
	D3D11_SAMPLER_DESC cubemapSamplerDesc;
	cubemapSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	cubemapSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	cubemapSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	cubemapSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	cubemapSamplerDesc.MipLODBias = 0.0f;
	cubemapSamplerDesc.MaxAnisotropy = 1;
	cubemapSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	cubemapSamplerDesc.MinLOD = 0;
	cubemapSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_Device->CreateSamplerState(&cubemapSamplerDesc, &m_SamplerState);

	// create raster state
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.AntialiasedLineEnable = false;
	device->CreateRasterizerState(&rasterDesc, &m_RasterState);

	// create depth state.
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&depthStencilDesc, &m_DepthState);
}

Skybox::~Skybox()
{
	delete m_CubeMesh;

	m_VS->Release();
	m_PS->Release();
	m_VSBuffer->Release();
	m_InputLayout->Release();
	m_RasterState->Release();
	m_DepthState->Release();
}

void Skybox::Render(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection)
{
	// send geometry data
	m_CubeMesh->sendData(deviceContext);

	// setup shader resources
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(world);
	XMMATRIX tview = XMMatrixTranspose(view);
	XMMATRIX tproj = XMMatrixTranspose(projection);

	// update data in buffers
	result = deviceContext->Map(m_VSBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	VSBufferType* dataPtr = (VSBufferType*)mappedResource.pData;
	dataPtr->world = tworld;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(m_VSBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &m_VSBuffer);

	ID3D11ShaderResourceView* cubemapSRV = m_Cubemap->GetSRV();
	deviceContext->PSSetShaderResources(0, 1, &cubemapSRV);
	deviceContext->PSSetSamplers(0, 1, &m_SamplerState);

	// render
	deviceContext->IASetInputLayout(m_InputLayout);
	
	deviceContext->RSSetState(m_RasterState);
	deviceContext->OMSetDepthStencilState(m_DepthState, 1);

	deviceContext->VSSetShader(m_VS, nullptr, 0);
	deviceContext->PSSetShader(m_PS, nullptr, 0);

	deviceContext->DrawIndexed(m_CubeMesh->getIndexCount(), 0, 0);

	deviceContext->VSSetShader(nullptr, nullptr, 0);
	deviceContext->PSSetShader(nullptr, nullptr, 0);
}

void Skybox::LoadVS()
{
	ID3DBlob* vertexShaderBuffer = nullptr;

	// Reads compiled shader into buffer (bytecode).
	HRESULT result = D3DReadFileToBlob(L"skybox_vs.cso", &vertexShaderBuffer);
	assert(result == S_OK && "Failed to load shader");

	// Create the vertex shader from the buffer.
	m_Device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_VS);

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

void Skybox::LoadPS()
{
	ID3DBlob* pixelShaderBuffer = nullptr;

	// Reads compiled shader into buffer (bytecode).
	HRESULT result = D3DReadFileToBlob(L"skybox_ps.cso", &pixelShaderBuffer);
	assert(result == S_OK && "Failed to load shader");

	// Create the shader from the buffer.
	m_Device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_PS);
	pixelShaderBuffer->Release();
}
