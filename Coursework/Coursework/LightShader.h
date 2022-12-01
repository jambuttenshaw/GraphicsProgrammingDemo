#pragma once

#include "DXF.h"

#include "SceneLight.h"
#include "Material.h"

using namespace std;
using namespace DirectX;

#define MAX_LIGHTS 4

class LightShader : public BaseShader
{
private:
	struct CameraBufferType
	{
		XMFLOAT3 cameraPos;
		float padding;
	};

	struct LightBufferType
	{
		XMFLOAT4 irradiance[MAX_LIGHTS];
		XMFLOAT4 position[MAX_LIGHTS];
		XMFLOAT4 direction[MAX_LIGHTS];
		XMFLOAT4 typeAndSpotAngles[MAX_LIGHTS];
		float lightCount;
		XMFLOAT3 padding;
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

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, int lightCount, const SceneLight* lights, Camera* camera, const Material* mat);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer = nullptr;
	ID3D11Buffer* cameraBuffer = nullptr;
	ID3D11Buffer* lightBuffer = nullptr;
	ID3D11Buffer* materialBuffer = nullptr;
};

