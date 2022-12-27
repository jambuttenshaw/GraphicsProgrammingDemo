#include "LightShader.h"

#include "SceneLight.h"
#include "Material.h"
#include "GlobalLighting.h"
#include "ShadowCubemap.h"

#include "imGUI/imgui.h"


LightShader::LightShader(ID3D11Device* device, HWND hwnd, GlobalLighting* globalLighting) : BaseShader(device, hwnd), m_GlobalLighting(globalLighting)
{
	initShader(L"lighting_vs.cso", L"lighting_ps.cso");
}


LightShader::~LightShader()
{
	m_MatrixBuffer->Release();
	m_VSLightBuffer->Release();
	m_PSLightBuffer->Release();
	m_MaterialBuffer->Release();

	m_MaterialSampler->Release();
	m_ShadowSampler->Release();
}

void LightShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	ShaderUtility::CreateBuffer(renderer, sizeof(MatrixBufferType), &m_MatrixBuffer);
	ShaderUtility::CreateBuffer(renderer, sizeof(ShaderUtility::VSLightBufferType), &m_VSLightBuffer);
	ShaderUtility::CreateBuffer(renderer, sizeof(ShaderUtility::PSLightBufferType), &m_PSLightBuffer);
	ShaderUtility::CreateBuffer(renderer, sizeof(ShaderUtility::MaterialBufferType), &m_MaterialBuffer);

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
	HRESULT hr = renderer->CreateSamplerState(&samplerDesc, &m_MaterialSampler);
	assert(hr == S_OK);

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
	hr = renderer->CreateSamplerState(&shadowSamplerDesc, &m_ShadowSampler);
	assert(hr == S_OK);
}


void LightShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
									size_t lightCount, SceneLight** lights, Camera* camera, Material* mat)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	ResourceBuffer tex2DBuffer, texCubeBuffer;

	{
		result = deviceContext->Map(m_MatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;
		dataPtr->world = XMMatrixTranspose(worldMatrix);
		dataPtr->view = XMMatrixTranspose(viewMatrix);
		dataPtr->projection = XMMatrixTranspose(projectionMatrix);
		deviceContext->Unmap(m_MatrixBuffer, 0);
	}
	
	ShaderUtility::ConstructVSLightBuffer(deviceContext, m_VSLightBuffer, lights, lightCount, camera);
	ShaderUtility::ConstructPSLightBuffer(deviceContext, m_PSLightBuffer, lights, lightCount, m_GlobalLighting, &tex2DBuffer, &texCubeBuffer);
	ShaderUtility::ConstructMaterialBuffer(deviceContext, m_MaterialBuffer, &mat, 1, &tex2DBuffer);


	ID3D11Buffer* vsBuffers[] = { m_MatrixBuffer, m_VSLightBuffer };
	deviceContext->VSSetConstantBuffers(0, 2, vsBuffers);

	ID3D11Buffer* psBuffers[] = { m_PSLightBuffer, m_MaterialBuffer };
	deviceContext->PSSetConstantBuffers(0, 2, psBuffers);
	
	deviceContext->PSSetShaderResources(0, RESOURCE_BUFFER_SIZE, tex2DBuffer.GetResourcePtr());
	deviceContext->PSSetShaderResources(RESOURCE_BUFFER_SIZE, RESOURCE_BUFFER_SIZE, texCubeBuffer.GetResourcePtr());

	ID3D11SamplerState* samplers[] = { m_GlobalLighting->GetBRDFIntegrationSampler(), m_GlobalLighting->GetCubemapSampler(), m_MaterialSampler, m_ShadowSampler };
	deviceContext->PSSetSamplers(0, 4, samplers);
}
