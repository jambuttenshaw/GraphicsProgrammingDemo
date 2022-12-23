#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

#include <string>


class Material
{
public:
	Material() = default;
	~Material() = default;

	void SettingsGUI();

	void LoadPBRFromDir(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& dir);

	// getters and setters
	inline void SetAlbedo(const XMFLOAT3& albedo) { m_Albedo = albedo; }
	inline const XMFLOAT3 GetAlbedo() const { return m_Albedo; }

	inline bool UseAlbedoMap() const { return m_UseAlbedoMap && m_AlbedoMap; }
	inline void SetAlbedoMap(ID3D11ShaderResourceView* map) { m_AlbedoMap = map; }
	inline ID3D11ShaderResourceView* GetAlbedoMap() const { return m_AlbedoMap; }

	inline void SetMetalness(float m) { m_Metalness = m; }
	inline float GetMetalness() const { return m_Metalness; }

	inline bool UseRoughnessMap() const { return m_UseRoughnessMap && m_RoughnessMap; }
	inline void SetRoughnessMap(ID3D11ShaderResourceView* map) { m_RoughnessMap = map; }
	inline ID3D11ShaderResourceView* GetRoughnessMap() const { return m_RoughnessMap; }

	inline void SetRoughness(float r) { m_Roughness = r; }
	inline float GetRoughness() const { return m_Roughness; }

	inline bool UseNormalMap() const { return m_UseNormalMap && m_NormalMap; }
	inline void SetNormalMap(ID3D11ShaderResourceView* map) { m_NormalMap = map; }
	inline ID3D11ShaderResourceView* GetNormalMap() const { return m_NormalMap; }

private:

	bool DoesFileExist(const wchar_t* filename) const;
	// loads texture at filename and stores a pointer to it in srv
	void LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename, ID3D11ShaderResourceView** srv);

private:
	XMFLOAT3 m_Albedo{ 1.0f, 1.0f, 1.0f };
	ID3D11ShaderResourceView* m_AlbedoMap = nullptr;
	bool m_UseAlbedoMap = true;

	float m_Roughness = 0.5f;
	ID3D11ShaderResourceView* m_RoughnessMap = nullptr;
	bool m_UseRoughnessMap = true;

	ID3D11ShaderResourceView* m_NormalMap = nullptr;
	bool m_UseNormalMap = true;

	float m_Metalness = 0.0f;
};
