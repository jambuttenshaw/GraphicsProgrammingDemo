#include "SceneLight.h"

#include "imGUI/imgui.h"


void SceneLight::SettingsGUI()
{
	ImGui::Checkbox("Enabled", &m_Enabled);
	if (!m_Enabled) return;

	ImGui::RadioButton("Directional", (int*)(&m_Type), (int)LightType::Directional); ImGui::SameLine();
	ImGui::RadioButton("Point", (int*)(&m_Type), (int)LightType::Point); ImGui::SameLine();
	ImGui::RadioButton("Spot", (int*)(&m_Type), (int)LightType::Spot);

	ImGui::ColorEdit3("Colour", &m_Colour.x);

	ImGui::DragFloat("Intensity", &m_Intensity, 0.05f);

	ImGui::Separator();

	if (m_Type != LightType::Directional)
	{
		ImGui::DragFloat3("Position", &m_Position.x, 0.1f);
	}

	if (m_Type != LightType::Point)
	{
		// store direction in position
		bool recalculateDir = false;
		recalculateDir |= ImGui::SliderAngle("Yaw", &m_Yaw, -180.0f, 180.0f);
		recalculateDir |= ImGui::SliderAngle("Pitch", &m_Pitch, -90.0f, 90.0f);
		if (recalculateDir) CalculateDirectionFromEulerAngles();
		ImGui::Text("Direction: %0.1f, %0.1f, %0.1f", m_Direction.x, m_Direction.y, m_Direction.z);
	}

	if (m_Type == LightType::Spot)
	{
		// additional spotlight settings
		ImGui::SliderAngle("Inner Angle", &m_InnerAngle, 0.0f, XMConvertToDegrees(m_OuterAngle));
		ImGui::SliderAngle("Outer Angle", &m_OuterAngle, XMConvertToDegrees(m_InnerAngle) + 1.0f, 90.0f);
	}
}

XMFLOAT3 SceneLight::GetIrradiance() const
{
	float i = m_Intensity * XM_PI;
	return XMFLOAT3{ m_Colour.x * i, m_Colour.y * i, m_Colour.z * i };
}

void SceneLight::CalculateDirectionFromEulerAngles()
{
	// calculate forward vector
	XMVECTOR v = XMVectorSet(
		sinf(m_Yaw) * cosf(m_Pitch),
		sinf(m_Pitch),
		cosf(m_Yaw) * cosf(m_Pitch),
		0.0f
	);

	// make sure it is normalized
	v = XMVector3Normalize(v);

	m_Direction.x = XMVectorGetX(v);
	m_Direction.y = XMVectorGetY(v);
	m_Direction.z = XMVectorGetZ(v);
}
