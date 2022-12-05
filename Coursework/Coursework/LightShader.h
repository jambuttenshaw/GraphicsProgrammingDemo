#pragma once

#include "DXF.h"

#include "SceneLight.h"
#include "Material.h"
#include "Cubemap.h"

using namespace std;
using namespace DirectX;

#define MAX_LIGHTS 4

class LightShader : public BaseShader
{
private:

	struct CameraBufferType
	{
		XMMATRIX lightViewProj[MAX_LIGHTS];
		XMFLOAT3 cameraPos;
		float padding;
	};

	struct LightBufferType
	{
		XMFLOAT4 globalAmbient;
		XMFLOAT4 irradiance[MAX_LIGHTS];
		XMFLOAT4 positionAndRange[MAX_LIGHTS];
		XMFLOAT4 direction[MAX_LIGHTS];
		// x = type
		// y = range
		// z = inner spot angle
		// w = outer spot angle
		XMFLOAT4 params0[MAX_LIGHTS];
		// x = shadows enabled
		// y = shadow bias
		XMFLOAT4 params1[MAX_LIGHTS];

		int lightCount;
		bool enableEnvironmentalLighting;
		XMFLOAT2 padding;
	};

	struct MaterialBufferType
	{
		XMFLOAT4 albedo;
		float metallic;
		float roughness;
		XMFLOAT2 padding;
	};

public:
	LightShader(ID3D11Device* device, HWND hwnd);
	~LightShader();

	void GlobalLightSettingsGUI();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, size_t lightCount, SceneLight** lights, ID3D11ShaderResourceView* environmentMap, Camera* camera, const Material* mat);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer = nullptr;
	ID3D11Buffer* cameraBuffer = nullptr;
	ID3D11Buffer* lightBuffer = nullptr;
	ID3D11Buffer* materialBuffer = nullptr;

	ID3D11SamplerState* environmentSampler = nullptr;
	ID3D11SamplerState* shadowSampler = nullptr;

	XMFLOAT4 m_GlobalAmbient{ 0.2f, 0.2f, 0.2f, 1.0f };
	bool m_EnableEnvironmentalLighting = false;
};

