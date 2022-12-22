#include "ShaderUtility.h"

#include "Material.h"
#include "SceneLight.h"
#include "ShadowCubemap.h"
#include "GlobalLighting.h"


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

void ShaderUtility::ConstructLightBuffer(LightBufferType* lightBuffer, SceneLight** lights, size_t lightCount, GlobalLighting* globalLighting, ResourceBuffer* tex2DBuffer, ResourceBuffer* texCubeBuffer)
{
	int count = 0;
	for (int i = 0; i < min(lightCount, MAX_LIGHTS); i++)
	{
		if (!lights[i]->IsEnabled()) continue;
		ShaderUtility::ConstructLightData(&lightBuffer->lights[count], lights[i], tex2DBuffer, texCubeBuffer);
		count++;
	}
	lightBuffer->lightCount = count;
	if (globalLighting->IsIBLEnabled())
	{
		lightBuffer->enableEnvironmentalLighting = true;

		lightBuffer->irradianceMapIndex = texCubeBuffer->AddResource(globalLighting->GetIrradianceMap());
		lightBuffer->prefilterMapIndex = texCubeBuffer->AddResource(globalLighting->GetPrefilterMap());
		lightBuffer->brdfIntegrationMapIndex = tex2DBuffer->AddResource(globalLighting->GetBRDFIntegrationMap());
	}
	else
		lightBuffer->enableEnvironmentalLighting = false;
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
