#pragma once

#include "DXF.h"
#include "BaseFullScreenShader.h"
#include "ShaderUtility.h"

class SceneLight;
class RenderTarget;
class GlobalLighting;

using namespace DirectX;


class WaterShader : public BaseFullScreenShader
{
private:
	// used to calculate view directions in VS
	struct CameraBufferType
	{
		XMMATRIX projection;
		XMMATRIX invView;
	};

	// properties that control the look of the water
	struct WaterBufferType
	{
		// the projection matrix of the scene is used to calculate linear depth
		// to calculate linear depth
		XMMATRIX projection; 

		XMFLOAT4 specularColour;
		XMFLOAT4 transmittanceColour;

		int rtColourMapIndex;
		int rtDepthMapIndex;
		int normalMapAIndex;
		int normalMapBIndex;

		XMFLOAT3 cameraPos;
		float transmittanceDepth;

		XMFLOAT3 oceanBoundsMin;
		float normalMapScale;

		XMFLOAT3 oceanBoundsMax;
		float normalMapStrength;

		float time;
		float waveSpeed;
		float waveAngle;
		float specularBrightness;
	};

public:
	WaterShader(ID3D11Device* device, GlobalLighting* globalLighting, ID3D11ShaderResourceView* normalMapA, ID3D11ShaderResourceView* normalMapB);
	~WaterShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, RenderTarget* renderTarget, SceneLight** lights, size_t lightCount, Camera* camera, float time);

	void SettingsGUI();

protected:

	virtual void CreateShaderResources() override;
	virtual void UnbindShaderResources(ID3D11DeviceContext* deviceContext) override;

private:
	ID3D11Buffer* m_CameraBuffer = nullptr;
	ID3D11Buffer* m_WaterBuffer = nullptr;
	ID3D11Buffer* m_PSLightBuffer = nullptr;
	ID3D11SamplerState* m_NormalMapSamplerState = nullptr;

	XMFLOAT3 m_OceanBoundsMin = { -50.0f, -10.0f, -50.0f };
	XMFLOAT3 m_OceanBoundsMax = { 50.0f, 0.0f, 50.0f };

	XMFLOAT4 m_SpecularColour = { 0.125f, 0.125f, 0.125f, 1.0f };
	XMFLOAT4 m_TransmittanceColour = { 0.067f, 0.608f, 0.945f, 1.0f };

	float m_TransmittanceDepth = 2.0f;

	float m_NormalMapStrength = 0.6f;
	float m_NormalMapScale = 30.0f;

	float m_WaveSpeed = 0.1f;
	float m_WaveAngle = 0.0f;

	float m_SpecularBrightness = 0.6f;

	ID3D11ShaderResourceView* m_NormalMapA = nullptr;
	ID3D11ShaderResourceView* m_NormalMapB = nullptr;

	GlobalLighting* m_GlobalLighting = nullptr;
};
