#pragma once

#include <d3d11.h>

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
	Cubemap(ID3D11Device* device, const char* right, const char* left, const char* top, const char* bottom, const char* front, const char* back);
	Cubemap(ID3D11Device* device, const char* faces[6]);
	~Cubemap();

	ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }

private:

	void Load(ID3D11Device* device, const char* faces[6]);

private:
	ID3D11Texture2D* m_CubemapTexture = nullptr;
	ID3D11ShaderResourceView* m_SRV = nullptr;
};