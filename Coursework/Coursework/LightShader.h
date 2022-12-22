#pragma once

#include "DXF.h"

using namespace DirectX;

#include "ShaderUtility.h"


class SceneLight;
class Material;
class GlobalLighting;


class LightShader : public BaseShader
{
private:

	struct CameraBufferType
	{
		XMMATRIX lightMatrix[MAX_LIGHTS];
		XMFLOAT4 lightPosAndType[MAX_LIGHTS];
		XMFLOAT3 cameraPos;
		float padding;
	};
	struct PointLightMatrixBufferType
	{
		XMMATRIX rightMatrix[MAX_LIGHTS];
		XMMATRIX leftMatrix[MAX_LIGHTS];
		XMMATRIX upMatrix[MAX_LIGHTS];
		XMMATRIX downMatrix[MAX_LIGHTS];
		XMMATRIX forwardMatrix[MAX_LIGHTS];
		XMMATRIX backMatrix[MAX_LIGHTS];
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
	ID3D11Buffer* pointLightMatrixBuffer = nullptr;
	ID3D11Buffer* lightBuffer = nullptr;
	ID3D11Buffer* materialBuffer = nullptr;

	ID3D11SamplerState* materialSampler = nullptr;
	ID3D11SamplerState* shadowSampler = nullptr;

	GlobalLighting* m_GlobalLighting;
};

