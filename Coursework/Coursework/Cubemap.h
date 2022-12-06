#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <cassert>
#include <vector>

using namespace DirectX;

/*
Convention for order of faces in memory:
- +X -> Right		-> 0
- -X -> Left		-> 1
- +Y -> Top			-> 2
- -Y -> Bottom		-> 3
- +Z -> Front		-> 4
- -Z -> Back		-> 5
*/



class Cubemap
{
public:
	Cubemap(ID3D11Device* device, unsigned int size, unsigned int mipLevels = 1);
	Cubemap(ID3D11Device* device, const char* right, const char* left, const char* top, const char* bottom, const char* front, const char* back);
	Cubemap(ID3D11Device* device, const char* faces[6]);
	~Cubemap();

	inline ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }
	ID3D11UnorderedAccessView* GetUAV(int face, int mip = 0) const;

public:
	static const XMFLOAT3& GetFaceNormal(int face) { return s_FaceNormals[face]; }
	static const XMFLOAT3& GetFaceTangent(int face) { return s_FaceTangents[face]; }
	static const XMFLOAT3& GetFaceBitangent(int face) { return s_FaceBitangents[face]; }

private:

	void Load(ID3D11Device* device, const char* faces[6]);

	void CreateUAVs(ID3D11Device* device, unsigned int mipSlice = 0);

private:
	bool m_ReadOnly = true;
	bool m_HasMips = false;

	ID3D11Texture2D* m_CubemapTexture = nullptr;
	ID3D11ShaderResourceView* m_SRV = nullptr;

	// for rendering to the cubemap
	std::vector<ID3D11UnorderedAccessView*> m_UAVs;

private:
	static XMFLOAT3 s_FaceNormals[6];
	static XMFLOAT3 s_FaceTangents[6];
	static XMFLOAT3 s_FaceBitangents[6];
};