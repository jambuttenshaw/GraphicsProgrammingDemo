#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

#include "Cubemap.h"


class GlobalLighting
{
public:
	GlobalLighting(ID3D11Device* device);
	~GlobalLighting();

	void SettingsGUI();

	inline bool IsIBLEnabled() const { return m_EnableIBL; }
	inline void SetIBLEnabled(bool e) { m_EnableIBL = e; }

	inline const XMFLOAT4& GetGlobalAmbient() const { return m_GlobalAmbient; }
	inline void SetGlobaAmbient(const XMFLOAT4& c) { m_GlobalAmbient = c; }

	inline ID3D11ShaderResourceView* GetIrradianceMap() const { return m_IrradianceMap->GetSRV(); }
	inline ID3D11ShaderResourceView* GetPrefilterMap() const { return m_PrefilteredEnvironmentMap->GetSRV(); }
	inline ID3D11ShaderResourceView* GetBRDFIntegrationMap() const { return m_BRDFIntegrationMapSRV; }

	inline ID3D11SamplerState* GetCubemapSampler() const { return m_CubemapSampler; }
	inline ID3D11SamplerState* GetBRDFIntegrationSampler() const { return m_BRDFIntegrationSampler; }

	void SetAndProcessEnvironmentMap(ID3D11DeviceContext* deviceContext, Cubemap* environment);

private:
	void LoadShader(const wchar_t* cs, ID3D11ComputeShader** shader);
	void CreateBuffer(UINT byteWidth, ID3D11Buffer** ppBuffer);

	void CreateIrradianceMap(ID3D11DeviceContext* deviceContext);
	void CreateBRDFIntegrationMap(ID3D11DeviceContext* deviceContext);
	void CreatePrefilteredEnvironmentMap(ID3D11DeviceContext* deviceContext);

private:
	ID3D11Device* m_Device = nullptr;

	XMFLOAT4 m_GlobalAmbient{ 0.3f, 0.3f, 0.3f, 1.0f };
	
	bool m_EnableIBL = true;
	Cubemap* m_EnvironmentMap = nullptr;

	// pre-processed maps for IBL
	const unsigned int m_IrradianceMapResolutuion = 32;
	Cubemap* m_IrradianceMap = nullptr;

	const unsigned int m_BRDFIntegrationMapResolution = 512;
	ID3D11Texture2D* m_BRDFIntegrationMap = nullptr;
	ID3D11ShaderResourceView* m_BRDFIntegrationMapSRV = nullptr;
	ID3D11UnorderedAccessView* m_BRDFIntegrationMapUAV = nullptr;

	const unsigned int m_PEMResolution = 128;
	const unsigned int m_PEMRoughnessBins = 5;
	Cubemap* m_PrefilteredEnvironmentMap = nullptr;

	ID3D11SamplerState* m_CubemapSampler = nullptr;
	ID3D11SamplerState* m_BRDFIntegrationSampler = nullptr;


	// pre-processing compute shaders
	ID3D11ComputeShader* m_IrradianceMapShader = nullptr;
	ID3D11Buffer* m_IrradianceMapShaderBuffer = nullptr;
	struct IrradianceMapBufferType
	{
		XMFLOAT3 faceNormal; float padding0;
		XMFLOAT3 faceTangent; float padding1;
		XMFLOAT3 faceBitangent; float padding2;
	};

	ID3D11ComputeShader* m_BRDFIntegrationShader = nullptr;

	ID3D11ComputeShader* m_PEMShader = nullptr;
	ID3D11Buffer* m_PEMShaderBuffer = nullptr;
	struct PEMBufferType
	{
		XMFLOAT3 faceNormal;
		float roughness;
		XMFLOAT3 faceTangent;
		float padding0;
		XMFLOAT3 faceBitangent;
		float padding1;
	};
};