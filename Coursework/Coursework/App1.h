// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include <d3d11.h>

#include "RenderTarget.h"
#include "SceneLight.h"
#include "Material.h"

#include "TerrainMesh.h"

#include "Transform.h"

#include <array>

class IHeightmapFilter;

class LightShader;
class WaterShader;
class TerrainShader;
class UnlitShader;


class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void gui();

	// passes
	void worldPass();
	void waterPass();

	void renderLightDebugSpheres();

	void terrainSettingsMenu();
	bool addTerrainFilterMenu();

	void applyFilterStack();

	IHeightmapFilter* createFilterFromIndex(int index);

	void saveSettings(const std::string& file);
	void loadSettings(const std::string& file);

private:
	float m_Time = 0.0f;

	// Shaders
	LightShader* m_LightShader;
	TerrainShader* m_TerrainShader;
	WaterShader* m_WaterShader;
	UnlitShader* m_UnlitShader;

	RenderTarget* m_RenderTarget;

	Transform m_TerrainTransform;
	TerrainMesh* m_Terrain;
	
	CubeMesh* m_Cube;
	SphereMesh* m_Sphere;

	std::array<SceneLight, 4> m_Lights;

	bool m_LightDebugSpheres = true;
	SphereMesh* m_LightDebugSphereMesh;

	Material mat1, mat2;

	std::vector<IHeightmapFilter*> m_HeightmapFilters;
	int m_SelectedHeightmapFilter = -1;
	std::array<const char*, 4> m_AllFilterNames = {
		"Simple Noise", "Ridge Noise", "Warped Simple Noise", "Terrain Noise"
	};
	bool m_TerrainSettingsOpen = false;

	char m_SaveFilePath[128];
	bool m_LoadOnOpen = true;
	bool m_SaveOnExit = false;
};

#endif