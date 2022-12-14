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
	ID3D11ShaderResourceView* luminance, unsigned int w, unsigned int h)
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
	data->padding = { 0.0f, 0.0f };
	deviceContext->Unmap(m_ParamsBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &m_ParamsBuffer);

	ID3D11ShaderResourceView* psSRVs[3] = { renderTextureColour, renderTextureDepth, luminance };
	deviceContext->PSSetShaderResources(0, 3, psSRVs);
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
	D3D11_BUFFER_DESC paramsBufferDesc;
	paramsBufferDesc.ByteWidth = sizeof(ParamsBufferType);
	paramsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	paramsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	paramsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	paramsBufferDesc.MiscFlags = 0;
	paramsBufferDesc.StructureByteStride = 0;
	m_Device->CreateBuffer(&paramsBufferDesc, nullptr, &m_ParamsBuffer);
}

void FinalPassShader::UnbindShaderResources(ID3D11DeviceContext* deviceContext)
{
	ID3D11Buffer* nullCB = nullptr;
	deviceContext->PSSetConstantBuffers(0, 1, &nullCB);

	ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
	deviceContext->PSSetShaderResources(0, 2, nullSRVs);
}
