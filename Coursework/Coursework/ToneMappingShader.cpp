#include "ToneMappingShader.h"

#include "imGUI/imgui.h"


ToneMappingShader::ToneMappingShader(ID3D11Device* device)
	: BaseFullScreenShader(device)
{
	Init(L"tonemapping_ps.cso");
}

ToneMappingShader::~ToneMappingShader()
{
	m_ParamsBuffer->Release();
}

void ToneMappingShader::setShaderParameters(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* renderTextureColour, ID3D11ShaderResourceView* renderTextureDepth)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	deviceContext->Map(m_ParamsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	ParamsBufferType* data = (ParamsBufferType*)mappedResource.pData;
	data->avgL = m_AvgL;
	data->whitePoint = m_WhitePoint;
	data->blackPoint = m_BlackPoint;
	data->toe = m_Toe;
	data->shoulder = m_Shoulder;
	data->crossPoint = m_CrossPoint;
	data->padding = { 0.0f, 0.0f };
	deviceContext->Unmap(m_ParamsBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &m_ParamsBuffer);

	ID3D11ShaderResourceView* psSRVs[2] = { renderTextureColour, renderTextureDepth };
	deviceContext->PSSetShaderResources(0, 2, psSRVs);
}

void ToneMappingShader::SettingsGUI()
{
	ImGui::SliderFloat("White Point", &m_WhitePoint, m_CrossPoint, 20.0f);
	ImGui::SliderFloat("Cross Point", &m_CrossPoint, m_BlackPoint, m_WhitePoint);
	ImGui::SliderFloat("Black Point", &m_BlackPoint, 0.0f, m_CrossPoint);

	ImGui::SliderFloat("Toe", &m_Toe, 0.0f, 1.0f);
	ImGui::SliderFloat("Shoulder", &m_Shoulder, 0.0f, 1.0f);
}

void ToneMappingShader::CreateShaderResources()
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

void ToneMappingShader::UnbindShaderResources(ID3D11DeviceContext* deviceContext)
{
	ID3D11Buffer* nullCB = nullptr;
	deviceContext->PSSetConstantBuffers(0, 1, &nullCB);

	ID3D11ShaderResourceView* nullSRVs[2] = { nullptr, nullptr };
	deviceContext->PSSetShaderResources(0, 2, nullSRVs);
}
