#include "FinalPassShader.h"

#include "imGUI/imgui.h"


FinalPassShader::FinalPassShader(ID3D11Device* device)
	: BaseFullScreenShader(device)
{
	Init(L"finalpass_ps.cso");


}

FinalPassShader::~FinalPassShader()
{
	m_ParamsBuffer->Release();
}

void FinalPassShader::setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* renderTextureColour, ID3D11ShaderResourceView* renderTextureDepth,
	ID3D11ShaderResourceView* luminance, unsigned int w, unsigned int h, ID3D11ShaderResourceView* bloom, int bloomLevels, float bloomStrength)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(m_ParamsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	ParamsBufferType* data = (ParamsBufferType*)mappedResource.pData;
	data->avgLumFactor = 1.0f / (w * h);
	data->whitePoint = m_WhitePoint;
	data->blackPoint = m_BlackPoint;
	data->toe = m_Toe;
	data->shoulder = m_Shoulder;
	data->crossPoint = m_CrossPoint;
	data->bloomLevels = bloomLevels;
	data->bloomStrength = bloomStrength;
	deviceContext->Unmap(m_ParamsBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &m_ParamsBuffer);

	ID3D11ShaderResourceView* psSRVs[4] = { renderTextureColour, renderTextureDepth, luminance, bloom };
	deviceContext->PSSetShaderResources(0, 4, psSRVs);

	deviceContext->PSSetSamplers(0, 1, &m_TrilinearSampler);
}

void FinalPassShader::SettingsGUI()
{
	ImGui::SliderFloat("White Point", &m_WhitePoint, m_CrossPoint, 20.0f);
	ImGui::SliderFloat("Cross Point", &m_CrossPoint, m_BlackPoint, m_WhitePoint);
	ImGui::SliderFloat("Black Point", &m_BlackPoint, 0.0f, m_CrossPoint);

	ImGui::SliderFloat("Toe", &m_Toe, 0.0f, 1.0f);
	ImGui::SliderFloat("Shoulder", &m_Shoulder, 0.0f, 1.0f);
}

void FinalPassShader::CreateShaderResources()
{
	HRESULT hr;

	D3D11_BUFFER_DESC paramsBufferDesc;
	paramsBufferDesc.ByteWidth = sizeof(ParamsBufferType);
	paramsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	paramsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	paramsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	paramsBufferDesc.MiscFlags = 0;
	paramsBufferDesc.StructureByteStride = 0;

	hr = m_Device->CreateBuffer(&paramsBufferDesc, nullptr, &m_ParamsBuffer);
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

	hr = m_Device->CreateSamplerState(&samplerDesc, &m_TrilinearSampler);
	assert(hr == S_OK);
}

void FinalPassShader::UnbindShaderResources(ID3D11DeviceContext* deviceContext)
{
	ID3D11Buffer* nullCB = nullptr;
	deviceContext->PSSetConstantBuffers(0, 1, &nullCB);

	ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
	deviceContext->PSSetShaderResources(0, 2, nullSRVs);

	ID3D11SamplerState* nullSampler = nullptr;
	deviceContext->PSSetSamplers(0, 1, &nullSampler);
}
