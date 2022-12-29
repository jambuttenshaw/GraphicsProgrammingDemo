#include "WaterShader.h"

#include "SerializationHelper.h"

#include "SceneLight.h"
#include "RenderTarget.h"
#include "GlobalLighting.h"


WaterShader::WaterShader(ID3D11Device* device, GlobalLighting* globalLighting, ID3D11ShaderResourceView* normalMapA, ID3D11ShaderResourceView* normalMapB)
	: BaseFullScreenShader(device),
	m_NormalMapA(normalMapA), m_NormalMapB(normalMapB), m_GlobalLighting(globalLighting)
{
	Init(L"water_ps.cso");
}


WaterShader::~WaterShader()
{
	if (m_CameraBuffer) m_CameraBuffer->Release();
	if (m_WaterBuffer) m_WaterBuffer->Release();
	if (m_PSLightBuffer) m_PSLightBuffer->Release();
	if (m_NormalMapSamplerState) m_NormalMapSamplerState->Release();
}

void WaterShader::CreateShaderResources()
{
	ShaderUtility::CreateBuffer(m_Device, sizeof(CameraBufferType), &m_CameraBuffer);
	ShaderUtility::CreateBuffer(m_Device, sizeof(WaterBufferType), &m_WaterBuffer);
	ShaderUtility::CreateBuffer(m_Device, sizeof(ShaderUtility::PSLightBufferType), &m_PSLightBuffer);

	D3D11_SAMPLER_DESC normalMapSamplerDesc;
	normalMapSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	normalMapSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	normalMapSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	normalMapSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	normalMapSamplerDesc.MipLODBias = 0.0f;
	normalMapSamplerDesc.MaxAnisotropy = 1;
	normalMapSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	normalMapSamplerDesc.MinLOD = 0;
	normalMapSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	m_Device->CreateSamplerState(&normalMapSamplerDesc, &m_NormalMapSamplerState);
}

void WaterShader::UnbindShaderResources(ID3D11DeviceContext* deviceContext)
{
	ID3D11Buffer* nullBuffers[] = { nullptr, nullptr };
	deviceContext->PSSetConstantBuffers(0, 2, nullBuffers);
	ResourceBuffer emptyResourceBuffer;
	deviceContext->PSSetShaderResources(0, RESOURCE_BUFFER_SIZE, emptyResourceBuffer.GetResourcePtr());
	deviceContext->PSSetShaderResources(RESOURCE_BUFFER_SIZE, RESOURCE_BUFFER_SIZE, emptyResourceBuffer.GetResourcePtr());
	ID3D11SamplerState* nullSampler = nullptr;
	deviceContext->PSSetSamplers(0, 1, &nullSampler);
}

void WaterShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, RenderTarget* renderTarget, SceneLight** lights, size_t lightCount, Camera* camera, float time)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	ResourceBuffer tex2DBuffer, texCubeBuffer;

	{
		deviceContext->Map(m_CameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		CameraBufferType* cameraPtr = (CameraBufferType*)mappedResource.pData;
		cameraPtr->invView = XMMatrixTranspose(XMMatrixInverse(nullptr, viewMatrix));
		cameraPtr->projection = XMMatrixTranspose(projectionMatrix);
		deviceContext->Unmap(m_CameraBuffer, 0);
	}
	{
		deviceContext->Map(m_WaterBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		WaterBufferType* dataPtr = (WaterBufferType*)mappedResource.pData;

		dataPtr->projection = XMMatrixTranspose(projectionMatrix);
		dataPtr->cameraPos = camera->getPosition();

		dataPtr->rtColourMapIndex = tex2DBuffer.AddResource(renderTarget->GetColourSRV());
		dataPtr->rtDepthMapIndex = tex2DBuffer.AddResource(renderTarget->GetDepthSRV());
		dataPtr->normalMapAIndex = tex2DBuffer.AddResource(m_NormalMapA);
		dataPtr->normalMapBIndex = tex2DBuffer.AddResource(m_NormalMapB);

		dataPtr->specularColour = m_SpecularColour;
		dataPtr->transmittanceColour = m_TransmittanceColour;

		dataPtr->oceanBoundsMin = m_OceanBoundsMin;
		dataPtr->oceanBoundsMax = m_OceanBoundsMax;

		dataPtr->transmittanceDepth= m_TransmittanceDepth;
		
		dataPtr->normalMapScale = m_NormalMapScale;
		dataPtr->normalMapStrength = m_NormalMapStrength;

		dataPtr->time = time;
		dataPtr->waveSpeed = m_WaveSpeed;
		dataPtr->waveAngle = m_WaveAngle;

		deviceContext->Unmap(m_WaterBuffer, 0);
	}
	ShaderUtility::ConstructPSLightBuffer(deviceContext, m_PSLightBuffer, lights, lightCount, m_GlobalLighting, &tex2DBuffer, &texCubeBuffer);

	deviceContext->VSSetConstantBuffers(0, 1, &m_CameraBuffer);

	ID3D11Buffer* psCBs[] = { m_WaterBuffer, m_PSLightBuffer };
	deviceContext->PSSetConstantBuffers(0, 2, psCBs);
	ID3D11SamplerState* psSamplers[] = { m_NormalMapSamplerState, m_GlobalLighting->GetBRDFIntegrationSampler(), m_GlobalLighting->GetCubemapSampler() };
	deviceContext->PSSetSamplers(0, 3, psSamplers);

	deviceContext->PSSetShaderResources(0, RESOURCE_BUFFER_SIZE, tex2DBuffer.GetResourcePtr());
	deviceContext->PSSetShaderResources(RESOURCE_BUFFER_SIZE, RESOURCE_BUFFER_SIZE, texCubeBuffer.GetResourcePtr());
}

void WaterShader::SettingsGUI()
{
	ImGui::DragFloat3("Ocean Bounds Min", &m_OceanBoundsMin.x, 0.1f);
	ImGui::DragFloat3("Ocean Bounds Max", &m_OceanBoundsMax.x, 0.1f);
	ImGui::ColorEdit3("Specular Colour", &m_SpecularColour.x);
	ImGui::ColorEdit3("Transmittance Colour", &m_TransmittanceColour.x);
	ImGui::SliderFloat("Transmittance Depth", &m_TransmittanceDepth, 0.0f, 10.0f);
	ImGui::SliderFloat("Normal Map Strength", &m_NormalMapStrength, 0.0f, 1.0f);
	ImGui::DragFloat("Normal Map Scale", &m_NormalMapScale, 0.1f);
	ImGui::DragFloat("Wave Speed", &m_WaveSpeed, 0.001f);
	ImGui::SliderAngle("Wave Angle", &m_WaveAngle, 0.0f, 360.0f);
}


nlohmann::json WaterShader::Serialize() const
{
	nlohmann::json serialized;
	/*
	serialized["oceanBoundsMin"] = SerializationHelper::SerializeFloat3(m_OceanBoundsMin);
	serialized["oceanBoundsMax"] = SerializationHelper::SerializeFloat3(m_OceanBoundsMax);

	serialized["shallowColour"] = SerializationHelper::SerializeFloat4(m_ShallowColour);
	serialized["deepColour"] = SerializationHelper::SerializeFloat4(m_DeepColour);

	serialized["depthMultiplier"] = m_DepthMultiplier;
	serialized["alphaMultiplier"] = m_AlphaMultiplier;

	serialized["normalMapStrength"] = m_NormalMapStrength;
	serialized["normalMapScale"] = m_NormalMapScale;
	serialized["smoothness"] = m_Smoothness;
	*/
	return serialized;
}

void WaterShader::LoadFromJson(const nlohmann::json& data)
{
	/*
	if (data.contains("oceanBoundsMin")) SerializationHelper::LoadFloat3FromJson(&m_OceanBoundsMin, data["oceanBoundsMin"]);
	if (data.contains("oceanBoundsMax")) SerializationHelper::LoadFloat3FromJson(&m_OceanBoundsMax, data["oceanBoundsMax"]);

	if (data.contains("shallowColour")) SerializationHelper::LoadFloat4FromJson(&m_ShallowColour, data["shallowColour"]);
	if (data.contains("deepColour")) SerializationHelper::LoadFloat4FromJson(&m_DeepColour, data["deepColour"]);

	if (data.contains("depthMultiplier")) m_DepthMultiplier = data["depthMultiplier"];
	if (data.contains("alphaMultiplier")) m_AlphaMultiplier = data["alphaMultiplier"];

	if (data.contains("normalMapStrength")) m_NormalMapStrength = data["normalMapStrength"];
	if (data.contains("normalMapScale")) m_NormalMapScale = data["normalMapScale"];
	if (data.contains("smoothness")) m_Smoothness = data["smoothness"];
	*/
}
