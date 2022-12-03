#include "LightShader.h"

#include "imGUI/imgui.h"


LightShader::LightShader(ID3D11Device* device, HWND hwnd) : BaseShader(device, hwnd)
{
	initShader(L"lighting_vs.cso", L"lighting_ps.cso");
}


LightShader::~LightShader()
{
	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	// Release the light constant buffer.
	if (lightBuffer)
	{
		lightBuffer->Release();
		lightBuffer = 0;
	}
}

void LightShader::GlobalLightSettingsGUI()
{
	ImGui::ColorEdit3("Global Ambience", &m_GlobalAmbient.x);
	ImGui::Checkbox("Enable Environmental Lighting", &m_EnableEnvironmentalLighting);
}

void LightShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC materialBufferDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);

	materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth = sizeof(MaterialBufferType);
	materialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	materialBufferDesc.MiscFlags = 0;
	materialBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&materialBufferDesc, NULL, &materialBuffer);

	// setup cubemap sampler state
	D3D11_SAMPLER_DESC environmentSamplerDesc;
	environmentSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	environmentSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	environmentSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	environmentSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	environmentSamplerDesc.MipLODBias = 0.0f;
	environmentSamplerDesc.MaxAnisotropy = 1;
	environmentSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	environmentSamplerDesc.MinLOD = 0;
	environmentSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&environmentSamplerDesc, &environmentSampler);

	D3D11_SAMPLER_DESC shadowSamplerDesc;
	shadowSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSamplerDesc.BorderColor[0] = 1.0f;
	shadowSamplerDesc.BorderColor[1] = 1.0f;
	shadowSamplerDesc.BorderColor[2] = 1.0f;
	shadowSamplerDesc.BorderColor[3] = 1.0f;
	renderer->CreateSamplerState(&shadowSamplerDesc, &shadowSampler);
}


void LightShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
									size_t lightCount, const SceneLight* lights, ID3D11ShaderResourceView* environmentMap, Camera* camera, const Material* mat)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;


	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld = XMMatrixTranspose(worldMatrix);
	XMMATRIX tview = XMMatrixTranspose(viewMatrix);
	XMMATRIX tproj = XMMatrixTranspose(projectionMatrix);

	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);

	CameraBufferType* cameraPtr;
	deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cameraPtr = (CameraBufferType*)mappedResource.pData;
	for (int i = 0; i < min(lightCount, MAX_LIGHTS); i++)
	{
		XMMATRIX tlightViewProj = XMMatrixTranspose(lights[i].GetViewMatrix() * lights[i].GetOrthoMatrix());
		cameraPtr->lightViewProj[i] = tlightViewProj;
	}
	cameraPtr->cameraPos = camera->getPosition();
	cameraPtr->padding = 0.0f;
	deviceContext->Unmap(cameraBuffer, 0);

	LightBufferType* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->globalAmbient = m_GlobalAmbient;
	int count = 0;
	for (int i = 0; i < min(lightCount, MAX_LIGHTS); i++)
	{
		const SceneLight* light = lights + i;
		
		if (!light->IsEnabled()) continue;

		XMFLOAT3 irradiance = light->GetIrradiance();
		lightPtr->irradiance[count] = { irradiance.x, irradiance.y, irradiance.z, 1.0f };

		XMFLOAT3 p = light->GetPosition();
		lightPtr->position[count] = XMFLOAT4{ p.x, p.y, p.z, 0.0f };

		XMFLOAT3 d = light->GetDirection();
		lightPtr->direction[count] = XMFLOAT4{ d.x, d.y, d.z, 0.0f };
		
		lightPtr->params[count] = { static_cast<float>(light->GetType()), cosf(light->GetInnerAngle()), cosf(light->GetOuterAngle()), light->IsShadowsEnabled() ? 1.0f : 0.0f };

		count++;
	}
	lightPtr->lightCount = count;
	lightPtr->enableEnvironmentalLighting = m_EnableEnvironmentalLighting;
	lightPtr->padding = { 0.0f, 0.0f };
	deviceContext->Unmap(lightBuffer, 0);

	MaterialBufferType* matPtr;
	deviceContext->Map(materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	matPtr = (MaterialBufferType*)mappedResource.pData;
	matPtr->albedo = mat->GetAlbedo();
	matPtr->metallic = mat->GetMetalness();
	matPtr->roughness = mat->GetRoughness();
	matPtr->padding = { 0.0f, 0.0f };
	deviceContext->Unmap(materialBuffer, 0);

	ID3D11Buffer* vsBuffers[2] = { matrixBuffer, cameraBuffer };
	deviceContext->VSSetConstantBuffers(0, 2, vsBuffers);

	ID3D11Buffer* psBuffers[2] = { lightBuffer, materialBuffer };
	deviceContext->PSSetConstantBuffers(0, 2, psBuffers);

	ID3D11ShaderResourceView* shadowMaps[4];
	for (int i = 0; i < min(lightCount, MAX_LIGHTS); i++)
	{
		if (lights[i].IsShadowsEnabled())
			shadowMaps[i] = lights[i].GetShadowMap()->getDepthMapSRV();
		else
			shadowMaps[i] = nullptr;
	}
	deviceContext->PSSetShaderResources(0, 1, &environmentMap);
	deviceContext->PSSetShaderResources(1, 4, shadowMaps);

	ID3D11SamplerState* samplers[2] = { environmentSampler, shadowSampler };
	deviceContext->PSSetSamplers(0, 2, samplers);
}
