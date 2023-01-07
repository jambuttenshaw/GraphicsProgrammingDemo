#pragma once

#include <d3d11.h>
#include <DirectXMath.h>


class TerrainMesh
{
	struct VertexType
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 UV;
		DirectX::XMFLOAT3 Normal;
	};

public:
	TerrainMesh(ID3D11Device* device, float size);
	~TerrainMesh();

	void SendData(ID3D11DeviceContext* deviceContext);

	// reconstruct the mesh with an edge length of size
	void BuildMesh(ID3D11Device* device, float size);

	// getters
	inline ID3D11UnorderedAccessView* GetHeightmapUAV() const { return m_HeightmapUAV; }
	inline ID3D11ShaderResourceView*  GetHeightmapSRV() const { return m_HeightmapSRV; }
	inline unsigned int GetHeightmapResolution() const { return m_HeightmapResolution; }

	inline unsigned long GetVertexCount() const { return m_VertexCount; }
	inline unsigned long GetIndexCount() const { return m_IndexCount; }

	inline float GetSize() const { return m_Size; }

	// preprocessing the heightmap
	void PreprocessHeightmap(ID3D11DeviceContext* deviceContext);
	inline ID3D11ShaderResourceView* GetPreprocessSRV() const { return m_PreprocessSRV; }

private:
	void CreateHeightmapTexture(ID3D11Device* device);
	void CreatePreprocessTexture(ID3D11Device* device);

private:
	// hard-coded heightmap resolution, but could be changed to any multiple of 16
	const unsigned int m_HeightmapResolution = 1024;

	const unsigned int m_Resolution = m_HeightmapResolution / 16; // number of cells along one axis of the terrain mesh
	float m_Size = 100.0f;			// length in units of one edge of the terrain mesh

	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11Buffer* m_IndexBuffer = nullptr;
	unsigned long m_VertexCount = 0, m_IndexCount = 0;

	// heightmap texture
	ID3D11UnorderedAccessView* m_HeightmapUAV = nullptr;
	ID3D11ShaderResourceView*  m_HeightmapSRV = nullptr;

	// preprocessed heightmap texture
	ID3D11UnorderedAccessView* m_PreprocessUAV = nullptr;
	ID3D11ShaderResourceView*  m_PreprocessSRV = nullptr;

	// CS for preprocessing the heightmap
	ID3D11ComputeShader* m_PreprocessCS;
};
