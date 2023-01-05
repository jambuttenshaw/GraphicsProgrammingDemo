#pragma once


#include <DirectXMath.h>
#include "imGUI/imgui.h"

using namespace DirectX;

class Transform
{
public:
	Transform() = default;

	inline void SetTranslation(XMFLOAT3 t) { m_Translation = t; }
	inline XMFLOAT3 GetTranslation() const { return m_Translation; }

	inline void SetPitch(float p) { m_Rotation.x = p; }
	inline float GetPitch() const { return m_Rotation.x; }
	inline void SetYaw(float y) { m_Rotation.y = y; }
	inline float GetYaw() const { return m_Rotation.y; }
	inline void SetRoll(float r) { m_Rotation.z = r; }
	inline float GetRoll() const { return m_Rotation.z; }

	void SetScale(float s) { m_Scale = { s, s, s }; }
	void SetScale(XMFLOAT3 s) { m_Scale = s; }
	XMFLOAT3 GetScale() const { return m_Scale; }

	XMMATRIX GetMatrix() const 
	{
		XMMATRIX m = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
		m *= XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
		m *= XMMatrixTranslation(m_Translation.x, m_Translation.y, m_Translation.z);
		return m;
	}

	void SettingsGUI()
	{
		ImGui::DragFloat3("Position", &m_Translation.x, 0.01f);
		ImGui::Text("Rotation");
		ImGui::SliderAngle("Pitch", &m_Rotation.x); 
		ImGui::SliderAngle("Yaw", &m_Rotation.y); 
		ImGui::SliderAngle("Roll", &m_Rotation.z);
		ImGui::DragFloat3("Scale", &m_Scale.x, 0.01f);
	}


private:
	XMFLOAT3 m_Translation { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_Rotation { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_Scale { 1.0f, 1.0f, 1.0f };
};
