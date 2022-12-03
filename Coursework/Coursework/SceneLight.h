#pragma once

#include "DXF.h"
#include <DirectXMath.h>

using namespace DirectX;


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

	SceneLight();
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

	inline float GetInnerAngle() const { return m_InnerAngle; }
	inline float GetOuterAngle() const { return m_OuterAngle; }
	inline void SetInnerAngle(float a) { m_InnerAngle = a; }
	inline void SetOuterAngle(float a) { m_OuterAngle = a; }

	// light matrices
	inline const XMMATRIX& GetViewMatrix() const { return m_ViewMatrix; }
	inline const XMMATRIX& GetOrthoMatrix() const { return m_OrthoMatrix; }
	void GenerateViewMatrix();
	void GenerateOrthoMatrix(float screenWidth, float screenHeight, float nearPlane, float farPlane);
	
	// shadow map
	void CreateShadowMap(ID3D11Device* device);
	inline ShadowMap* GetShadowMap() const { return m_ShadowMap; }


private:
	void CalculateDirectionFromEulerAngles();

private:
	bool m_Enabled = false;

	LightType m_Type = LightType::Directional;

	XMFLOAT3 m_Colour{ 1.0f, 1.0f, 1.0f };
	float m_Intensity = 1.0f;

	XMFLOAT3 m_Position{ 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_Direction{ 0.0f, 0.0f, 1.0f };

	// for directional lights only
	float m_Yaw = 0.0f, m_Pitch = 0.0f;

	// for spot lights only
	float m_InnerAngle = 0.175f;
	float m_OuterAngle = 0.524f;

	// shadow mapping
	ShadowMap* m_ShadowMap = nullptr;
	XMMATRIX m_ViewMatrix, m_OrthoMatrix;
};