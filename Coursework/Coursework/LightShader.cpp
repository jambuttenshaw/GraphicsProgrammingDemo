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
	lightBufferDesc.ByteWidth = sizeof(ShaderUtility::LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	hr = renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);
	assert(hr == S_OK);

	materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth = sizeof(ShaderUtility::MaterialBufferType);
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

	ResourceBuffer tex2DBuffer, texCubeBuffer;

	MatrixBufferType* dataPtr;
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = XMMatrixTranspose(worldMatrix);
	dataPtr->view = XMMatrixTranspose(viewMatrix);
	dataPtr->projection = XMMatrixTranspose(projectionMatrix);
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


	ShaderUtility::LightBufferType* lightPtr;
	deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (ShaderUtility::LightBufferType*)mappedResource.pData;
	ShaderUtility::ConstructLightBuffer(lightPtr, lights, lightCount, m_GlobalLighting, &tex2DBuffer, &texCubeBuffer);
	deviceContext->Unmap(lightBuffer, 0);

	ShaderUtility::MaterialBufferType* matPtr;
	deviceContext->Map(materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	matPtr = (ShaderUtility::MaterialBufferType*)mappedResource.pData;
	ShaderUtility::ConstructMaterialData(&matPtr->material, mat, &tex2DBuffer);
	deviceContext->Unmap(materialBuffer, 0);


	ID3D11Buffer* vsBuffers[3] = { matrixBuffer, cameraBuffer, pointLightMatrixBuffer };
	deviceContext->VSSetConstantBuffers(0, 3, vsBuffers);

	ID3D11Buffer* psBuffers[2] = { lightBuffer, materialBuffer };
	deviceContext->PSSetConstantBuffers(0, 2, psBuffers);
	
	deviceContext->PSSetShaderResources(0, RESOURCE_BUFFER_SIZE, tex2DBuffer.GetResourcePtr());
	deviceContext->PSSetShaderResources(RESOURCE_BUFFER_SIZE, RESOURCE_BUFFER_SIZE, texCubeBuffer.GetResourcePtr());

	ID3D11SamplerState* samplers[4] = { m_GlobalLighting->GetBRDFIntegrationSampler(), m_GlobalLighting->GetCubemapSampler(), materialSampler, shadowSampler };
	deviceContext->PSSetSamplers(0, 4, samplers);
}
