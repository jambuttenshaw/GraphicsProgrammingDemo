#include "LightShader.h"

#include "GlobalLighting.h"
#include "ShadowCubemap.h"
#include "imGUI/imgui.h"


LightShader::LightShader(ID3D11Device* device, HWND hwnd, GlobalLighting* globalLighting) : BaseShader(device, hwnd), m_GlobalLighting(globalLighting)
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

void LightShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC cameraBufferDesc;
	D3D11_BUFFER_DESC plmBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC materialBufferDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	HRESULT hr;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	hr = renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);
	assert(hr == S_OK);

	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	hr = renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);
	assert(hr == S_OK);

	plmBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	plmBufferDesc.ByteWidth = sizeof(PointLightMatrixBufferType);
	plmBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	plmBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	plmBufferDesc.MiscFlags = 0;
	plmBufferDesc.StructureByteStride = 0;
	hr = renderer->CreateBuffer(&plmBufferDesc, NULL, &pointLightMatrixBuffer);
	assert(hr == S_OK);

	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	hr = renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);
	assert(hr == S_OK);

	materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth = sizeof(MaterialBufferType);
	materialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	materialBufferDesc.MiscFlags = 0;
	materialBufferDesc.StructureByteStride = 0;
	hr = renderer->CreateBuffer(&materialBufferDesc, NULL, &materialBuffer);
	assert(hr == S_OK);

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
	hr = renderer->CreateSamplerState(&samplerDesc, &materialSampler);
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
	hr = renderer->CreateSamplerState(&shadowSamplerDesc, &shadowSampler);
	assert(hr == S_OK);
}


void LightShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix,
									size_t lightCount, SceneLight** lights, Camera* camera, const Material* mat)
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

	PointLightMatrixBufferType plmb{};
	int index = 0;
	for (int i = 0; i < min(lightCount, MAX_LIGHTS); i++)
	{
		if (!lights[i]->IsEnabled()) continue;

		XMMATRIX tlightMatrix;
		if (lights[i]->GetType() == SceneLight::LightType::Point)
		{
			tlightMatrix = XMMatrixTranspose(lights[i]->GetProjectionMatrix());

			XMMATRIX viewMats[6];
			lights[i]->GetPointLightViewMatrices(viewMats);

			plmb.rightMatrix[index] = XMMatrixTranspose(viewMats[0]);
			plmb.leftMatrix[index] = XMMatrixTranspose(viewMats[1]);
			plmb.upMatrix[index] = XMMatrixTranspose(viewMats[2]);
			plmb.downMatrix[index] = XMMatrixTranspose(viewMats[3]);
			plmb.forwardMatrix[index] = XMMatrixTranspose(viewMats[4]);
			plmb.backMatrix[index] = XMMatrixTranspose(viewMats[5]);
		}
		else
			tlightMatrix = XMMatrixTranspose(lights[i]->GetViewMatrix() * lights[i]->GetProjectionMatrix());
		cameraPtr->lightMatrix[index] = tlightMatrix;

		XMFLOAT3 p = lights[i]->GetPosition();
		cameraPtr->lightPosAndType[index] = { p.x, p.y, p.z, static_cast<float>(lights[i]->GetType()) };

		index++;
	}
	cameraPtr->cameraPos = camera->getPosition();
	cameraPtr->padding = 0.0f;
	deviceContext->Unmap(cameraBuffer, 0);

	deviceContext->Map(pointLightMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	memcpy(mappedResource.pData, &plmb, sizeof(plmb));
	deviceContext->Unmap(pointLightMatrixBuffer, 0);


	ID3D11ShaderResourceView* tex2DBuffer[TEX_BUFFER_SIZE];
	int tex2DCount = 0;
	ID3D11ShaderResourceView* texCubeBuffer[TEX_BUFFER_SIZE];
	int texCubeCount = 0;
	for (int i = 0; i < TEX_BUFFER_SIZE; i++) { tex2DBuffer[i] = nullptr; texCubeBuffer[i] = nullptr; }


	LightBufferType* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	int count = 0;
	for (int i = 0; i < min(lightCount, MAX_LIGHTS); i++)
	{
		SceneLight* light = lights[i];
		
		if (!light->IsEnabled()) continue;

		LightDataType lightData;

		XMFLOAT3 irradiance = light->GetIrradiance();
		lightData.irradiance = { irradiance.x, irradiance.y, irradiance.z, 1.0f };

		XMFLOAT3 p = light->GetPosition();
		lightData.position = XMFLOAT4{ p.x, p.y, p.z, 1.0f };

		XMFLOAT3 d = light->GetDirection();
		lightData.direction = XMFLOAT4{ d.x, d.y, d.z, 0.0f };
		
		lightData.type = static_cast<float>(light->GetType());
		lightData.range = light->GetRange();
		lightData.spotAngles = { cosf(light->GetInnerAngle()), cosf(light->GetOuterAngle()) };
		if (light->IsShadowsEnabled())
		{
			if (light->GetType() == SceneLight::LightType::Point)
			{
				lightData.shadowMapIndex = texCubeCount;
				texCubeBuffer[texCubeCount] = light->GetShadowCubemap()->GetSRV();
				texCubeCount++;
			}
			else
			{
				lightData.shadowMapIndex = tex2DCount;
				tex2DBuffer[tex2DCount] = light->GetShadowMap()->getDepthMapSRV();
				tex2DCount++;
			}
		}
		else
			lightData.shadowMapIndex = -1;
		lightData.shadowBiasCoeffs = light->GetShadowBiasCoeffs();

		lightPtr->lights[count] = lightData;

		count++;
	}
	lightPtr->lightCount = count;
	if (m_GlobalLighting->IsIBLEnabled())
	{
		lightPtr->enableEnvironmentalLighting = true;

		lightPtr->irradianceMapIndex = texCubeCount;
		texCubeBuffer[texCubeCount] = m_GlobalLighting->GetIrradianceMap();
		texCubeCount++;

		lightPtr->prefilterMapIndex = texCubeCount;
		texCubeBuffer[texCubeCount] = m_GlobalLighting->GetPrefilterMap();
		texCubeCount++;

		lightPtr->brdfIntegrationMapIndex = tex2DCount;
		tex2DBuffer[tex2DCount] = m_GlobalLighting->GetBRDFIntegrationMap();
		tex2DCount++;
	}

	lightPtr->padding = { 0.0f, 0.0f, 0.0f };
	
	deviceContext->Unmap(lightBuffer, 0);

	MaterialBufferType* matPtr;
	deviceContext->Map(materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	matPtr = (MaterialBufferType*)mappedResource.pData;
	MaterialDataType matData;

	matData.albedo = mat->GetAlbedo();
	if (mat->UseAlbedoMap())
	{
		matData.albedoMapIndex = tex2DCount;
		tex2DBuffer[tex2DCount++] = mat->GetAlbedoMap();
	}
	else matData.albedoMapIndex = -1;

	matData.roughness = mat->GetRoughness();
	if (mat->UseRoughnessMap())
	{
		matData.roughnessMapIndex= tex2DCount;
		tex2DBuffer[tex2DCount++] = mat->GetRoughnessMap();
	}
	else matData.roughnessMapIndex = -1;

	if (mat->UseNormalMap())
	{
		matData.normalMapIndex = tex2DCount;
		tex2DBuffer[tex2DCount++] = mat->GetNormalMap();
	}
	else matData.normalMapIndex = -1;

	matData.metallic = mat->GetMetalness();

	matPtr->material = matData;
	deviceContext->Unmap(materialBuffer, 0);

	ID3D11Buffer* vsBuffers[3] = { matrixBuffer, cameraBuffer, pointLightMatrixBuffer };
	deviceContext->VSSetConstantBuffers(0, 3, vsBuffers);

	ID3D11Buffer* psBuffers[2] = { lightBuffer, materialBuffer };
	deviceContext->PSSetConstantBuffers(0, 2, psBuffers);
	
	deviceContext->PSSetShaderResources(0, TEX_BUFFER_SIZE, tex2DBuffer);
	deviceContext->PSSetShaderResources(TEX_BUFFER_SIZE, TEX_BUFFER_SIZE, texCubeBuffer);

	ID3D11SamplerState* samplers[4] = { m_GlobalLighting->GetBRDFIntegrationSampler(), m_GlobalLighting->GetCubemapSampler(), materialSampler, shadowSampler };
	deviceContext->PSSetSamplers(0, 4, samplers);
}
