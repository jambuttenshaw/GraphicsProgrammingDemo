#pragma once

#include <DirectXMath.h>
using namespace DirectX;

class Cubemap;


class GlobalLighting
{
public:
	GlobalLighting() = default;
	~GlobalLighting() = default;

	void SettingsGUI();

	inline bool IsEnvironmentMapEnabled() const { return m_EnableEnvironmentMap; }
	inline void SetEnvironmentMapEnabled(bool e) { m_EnableEnvironmentMap = e; }

	inline const XMFLOAT4& GetGlobalAmbient() const { return m_GlobalAmbient; }
	inline void SetGlobaAmbient(const XMFLOAT4& c) { m_GlobalAmbient = c; }

	inline Cubemap* GetEnvironmentMap() const { return m_EnvironmentMap; }
	inline void SetEnvironmentMap(Cubemap* environment) { m_EnvironmentMap = environment; }

private:
	XMFLOAT4 m_GlobalAmbient{ 0.3f, 0.3f, 0.3f, 1.0f };
	
	bool m_EnableEnvironmentMap = true;
	Cubemap* m_EnvironmentMap = nullptr;
};