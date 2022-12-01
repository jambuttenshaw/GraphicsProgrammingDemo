#pragma once

#include <d3d11.h>

/*
Follow OpenGL convention for order of faces in memory:
- Right		-> 0
- Left		-> 1
- Top		-> 2
- Bottom	-> 3
- Back		-> 4
- Front		-> 5
*/



class Cubemap
{
public:
	Cubemap(ID3D11Device* device, const char* faces[6]);
	~Cubemap();

	ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }

private:
	bool LoadCubeFaces(const char** facePaths, const unsigned char** loadedData);

private:
	ID3D11Texture2D* m_CubemapTexture = nullptr;
	ID3D11ShaderResourceView* m_SRV = nullptr;
};