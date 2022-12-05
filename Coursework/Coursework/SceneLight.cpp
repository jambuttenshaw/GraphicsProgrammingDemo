#include "SceneLight.h"

#include "imGUI/imgui.h"


SceneLight::SceneLight(ID3D11Device* device)
	: m_Device(device)
{
	m_ViewMatrix = XMMatrixIdentity();
	m_OrthoMatrix = XMMatrixIdentity();
	m_PerspectiveMatrix = XMMatrixIdentity();

	GenerateOrthoMatrix(100, 100, 0.1f, 100.0f);
	GeneratePerspectiveMatrix(0.1f);
}

SceneLight::~SceneLight()
{
	if (m_ShadowMap) delete m_ShadowMap;
}

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
		if (ImGui::DragFloat("Range", &m_Range, 0.05f))
			GeneratePerspectiveMatrix(0.1f);
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

	ImGui::Separator();

	bool s = m_ShadowsEnabled;
	if (ImGui::Checkbox("Enable Shadows", &s))
	{
		s ? EnableShadows() : DisableShadows();
	}
	if (m_ShadowsEnabled)
	{
		ImGui::DragFloat("Shadow Bias", &m_ShadowBias, 0.0001f, 0.0f, 0.0f, "%.4f");
	}
}

XMFLOAT3 SceneLight::GetIrradiance() const
{
	float i = m_Intensity;
	return XMFLOAT3{ m_Colour.x * i, m_Colour.y * i, m_Colour.z * i };
}

const XMMATRIX& SceneLight::GetProjectionMatrix() const
{
	switch (m_Type)
	{
	case SceneLight::LightType::Directional:	return m_OrthoMatrix; break;
	case SceneLight::LightType::Point:			return XMMatrixIdentity(); break;
	case SceneLight::LightType::Spot:			return m_PerspectiveMatrix; break;
	default:									return XMMatrixIdentity(); break;
	}
}

void SceneLight::GenerateViewMatrix()
{
	// default up vector
	XMVECTOR up;
	if ((m_Direction.y == 0 && m_Direction.z == 0))
		up = XMVectorSet(0.0f, 0.0f, copysignf(1.0f, m_Direction.y), 1.0);
	else
		up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);

	XMVECTOR pos = XMVectorSet(m_Position.x, m_Position.y, m_Position.z, 1.0f);

	XMVECTOR dir = XMVectorSet(m_Direction.x, m_Direction.y, m_Direction.z, 1.0f);
	XMVECTOR right = XMVector3Cross(dir, up);
	up = XMVector3Cross(right, dir); // make dir, right and up orthonormal

	// Create the view matrix from the three vectors.
	m_ViewMatrix = XMMatrixLookAtLH(pos, pos + dir, up);
}

void SceneLight::GenerateOrthoMatrix(float screenWidth, float screenHeight, float nearPlane, float farPlane)
{
	m_OrthoMatrix = XMMatrixOrthographicLH(screenWidth, screenHeight, nearPlane, farPlane);
}

void SceneLight::GeneratePerspectiveMatrix(float nearPlane)
{
	m_PerspectiveMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, nearPlane, 2.0f * m_Range);
}

void SceneLight::EnableShadows()
{
	if (m_ShadowsEnabled) return;
	m_ShadowsEnabled = true;

	if (!m_ShadowMap) CreateShadowMap();
}

void SceneLight::DisableShadows()
{
	if (!m_ShadowsEnabled) return;
	m_ShadowsEnabled = false;
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

void SceneLight::CreateShadowMap()
{
	m_ShadowMap = new ShadowMap(m_Device, 1024, 1024);
}
