#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class TerrainMesh;


class UnlitTerrainShader
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
	UnlitTerrainShader(ID3D11Device* device);
	~UnlitTerrainShader();

	void SetShaderParameters(ID3D11DeviceContext* deviceContext,
		const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
		TerrainMesh* terrainMesh, const XMFLOAT3& tessPOV, XMFLOAT2 minMaxDist, XMFLOAT2 minMaxLOD, XMFLOAT2 minMaxHeightDeviation, float distanceLODBlending);
	void Render(ID3D11DeviceContext* deviceContext, unsigned int indexCount);

private:
	void InitShader();

	void LoadVS(const wchar_t* vs);
	void LoadHS(const wchar_t* hs);
	void LoadDS(const wchar_t* ds);

	void CreateBuffer(UINT byteWidth, ID3D11Buffer** ppBuffer);

private:
	ID3D11Device* m_Device = nullptr;

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11HullShader* m_HullShader = nullptr;
	ID3D11DomainShader* m_DomainShader = nullptr;

	ID3D11InputLayout* m_InputLayout = nullptr;

	ID3D11Buffer* m_VSMatrixBuffer = nullptr;			// matrices to be sent to vertex shader
	ID3D11Buffer* m_TessellationBuffer = nullptr;		// tessellation parameters
	ID3D11Buffer* m_DSMatrixBuffer = nullptr;			// matrices to be sent to domain shader

	ID3D11SamplerState* m_HeightmapSampleState = nullptr;
};

