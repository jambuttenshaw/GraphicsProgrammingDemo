#pragma once

#include "DXF.h"
#include <nlohmann/json.hpp>

using namespace DirectX;

#include "ShaderUtility.h"

class SceneLight;
class Material;
class GlobalLighting;


class TerrainShader
{
private:
	struct VSMatrixBufferType
	{
		XMMATRIX world;
	};
	struct DSMatrixBufferType
	{
		XMMATRIX view;
		XMMATRIX projection;
	};
	struct TerrainBufferType
	{
		int heightmapIndex;
		float flatThreshold;
		float cliffThreshold;
		float steepnessSmoothing;
	};
	struct TessellationBufferType
	{
		XMFLOAT2 minMaxDistance;
		XMFLOAT2 minMaxLOD;
		XMFLOAT3 cameraPos;
		float padding;
	};

public:
	TerrainShader(ID3D11Device* device, GlobalLighting* globalLighing);
	~TerrainShader();

	void SetShaderParameters(ID3D11DeviceContext* deviceContext,
								const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection,
								ID3D11ShaderResourceView* heightmap,
								size_t lightCount, SceneLight** lights, Camera* camera, Material* mat);
	void Render(ID3D11DeviceContext* deviceContext, unsigned int indexCount);

	void GUI();

	inline const XMFLOAT2& GetMinMaxDist() const { return m_MinMaxDistance; }
	inline const XMFLOAT2& GetMinMaxLOD() const { return m_MinMaxLOD; }

	nlohmann::json Serialize() const;
	void LoadFromJson(const nlohmann::json& data);

private:
	void InitShader();
	
	void LoadVS(const wchar_t* vs);
	void LoadHS(const wchar_t* hs);
	void LoadDS(const wchar_t* ds);
	void LoadPS(const wchar_t* ps);

private:
	ID3D11Device* m_Device = nullptr;

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11HullShader* m_HullShader = nullptr;
	ID3D11DomainShader* m_DomainShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

	ID3D11InputLayout* m_InputLayout = nullptr;

	ID3D11Buffer* m_VSMatrixBuffer = nullptr;			// matrices to be sent to vertex shader
	ID3D11Buffer* m_TessellationBuffer = nullptr;		// tessellation parameters
	ID3D11Buffer* m_DSMatrixBuffer = nullptr;			// matrices to be sent to domain shader
	ID3D11Buffer* m_DSLightBuffer = nullptr;			// camera and light matrices
	ID3D11Buffer* m_PSLightBuffer = nullptr;				// lighting data
	ID3D11Buffer* m_MaterialBuffer = nullptr;			// material data
	ID3D11Buffer* m_TerrainBuffer = nullptr;			// terrain data

	ID3D11SamplerState* m_HeightmapSampleState = nullptr;
	ID3D11SamplerState* m_MaterialSampler = nullptr;
	ID3D11SamplerState* m_ShadowSampler = nullptr;

	GlobalLighting* m_GlobalLighting = nullptr;

	// terrain properties
	float m_FlatThreshold = 0.5f;
	float m_CliffThreshold = 0.8f;
	float m_SteepnessSmoothing = 0.1f;

	// tessellation params
	XMFLOAT2 m_MinMaxDistance{ 10.0f, 40.0f };
	XMFLOAT2 m_MinMaxLOD{ 1.0f, 10.0f };
};

