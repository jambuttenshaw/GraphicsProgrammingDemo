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
		XMFLOAT4 position[MAX_LIGHTS];
		XMFLOAT4 direction[MAX_LIGHTS];
		// params[0] = type
		// params[1] = inner spot angle
		// params[2] = outer spot angle
		// params[3] = shadows enabled
		XMFLOAT4 params[MAX_LIGHTS];
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
	bool m_EnableEnvironmentalLighting = true;
};

