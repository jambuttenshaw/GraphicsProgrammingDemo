#include "SceneLight.h"

#include "imGUI/imgui.h"

#include "ShadowCubemap.h"


SceneLight::SceneLight(ID3D11Device* device)
	: m_Device(device)
{
	m_ViewMatrix = XMMatrixIdentity();
	m_OrthoMatrix = XMMatrixIdentity();
	m_PerspectiveMatrix = XMMatrixIdentity();

	GenerateOrthoMatrix();
	GeneratePerspectiveMatrix();
}

SceneLight::~SceneLight()
{
	if (m_ShadowMap) delete m_ShadowMap;
	if (m_ShadowCubeMap) delete m_ShadowCubeMap;
}

void SceneLight::SettingsGUI()
{
	ImGui::Checkbox("Enabled", &m_Enabled);
	if (!m_Enabled) return;

	LightType t = m_Type;
	ImGui::RadioButton("Directional", (int*)(&t), (int)LightType::Directional); ImGui::SameLine();
	ImGui::RadioButton("Point", (int*)(&t), (int)LightType::Point); ImGui::SameLine();
	ImGui::RadioButton("Spot", (int*)(&t), (int)LightType::Spot);
	if (t != m_Type)
	{
		m_Type = t;
		// check to see if a new shadow map needs switched
		if (m_ShadowsEnabled) CreateShadowMap();
	}

	ImGui::ColorEdit3("Colour", &m_Colour.x);

	ImGui::DragFloat("Intensity", &m_Intensity, 0.05f);

	ImGui::Separator();

	if (m_Type != LightType::Directional)
	{
		ImGui::DragFloat3("Position", &m_Position.x, 0.1f);
		ImGui::DragFloat("Range", &m_Range, 0.05f);
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
		bool generateProjectionMatrix = false;

		if (m_Type == LightType::Directional)
		{
			ImGui::DragFloat3("Position", &m_Position.x, 0.1f);

			generateProjectionMatrix |= ImGui::DragFloat("Frustum Width", &m_FrustumWidth, 0.01f);
			generateProjectionMatrix |= ImGui::DragFloat("Frustum Height", &m_FrustumHeight, 0.01f);
		}
		generateProjectionMatrix |= ImGui::DragFloat("Near Plane", &m_NearPlane, 0.01f);
		generateProjectionMatrix |= ImGui::DragFloat("Far Plane", &m_FarPlane, 0.01f);
		if (generateProjectionMatrix) GenerateProjectionMatrix();
		

		if (m_Type == LightType::Point)
		{
			ImGui::SliderFloat("Shadow Bias Amount", &m_ShadowBiasCoeffs.x, 0.0f, 0.05f);
			ImGui::SliderFloat("Shadow Bias Attenuation", &m_ShadowBiasCoeffs.y, 0.0f, 5.0f);
		}
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
	case SceneLight::LightType::Directional:	return m_OrthoMatrix;
	case SceneLight::LightType::Point:			return m_PerspectiveMatrix;
	case SceneLight::LightType::Spot:			return m_PerspectiveMatrix;
	default:									return XMMatrixIdentity();
	}
}

void SceneLight::GenerateProjectionMatrix()
{
	switch(m_Type)
	{
	case SceneLight::LightType::Directional:	GenerateOrthoMatrix();
	case SceneLight::LightType::Point:			GeneratePerspectiveMatrix();
	case SceneLight::LightType::Spot:			GeneratePerspectiveMatrix();
	default:									break;
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

void SceneLight::GenerateOrthoMatrix(float frustumWidth, float frustumHeight, float nearPlane, float farPlane)
{
	m_FrustumWidth = frustumWidth;
	m_FrustumHeight = frustumHeight;
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
	GenerateOrthoMatrix();
}

void SceneLight::GenerateOrthoMatrix()
{
	m_OrthoMatrix = XMMatrixOrthographicLH(m_FrustumWidth, m_FrustumHeight, m_NearPlane, m_FarPlane);

}

void SceneLight::GeneratePerspectiveMatrix(float nearPlane, float farPlane)
{
	m_NearPlane = nearPlane;
	m_FarPlane = farPlane;
	GeneratePerspectiveMatrix();
}

void SceneLight::GeneratePerspectiveMatrix()
{
	m_PerspectiveMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, m_NearPlane, m_FarPlane);
}

void SceneLight::GetPointLightViewMatrices(XMMATRIX* matArray)
{
	static const XMVECTOR px = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	static const XMVECTOR py = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	static const XMVECTOR pz = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	const XMVECTOR pos = XMVectorSet(m_Position.x, m_Position.y, m_Position.z, 1.0f);

	matArray[0] = XMMatrixLookAtLH(pos, pos + px, py);
	matArray[1] = XMMatrixLookAtLH(pos, pos - px, py);
	matArray[2] = XMMatrixLookAtLH(pos, pos + py, pz);
	matArray[3] = XMMatrixLookAtLH(pos, pos - py, pz);
	matArray[4] = XMMatrixLookAtLH(pos, pos + pz, py);
	matArray[5] = XMMatrixLookAtLH(pos, pos - pz, py);
}

void SceneLight::EnableShadows()
{
	if (m_ShadowsEnabled) return;
	m_ShadowsEnabled = true;

	CreateShadowMap();
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
	if (m_Type == LightType::Point)
	{
		if (!m_ShadowCubeMap)
			m_ShadowCubeMap = new ShadowCubemap(m_Device, 1024);
	}
	else
	{
		if (!m_ShadowMap)
			m_ShadowMap = new ShadowMap(m_Device, 1024, 1024);
	}
}
