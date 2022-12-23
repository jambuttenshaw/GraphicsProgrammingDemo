#pragma once

#include "DXF.h"

using namespace DirectX;

#include "ShaderUtility.h"


class SceneLight;
class Material;
class GlobalLighting;


class LightShader : public BaseShader
{
public:
	LightShader(ID3D11Device* device, HWND hwnd, GlobalLighting* globalLighing);
	~LightShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, size_t lightCount, SceneLight** lights, Camera* camera, const Material* mat);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* m_MatrixBuffer = nullptr;
	ID3D11Buffer* m_VSLightBuffer = nullptr;
	ID3D11Buffer* m_PSLightBuffer = nullptr;
	ID3D11Buffer* m_MaterialBuffer = nullptr;

	ID3D11SamplerState* m_MaterialSampler = nullptr;
	ID3D11SamplerState* m_ShadowSampler = nullptr;

	GlobalLighting* m_GlobalLighting = nullptr;
};

