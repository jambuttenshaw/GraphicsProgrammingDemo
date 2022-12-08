#pragma once

#include "DXF.h"

#include "SceneLight.h"
#include "Material.h"
#include "Cubemap.h"

using namespace std;
using namespace DirectX;

#define MAX_LIGHTS 4

class GlobalLighting;


class LightShader : public BaseShader
{
private:

	struct CameraBufferType
	{
		XMMATRIX lightViewProj[MAX_LIGHTS];
		XMFLOAT3 cameraPos;
		float padding;
	};

	struct LightDataType
	{
		XMFLOAT4 irradiance;
		XMFLOAT4 position;
		XMFLOAT4 direction;

		float type;
		float range;
		XMFLOAT2 spotAngles;

		float shadowsEnabled;
		float shadowBias;

		XMFLOAT2 padding;
	};
	struct LightBufferType
	{
		LightDataType lights[MAX_LIGHTS];

		int lightCount;
		bool enableEnvironmentalLighting;
		XMFLOAT2 padding;
	};

	struct MaterialDataType
	{
		XMFLOAT3 albedo;
		float useAlbedoTexture;
		float roughness;
		float useRoughnessMap;
		float metallic;
		float useNormalMap;
	};
	struct MaterialBufferType
	{
		MaterialDataType material;
	};

public:
	LightShader(ID3D11Device* device, HWND hwnd, GlobalLighting* globalLighing);
	~LightShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, size_t lightCount, SceneLight** lights, Camera* camera, const Material* mat);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer = nullptr;
	ID3D11Buffer* cameraBuffer = nullptr;
	ID3D11Buffer* lightBuffer = nullptr;
	ID3D11Buffer* materialBuffer = nullptr;

	ID3D11SamplerState* materialSampler = nullptr;
	ID3D11SamplerState* shadowSampler = nullptr;

	GlobalLighting* m_GlobalLighting;
};

