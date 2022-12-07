#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;


class Material
{
public:
	Material() = default;
	~Material() = default;

	void SettingsGUI();

	// getters and setters
	inline const XMFLOAT3 GetAlbedo() const { return m_Albedo; }

	inline bool UseAlbedoMap() const { return m_UseAlbedoMap; }
	inline void SetAlbedoMap(ID3D11ShaderResourceView* map) { m_AlbedoMap = map; }
	inline ID3D11ShaderResourceView* GetAlbedoMap() const { return m_AlbedoMap; }

	inline float GetMetalness() const { return m_Metalness; }

	inline bool UseRoughnessMap() const { return m_UseRoughnessMap; }
	inline void SetRoughnessMap(ID3D11ShaderResourceView* map) { m_RoughnessMap = map; }
	inline ID3D11ShaderResourceView* GetRoughnessMap() const { return m_RoughnessMap; }

	inline float GetRoughness() const { return m_Roughness; }

	inline bool UseNormalMap() const { return m_UseNormalMap; }
	inline void SetNormalMap(ID3D11ShaderResourceView* map) { m_NormalMap = map; }
	inline ID3D11ShaderResourceView* GetNormalMap() const { return m_NormalMap; }


private:
	XMFLOAT3 m_Albedo{ 1.0f, 1.0f, 1.0f };
	ID3D11ShaderResourceView* m_AlbedoMap = nullptr;
	bool m_UseAlbedoMap = false;

	float m_Roughness = 0.5f;
	ID3D11ShaderResourceView* m_RoughnessMap = nullptr;
	bool m_UseRoughnessMap = false;

	ID3D11ShaderResourceView* m_NormalMap = nullptr;
	bool m_UseNormalMap = false;

	float m_Metalness = 0.0f;
};
