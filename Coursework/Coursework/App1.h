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
#include "GameObject.h"

#include <array>

class IHeightmapFilter;

class LightShader;
class TerrainShader;
class TextureShader;

class UnlitShader;
class UnlitTerrainShader;

class WaterShader;
class MeasureLuminanceShader;
class BloomShader;
class FinalPassShader;

class GlobalLighting;
class Cubemap;
class Skybox;


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
	void depthPass(SceneLight* light);
	void worldPass();
	
	void waterPass();

	void renderLightDebugSpheres();

	void terrainSettingsMenu();
	bool addTerrainFilterMenu();

	void applyFilterStack();

	IHeightmapFilter* createFilterFromIndex(int index);

	void saveSettings(const std::string& file);
	void loadSettings(const std::string& file);

	Material* GetMaterialByName(const std::string& name);

private:
	float m_Time = 0.0f;

	// Shaders
	LightShader* m_LightShader = nullptr;
	TerrainShader* m_TerrainShader = nullptr;
	TextureShader* m_TextureShader = nullptr;
	
	UnlitShader* m_UnlitShader = nullptr;
	UnlitTerrainShader* m_UnlitTerrainShader = nullptr;

	WaterShader* m_WaterShader = nullptr;
	MeasureLuminanceShader* m_MeasureLuminenceShader = nullptr;
	BloomShader* m_BloomShader = nullptr;
	FinalPassShader* m_FinalPassShader = nullptr;

	// render targets
	RenderTarget* m_SceneRenderTexture = nullptr;
	RenderTarget* m_WaterRenderTexture = nullptr;
	//const XMFLOAT4 m_ClearColour{ 1.0f, 0.0f, 1.0f, 1.0f };
	const XMFLOAT4 m_ClearColour{ 0.0f, 0.0f, 0.0f, 1.0f };

	// environment
	GlobalLighting* m_GlobalLighting = nullptr;
	Cubemap* m_EnvironmentMap = nullptr;
	Skybox* m_Skybox = nullptr;
	int m_SelectedSkybox = 0;
	bool m_DrawSkybox = true;

	// meshes
	CubeMesh* m_CubeMesh = nullptr;
	SphereMesh* m_SphereMesh = nullptr;
	PlaneMesh* m_PlaneMesh = nullptr;
	TerrainMesh* m_TerrainMesh = nullptr;

	// game objects
	std::vector<GameObject> m_GameObjects;

	std::array<SceneLight*, 4> m_Lights;

	bool m_LightDebugSpheres = true;

	D3D11_RASTERIZER_DESC m_ShadowRasterDesc;
	ID3D11RasterizerState* m_ShadowRasterizerState = nullptr;

	bool m_ShowShadowMap = false;
	int m_SelectedShadowMap = 0;
	int m_SelectedShadowCubemapFace = 0;
	OrthoMesh* m_ShadowMapMesh = nullptr;

	std::array<Material, 6> m_Materials;

	// post processing
	bool m_EnablePostProcessing = false;

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