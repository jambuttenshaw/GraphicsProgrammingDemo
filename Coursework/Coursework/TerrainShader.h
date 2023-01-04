#pragma once

#include "DXF.h"
using namespace DirectX;

#include "ShaderUtility.h"

class SceneLight;
class Material;
class GlobalLighting;
class TerrainMesh;


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
		float heightmapDims;
		float terrainSize;

		int heightmapIndex;
		float uvScale;
		float flatThreshold;
		float cliffThreshold;

		float steepnessSmoothing;
		float heightSmoothing;
		float shoreThreshold;
		float snowHeightThreshold;

		XMFLOAT2 minMaxSnowSteepness;
	};
	struct TessellationBufferType
	{
		XMFLOAT2 minMaxDistance;
		XMFLOAT2 minMaxHeightDeviation;
		XMFLOAT2 minMaxLOD;
		float distanceLODBlending;
		float padding;
		XMFLOAT3 cameraPos;
		float size;
	};

public:
	TerrainShader(ID3D11Device* device, GlobalLighting* globalLighing);
	~TerrainShader();

	void SetShaderParameters(ID3D11DeviceContext* deviceContext,
								const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection,
								TerrainMesh* terrainMesh,
								size_t lightCount, SceneLight** lights, Camera* camera, const std::vector<Material*>& materials);
	void Render(ID3D11DeviceContext* deviceContext, unsigned int indexCount);

	void GUI();

	inline const XMFLOAT2& GetMinMaxDist() const { return m_MinMaxDistance; }
	inline const XMFLOAT2& GetMinMaxHeightDeviation() const { return m_MinMaxHeightDeviation; }
	inline float GetDistanceLODBlending() const { return m_DistanceLODBlending; }
	inline const XMFLOAT2& GetMinMaxLOD() const { return m_MinMaxLOD; }

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
	ID3D11SamplerState* m_PointSampler = nullptr;

	GlobalLighting* m_GlobalLighting = nullptr;

	// terrain properties
	float m_UVScale = 32.0f;
	float m_FlatThreshold = 0.406f;
	float m_CliffThreshold = 0.674f;
	float m_ShoreThreshold = 0.5f;
	float m_SnowHeightThreshold = 2.0f;
	XMFLOAT2 m_MinMaxSnowSteepness = { 0.27f, 0.8f };
	float m_SteepnessSmoothing = 0.108f;
	float m_HeightSmoothing = 1.0f;

	// tessellation params
	XMFLOAT2 m_MinMaxDistance{ 5.0f, 25.0f };
	XMFLOAT2 m_MinMaxHeightDeviation{ 0.5f, 2.0f };
	XMFLOAT2 m_MinMaxLOD{ 1.0f, 8.0f };
	float m_DistanceLODBlending = 0.75f;
};

