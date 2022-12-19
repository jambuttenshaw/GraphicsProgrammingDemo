#pragma once

#include "DXF.h"
#include <DirectXMath.h>

using namespace DirectX;

class ShadowCubemap;


class SceneLight
{
public:
	enum class LightType : int
	{
		Directional = 0,
		Point = 1,
		Spot = 2
	};

public:

	SceneLight(ID3D11Device* device);
	~SceneLight();

	void SettingsGUI();

	// getters and setters
	inline bool IsEnabled() const { return m_Enabled; }
	inline void SetEnbled(bool e) { m_Enabled = e; }

	inline LightType GetType() const { return m_Type; }
	inline void SetType(LightType t) { m_Type = t; }

	inline const XMFLOAT3& GetColour() const { return m_Colour; }
	inline void SetColour(const XMFLOAT3& c) { m_Colour = c; }
	inline float GetIntensity() const { return m_Intensity; }
	inline void SetIntensity(float i) { m_Intensity = i; }
	XMFLOAT3 GetIrradiance() const;

	inline const XMFLOAT3& GetPosition() const { return m_Position; }
	inline void SetPosition(const XMFLOAT3& p) { m_Position = p; }
	inline const XMFLOAT3& GetDirection() const { return m_Direction; }
	inline void SetYaw(float y) { m_Yaw = y; CalculateDirectionFromEulerAngles(); }
	inline void SetPitch(float p) { m_Pitch = p; CalculateDirectionFromEulerAngles(); }
	inline float GetRange() const { return m_Range; }
	inline void SetRange(float r) { m_Range = r; }

	inline float GetInnerAngle() const { return m_InnerAngle; }
	inline float GetOuterAngle() const { return m_OuterAngle; }
	inline void SetInnerAngle(float a) { m_InnerAngle = a; }
	inline void SetOuterAngle(float a) { m_OuterAngle = a; }

	// light matrices
	inline const XMMATRIX& GetViewMatrix() const { return m_ViewMatrix; }
	const XMMATRIX& GetProjectionMatrix() const;
	void GenerateViewMatrix();
	void GenerateOrthoMatrix(float screenWidth, float screenHeight, float nearPlane, float farPlane);
	void GeneratePerspectiveMatrix(float nearPlane, float farPlane);

	void GetPointLightViewMatrices(XMMATRIX* matArray);
	
	// shadow mapping
	void EnableShadows();
	void DisableShadows();
	inline bool IsShadowsEnabled() const { return m_ShadowsEnabled; }
	
	inline ShadowMap* GetShadowMap() const { return m_ShadowMap; }
	inline ShadowCubemap* GetShadowCubemap() const { return m_ShadowCubeMap; }

	inline XMFLOAT2 GetShadowBiasCoeffs() const { return m_ShadowBiasCoeffs; }
	inline void SetShadowBiasCoeffs(const XMFLOAT2& c) { m_ShadowBiasCoeffs = c; }


private:
	void CalculateDirectionFromEulerAngles();
	
	void CreateShadowMap();

private:
	ID3D11Device* m_Device = nullptr;

	bool m_Enabled = false;

	LightType m_Type = LightType::Directional;

	XMFLOAT3 m_Colour{ 1.0f, 1.0f, 1.0f };
	float m_Intensity = 1.0f;

	XMFLOAT3 m_Position{ 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_Direction{ 0.0f, 0.0f, 1.0f };

	float m_Range = 5.0f;

	// for directional lights only
	float m_Yaw = 0.0f, m_Pitch = 0.0f;

	// for spot lights only
	float m_InnerAngle = 0.175f;
	float m_OuterAngle = 0.524f;

	// shadow mapping
	bool m_ShadowsEnabled = false;
	ShadowMap* m_ShadowMap = nullptr;
	ShadowCubemap* m_ShadowCubeMap = nullptr;
	XMFLOAT2 m_ShadowBiasCoeffs = { 0.0f, 0.0f };

	XMMATRIX m_ViewMatrix, m_OrthoMatrix, m_PerspectiveMatrix;
};