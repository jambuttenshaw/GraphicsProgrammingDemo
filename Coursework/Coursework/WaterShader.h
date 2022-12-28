#pragma once

#include "DXF.h"
#include "BaseFullScreenShader.h"
#include "ShaderUtility.h"

#include <nlohmann/json.hpp>

class SceneLight;
class RenderTarget;
class GlobalLighting;

using namespace DirectX;


class WaterShader : public BaseFullScreenShader
{
private:
	struct CameraBufferType
	{
		XMMATRIX projection;
		XMMATRIX invView;
	};

	struct WaterBufferType
	{
		XMMATRIX projection;

		XMFLOAT4 specularColour;
		XMFLOAT4 transmittanceColour;

		int rtColourMapIndex;
		int rtDepthMapIndex;
		int normalMapAIndex;
		int normalMapBIndex;

		XMFLOAT3 cameraPos;
		float roughness;

		XMFLOAT3 oceanBoundsMin;
		float transmittanceDepth;

		XMFLOAT3 oceanBoundsMax;
		float normalMapScale;

		float normalMapStrength;
		float time;
		XMFLOAT2 padding;
	};

public:
	WaterShader(ID3D11Device* device, GlobalLighting* globalLighting, ID3D11ShaderResourceView* normalMapA, ID3D11ShaderResourceView* normalMapB);
	~WaterShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, RenderTarget* renderTarget, SceneLight** lights, size_t lightCount, Camera* camera, float time);

	void SettingsGUI();

	nlohmann::json Serialize() const;
	void LoadFromJson(const nlohmann::json& data);

protected:

	virtual void CreateShaderResources() override;
	virtual void UnbindShaderResources(ID3D11DeviceContext* deviceContext) override;

private:
	ID3D11Buffer* m_CameraBuffer = nullptr;
	ID3D11Buffer* m_WaterBuffer = nullptr;
	ID3D11Buffer* m_PSLightBuffer = nullptr;
	ID3D11SamplerState* m_NormalMapSamplerState = nullptr;

	XMFLOAT3 m_OceanBoundsMin = { -25.0f, -10.0f, -25.0f };
	XMFLOAT3 m_OceanBoundsMax = { 25.0f, 0.0f, 25.0f };

	XMFLOAT4 m_SpecularColour = { 0.125f, 0.125f, 0.125f, 1.0f };
	XMFLOAT4 m_TransmittanceColour = { 0.067f, 0.608f, 0.945f, 1.0f };

	float m_Roughness = 0.01f;
	float m_TransmittanceDepth = 2.0f;

	float m_NormalMapStrength = 1.0f;
	float m_NormalMapScale = 30.0f;

	ID3D11ShaderResourceView* m_NormalMapA = nullptr;
	ID3D11ShaderResourceView* m_NormalMapB = nullptr;

	GlobalLighting* m_GlobalLighting = nullptr;
};
