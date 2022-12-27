#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "DXF.h"

using namespace DirectX;

#include <cassert>
#include <array>

class Material;
class SceneLight;
class GlobalLighting;


#define MAX_LIGHTS 4
#define MAX_MATERIALS 8
#define RESOURCE_BUFFER_SIZE 32

class ResourceBuffer
{
public:
	ResourceBuffer()
	{
		m_Resources.fill(nullptr);
	}
	~ResourceBuffer() = default;

	inline int AddResource(ID3D11ShaderResourceView* resource)
	{ 
		assert(m_Count < RESOURCE_BUFFER_SIZE && "Overfilling resource buffer!");
		m_Resources[m_Count] = resource; 
		return static_cast<int>(m_Count++);
	}

	inline ID3D11ShaderResourceView* const* GetResourcePtr() const
	{ 
		ID3D11ShaderResourceView* const& srv = m_Resources.at(0); 
		return &srv;
	}

private:
	std::array<ID3D11ShaderResourceView*, RESOURCE_BUFFER_SIZE> m_Resources;
	size_t m_Count = 0;
};


class ShaderUtility
{
public:
	// constant buffer type definitions
	struct LightDataType
	{
		XMFLOAT4 irradiance;
		XMFLOAT4 position;
		XMFLOAT4 direction;

		float type;
		float range;
		XMFLOAT2 spotAngles;

		int shadowMapIndex;
		XMFLOAT2 shadowBiasCoeffs;

		float padding;
	};
	struct MaterialDataType
	{
		XMFLOAT3 albedo;
		int albedoMapIndex;
		float roughness;
		int roughnessMapIndex;
		float metallic;
		int normalMapIndex;
	};

	struct VSLightBufferType
	{
		XMMATRIX lightMatrix[MAX_LIGHTS];
		XMFLOAT4 lightPosAndType[MAX_LIGHTS];
		XMFLOAT3 cameraPos;
		float padding;
		struct {
			XMMATRIX right[MAX_LIGHTS];
			XMMATRIX left[MAX_LIGHTS];
			XMMATRIX up[MAX_LIGHTS];
			XMMATRIX down[MAX_LIGHTS];
			XMMATRIX forward[MAX_LIGHTS];
			XMMATRIX back[MAX_LIGHTS];
		} pointLightMatrices;
	};
	struct PSLightBufferType
	{
		LightDataType lights[MAX_LIGHTS];

		int lightCount;
		bool enableEnvironmentalLighting;
		int irradianceMapIndex;
		int prefilterMapIndex;

		int brdfIntegrationMapIndex;
		XMFLOAT3 padding;
	};

	struct MaterialBufferType
	{
		MaterialDataType materials[MAX_MATERIALS];
		int materialCount;
		XMFLOAT3 padding;
	};

public:
	// pure static helper class
	ShaderUtility() = delete;
	~ShaderUtility() = delete;

	static void CreateBuffer(ID3D11Device* device, UINT byteWidth, ID3D11Buffer** ppBuffer);

	static void ConstructLightData(LightDataType* lightData, const SceneLight* light, ResourceBuffer* tex2DBuffer, ResourceBuffer* texCubeBuffer);
	static void ConstructMaterialData(MaterialDataType* matData, const Material* mat, ResourceBuffer* tex2DBuffer);

	static void ConstructVSLightBuffer(ID3D11DeviceContext* deviceContext, ID3D11Buffer* lightBuffer,
		SceneLight** lights, size_t lightCount, Camera* camera);
	static void ConstructPSLightBuffer(ID3D11DeviceContext* deviceContext, ID3D11Buffer* lightBuffer,
		SceneLight** lights, size_t lightCount, GlobalLighting* globalLighting,
		ResourceBuffer* tex2DBuffer, ResourceBuffer* texCubeBuffer);

	static void ConstructMaterialBuffer(ID3D11DeviceContext* deviceContext, ID3D11Buffer* matBuffer,
		Material* const* mats, size_t matCount,
		ResourceBuffer* tex2DBuffer);

};