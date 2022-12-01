#pragma once

#include <DirectXMath.h>
using namespace DirectX;


class Material
{
public:
	Material() = default;
	~Material() = default;

	void SettingsGUI();

	// getters and setters
	inline const XMFLOAT4 GetAlbedo() const { return m_Albedo; }
	inline float GetMetalness() const { return m_Metalness; }
	inline float GetRoughness() const { return m_Roughness; }

private:
	XMFLOAT4 m_Albedo{ 1.0f, 1.0f, 1.0f, 1.0f };
	float m_Metalness = 0.0f;
	float m_Roughness = 0.5f;
};
