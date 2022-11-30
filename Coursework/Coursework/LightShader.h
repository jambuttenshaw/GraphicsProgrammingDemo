#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

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
		XMFLOAT4 irradiance;
		XMFLOAT3 direction;
		float padding;
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

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, Light* light, Camera* camera);

	void materialGUI();

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer = nullptr;
	ID3D11Buffer* cameraBuffer = nullptr;
	ID3D11Buffer* lightBuffer = nullptr;
	ID3D11Buffer* materialBuffer = nullptr;

	XMFLOAT4 albedo{ 1.0f, 1.0f, 1.0f, 1.0f };
	float metallic = 0.0f;
	float roughness = 1.0f;
	float lightStrength = 1.0f;
};

