#include "ShaderUtility.h"

#include "Material.h"
#include "SceneLight.h"
#include "ShadowCubemap.h"
#include "GlobalLighting.h"


void ShaderUtility::CreateBuffer(ID3D11Device* device, UINT byteWidth, ID3D11Buffer** ppBuffer)
{
	assert(byteWidth % 16 == 0 && "Constant buffer byte width must be multiple of 16!");

	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = byteWidth;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	HRESULT hr = device->CreateBuffer(&desc, NULL, ppBuffer);
	assert(hr == S_OK);
}


void ShaderUtility::ConstructLightData(LightDataType* lightData, const SceneLight* light, ResourceBuffer* tex2DBuffer, ResourceBuffer* texCubeBuffer)
{
	// if you are trying to construct light data for disabled lights, then something is wrong with your logic outside of this function
	assert(light->IsEnabled());

	XMFLOAT3 irradiance = light->GetIrradiance();
	lightData->irradiance = { irradiance.x, irradiance.y, irradiance.z, 1.0f };

	XMFLOAT3 p = light->GetPosition();
	lightData->position = XMFLOAT4{ p.x, p.y, p.z, 1.0f };

	XMFLOAT3 d = light->GetDirection();
	lightData->direction = XMFLOAT4{ d.x, d.y, d.z, 0.0f };

	lightData->type = static_cast<float>(light->GetType());
	lightData->range = light->GetRange();
	lightData->spotAngles = { cosf(light->GetInnerAngle()), cosf(light->GetOuterAngle()) };

	if (light->IsShadowsEnabled())
	{
		if (light->GetType() == SceneLight::LightType::Point)
			lightData->shadowMapIndex = texCubeBuffer->AddResource(light->GetShadowCubemap()->GetSRV());
		else
			lightData->shadowMapIndex = tex2DBuffer->AddResource(light->GetShadowMap()->getDepthMapSRV());
	}
	else
		lightData->shadowMapIndex = -1;
	lightData->shadowBiasCoeffs = light->GetShadowBiasCoeffs();
}


void ShaderUtility::ConstructMaterialData(MaterialDataType* matData, const Material* mat, ResourceBuffer* tex2DBuffer)
{
	matData->albedo = mat->GetAlbedo();
	if (mat->UseAlbedoMap())
		matData->albedoMapIndex = tex2DBuffer->AddResource(mat->GetAlbedoMap());
	else
		matData->albedoMapIndex = -1;

	matData->roughness = mat->GetRoughness();
	if (mat->UseRoughnessMap())
		matData->roughnessMapIndex = tex2DBuffer->AddResource(mat->GetRoughnessMap());
	else
		matData->roughnessMapIndex = -1;

	if (mat->UseNormalMap())
		matData->normalMapIndex = tex2DBuffer->AddResource(mat->GetNormalMap());
	else
		matData->normalMapIndex = -1;

	matData->metallic = mat->GetMetalness();
}



void ShaderUtility::ConstructVSLightBuffer(ID3D11DeviceContext* deviceContext, ID3D11Buffer* lightBuffer, SceneLight** lights, size_t lightCount, Camera* camera)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(hr == S_OK);
	VSLightBufferType* bufferPtr = reinterpret_cast<VSLightBufferType*>(mappedResource.pData);

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

			bufferPtr->pointLightMatrices.right[index] = XMMatrixTranspose(viewMats[0]);
			bufferPtr->pointLightMatrices.left[index] = XMMatrixTranspose(viewMats[1]);
			bufferPtr->pointLightMatrices.up[index] = XMMatrixTranspose(viewMats[2]);
			bufferPtr->pointLightMatrices.down[index] = XMMatrixTranspose(viewMats[3]);
			bufferPtr->pointLightMatrices.forward[index] = XMMatrixTranspose(viewMats[4]);
			bufferPtr->pointLightMatrices.back[index] = XMMatrixTranspose(viewMats[5]);
		}
		else
			tlightMatrix = XMMatrixTranspose(lights[i]->GetViewMatrix() * lights[i]->GetProjectionMatrix());
		bufferPtr->lightMatrix[index] = tlightMatrix;

		XMFLOAT3 p = lights[i]->GetPosition();
		bufferPtr->lightPosAndType[index] = { p.x, p.y, p.z, static_cast<float>(lights[i]->GetType()) };

		index++;
	}
	bufferPtr->cameraPos = camera->getPosition();

	deviceContext->Unmap(lightBuffer, 0);
}


void ShaderUtility::ConstructPSLightBuffer(ID3D11DeviceContext* deviceContext, ID3D11Buffer* lightBuffer, SceneLight** lights, size_t lightCount, GlobalLighting* globalLighting, ResourceBuffer* tex2DBuffer, ResourceBuffer* texCubeBuffer)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = deviceContext->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(hr == S_OK);
	PSLightBufferType* bufferPtr = reinterpret_cast<PSLightBufferType*>(mappedResource.pData);

	int count = 0;
	for (int i = 0; i < min(lightCount, MAX_LIGHTS); i++)
	{
		if (!lights[i]->IsEnabled()) continue;
		ShaderUtility::ConstructLightData(&bufferPtr->lights[count], lights[i], tex2DBuffer, texCubeBuffer);
		count++;
	}
	bufferPtr->lightCount = count;
	if (globalLighting->IsIBLEnabled())
	{
		bufferPtr->enableEnvironmentalLighting = true;

		bufferPtr->irradianceMapIndex = texCubeBuffer->AddResource(globalLighting->GetIrradianceMap());
		bufferPtr->prefilterMapIndex = texCubeBuffer->AddResource(globalLighting->GetPrefilterMap());
		bufferPtr->brdfIntegrationMapIndex = tex2DBuffer->AddResource(globalLighting->GetBRDFIntegrationMap());
	}
	else
		bufferPtr->enableEnvironmentalLighting = false;

	deviceContext->Unmap(lightBuffer, 0);
}

void ShaderUtility::ConstructMaterialBuffer(ID3D11DeviceContext* deviceContext, ID3D11Buffer* matBuffer, Material* const* mats, size_t matCount, ResourceBuffer* tex2DBuffer)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = deviceContext->Map(matBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	assert(hr == S_OK);
	MaterialBufferType* bufferPtr = reinterpret_cast<MaterialBufferType*>(mappedResource.pData);

	for (size_t i = 0; i < min(matCount, MAX_MATERIALS); i++)
		ConstructMaterialData(&bufferPtr->materials[i], mats[i], tex2DBuffer);

	bufferPtr->materialCount = static_cast<int>(matCount);

	deviceContext->Unmap(matBuffer, 0);
}
