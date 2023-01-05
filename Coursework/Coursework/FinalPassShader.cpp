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
	ID3D11ShaderResourceView* luminance, unsigned int w, unsigned int h, ID3D11ShaderResourceView* bloom)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(m_ParamsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	ParamsBufferType* data = (ParamsBufferType*)mappedResource.pData;

	data->enableTonemapping = m_EnableTonemapping;
	data->avgLumFactor = 1.0f / (w * h);

	data->hdrMax = m_HDRMax;
	data->contrast = m_Contrast;
	data->shoulder = m_Shoulder;
	data->midIn = m_MidIn;
	data->midOut = m_MidOut;
	data->crosstalk = m_Crosstalk;
	data->white = m_White;

	data->enableBloom = m_EnableBloom;
	data->bloomStrength = m_BloomStrength;

	deviceContext->Unmap(m_ParamsBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &m_ParamsBuffer);

	ID3D11ShaderResourceView* psSRVs[4] = { renderTextureColour, renderTextureDepth, luminance, bloom };
	deviceContext->PSSetShaderResources(0, 4, psSRVs);

	deviceContext->PSSetSamplers(0, 1, &m_TrilinearSampler);
}

void FinalPassShader::SettingsGUI()
{
	ImGui::Checkbox("Enable Tonemapping", &m_EnableTonemapping);

	ImGui::DragFloat("HDR Max", &m_HDRMax, 0.01f);
	ImGui::DragFloat("Contrast", &m_Contrast, 0.005f);
	ImGui::DragFloat("Shoulder", &m_Shoulder, 0.001f);
	ImGui::DragFloat("Mid In", &m_MidIn, 0.001f);
	ImGui::DragFloat("Mid Out", &m_MidOut, 0.001f);
	ImGui::DragFloat("Crosstalk", &m_Crosstalk, 0.005f);
	ImGui::DragFloat("White", &m_White, 0.005f);

	ImGui::Separator();

	ImGui::Checkbox("Enable Bloom", &m_EnableBloom);
	ImGui::DragFloat("Bloom Strength", &m_BloomStrength, 0.01f);
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
