#include "Cubemap.h"

#include "stb_image.h"


Cubemap::Cubemap(ID3D11Device* device, const char* faces[6])
{
	const char* faceData[6];
	if (!LoadCubeFaces(&faceData[0]))
	{
		// error
	}
}

Cubemap::~Cubemap()
{

}

bool Cubemap::LoadCubeFaces(const char** facePaths, const unsigned char** loadedData)
{
	for (size_t i = 0; i < 6; i++)
	{
		int width, height, channels;
		loadedData[i] = stbi_load(facePaths[i], &width, &height, &channels, 4);

		if (!loadedData[i]) return false;

		// do additional checks on width height etc
	}

	return true;
}
