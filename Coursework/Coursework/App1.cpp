#include "App1.h"

#include <nlohmann/json.hpp>

#include "LightShader.h"
#include "TerrainShader.h"
#include "WaterShader.h"
#include "UnlitShader.h"
#include "TextureShader.h"
#include "ToneMappingShader.h"

#include "GlobalLighting.h"
#include "Cubemap.h"
#include "Skybox.h"

#include "HeightmapFilters.h"
#include "SerializationHelper.h"


App1::App1()
{
	m_Terrain = nullptr;
	m_TerrainShader = nullptr;

	strcpy_s(m_SaveFilePath, "res/settings/earth.json");
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Load textures
	textureMgr->loadTexture(L"grass", L"res/grass.png");
	textureMgr->loadTexture(L"dirt", L"res/dirt.png");
	textureMgr->loadTexture(L"oceanNormalMapA", L"res/wave_normals1.png");
	textureMgr->loadTexture(L"oceanNormalMapB", L"res/wave_normals2.png");

	textureMgr->loadTexture(L"asphalt_albedo", L"res/pbr/cracking_painted_asphalt_albedo.png");
	textureMgr->loadTexture(L"asphalt_roughness", L"res/pbr/cracking_painted_asphalt_roughness.png");
	textureMgr->loadTexture(L"asphalt_normal", L"res/pbr/cracking_painted_asphalt_normal.png");

	textureMgr->loadTexture(L"worn_metal_albedo", L"res/pbr/worn-shiny-metal-albedo.png");
	textureMgr->loadTexture(L"worn_metal_roughness", L"res/pbr/worn-shiny-metal-roughness.png");

	textureMgr->loadTexture(L"armor_albedo", L"res/pbr/armor-plating1_albedo.png");
	textureMgr->loadTexture(L"armor_roughness", L"res/pbr/armor-plating1_roughness.png");
	textureMgr->loadTexture(L"armor_normal", L"res/pbr/armor-plating1_normal.png");

	m_GlobalLighting = new GlobalLighting(renderer->getDevice());

	m_LightShader = new LightShader(renderer->getDevice(), hwnd, m_GlobalLighting);
	m_TerrainShader = new TerrainShader(renderer->getDevice());
	m_WaterShader = new WaterShader(renderer->getDevice(), textureMgr->getTexture(L"oceanNormalMapA"), textureMgr->getTexture(L"oceanNormalMapB"));
	m_UnlitShader = new UnlitShader(renderer->getDevice(), hwnd);
	m_TextureShader = new TextureShader(renderer->getDevice(), hwnd);
	m_ToneMappingShader = new ToneMappingShader(renderer->getDevice());

	m_SrcRenderTarget = new RenderTarget(renderer->getDevice(), screenWidth, screenHeight);
	m_DstRenderTarget = new RenderTarget(renderer->getDevice(), screenWidth, screenHeight);

	m_OutputMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);

	//m_EnvironmentMap = new Cubemap(renderer->getDevice(), 
	//	"res/skybox/right.png", "res/skybox/left.png", 
	//	"res/skybox/top.png", "res/skybox/bottom.png",
	//	"res/skybox/front.png", "res/skybox/back.png");
	m_EnvironmentMap = new Cubemap(renderer->getDevice(),
		"res/skybox2/px.png",	"res/skybox2/nx.png",
		"res/skybox2/py.png",	"res/skybox2/ny.png",
		"res/skybox2/pz.png",	"res/skybox2/nz.png");
	m_GlobalLighting->SetAndProcessEnvironmentMap(renderer->getDeviceContext(), m_EnvironmentMap);
	m_Skybox = new Skybox(renderer->getDevice(), m_EnvironmentMap);

	m_LightDebugSphereMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	m_Terrain = new TerrainMesh(renderer->getDevice());

	m_Cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	m_Sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	m_Plane = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 20);
	m_ShadowMapMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), 300, 300, (screenWidth / 2) - 150, (screenHeight / 2) - 150);

	camera->setPosition(4.0f, 1.0f, 0.0f);
	camera->setRotation(0.0f, 0.0f, 0.0f);

	// create lights
	for (auto& light : m_Lights)
	{
		light = new SceneLight(renderer->getDevice());
		light->SetPosition({ 0.0f, 0.0f, -10.0f });
	}

	// setup default light settings
	/*
	SceneLight& light = *(m_Lights[0]);
	light.SetEnbled(true);
	light.SetPosition({ 4.0f, 8.0f, 5.0f });
	light.SetType(SceneLight::LightType::Spot);
	light.SetPitch(XMConvertToRadians(-89.0f));
	light.SetIntensity(2.0f);
	light.SetRange(20.0f);
	light.EnableShadows();
	light.SetShadowBias(0.001f);
	*/

	SceneLight& light2 = *(m_Lights[1]);
	light2.SetEnbled(true);
	light2.SetPosition({ 0.0f, 0.0f, -3.0f });
	light2.SetType(SceneLight::LightType::Directional);
	light2.SetYaw(XMConvertToRadians(45.0f));
	light2.SetPitch(XMConvertToRadians(-45.0f));
	light2.SetIntensity(1.5f);
	light2.EnableShadows();
	
	mat1.SetAlbedoMap(textureMgr->getTexture(L"armor_albedo"));
	mat1.SetRoughnessMap(textureMgr->getTexture(L"armor_roughness"));
	mat1.SetNormalMap(textureMgr->getTexture(L"armor_normal"));

	if (m_LoadOnOpen)
	{
		loadSettings(std::string(m_SaveFilePath));
		applyFilterStack();
	}
}


App1::~App1()
{
	// probably not the best place to put this...
	if (m_SaveOnExit) saveSettings(std::string(m_SaveFilePath));

	// Release the Direct3D object.
	if (m_Terrain) delete m_Terrain;
	if (m_TerrainShader) delete m_TerrainShader;
	if (m_LightShader) delete m_LightShader;

	if (m_SrcRenderTarget) delete m_SrcRenderTarget;
	if (m_DstRenderTarget) delete m_DstRenderTarget;

	for (auto filter : m_HeightmapFilters)
	{
		if (filter) delete filter;
	}
	m_HeightmapFilters.clear();
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	m_Time += timer->getTime();

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	for (auto light : m_Lights)
	{
		if (light->IsShadowsEnabled())
			depthPass(light);
	}
	renderer->resetViewport();


	m_DstRenderTarget->Clear(renderer->getDeviceContext(), { 0.39f, 0.58f, 0.92f, 1.0f });
	m_DstRenderTarget->Set(renderer->getDeviceContext());

	worldPass();

	if (m_LightDebugSpheres) renderLightDebugSpheres();

	// post processing
	renderer->setZBuffer(false);
	if (!wireframeToggle && m_EnablePostProcessing)
	{
		// water is a post-processing effect and rendered afterwards
		//waterPass();
		
		SwitchRenderTarget();

		m_ToneMappingShader->setShaderParameters(renderer->getDeviceContext(), m_SrcRenderTarget->GetColourSRV(), m_SrcRenderTarget->GetDepthSRV());
		m_ToneMappingShader->Render(renderer->getDeviceContext());

	}

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	// Default camera position for orthographic rendering
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();			// ortho matrix for 2D rendering

	// output to backbuffer
	renderer->setBackBufferRenderTarget();
	{
		m_OutputMesh->sendData(renderer->getDeviceContext());
		m_TextureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, m_DstRenderTarget->GetColourSRV());
		m_TextureShader->render(renderer->getDeviceContext(), m_OutputMesh->getIndexCount());
	}

	// Render ortho mesh
	if (m_ShowShadowMap && m_Lights[m_SelectedShadowMap]->GetShadowMap() != nullptr)
	{
		m_ShadowMapMesh->sendData(renderer->getDeviceContext());
		m_TextureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, m_Lights[m_SelectedShadowMap]->GetShadowMap()->getDepthMapSRV());
		m_TextureShader->render(renderer->getDeviceContext(), m_ShadowMapMesh->getIndexCount());
	}
	renderer->setZBuffer(true);

	// GUI
	
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build GUI
	gui();

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Swap the buffers
	renderer->endScene();

	return true;
}

void App1::depthPass(SceneLight* light)
{
	// bind shadow map
	assert(light->GetShadowMap() && "Light doesnt have a shadow map!");
	light->GetShadowMap()->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	// get world view projection matrices
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	light->GenerateViewMatrix();
	XMMATRIX lightViewMatrix = light->GetViewMatrix();
	XMMATRIX lightProjectionMatrix = light->GetProjectionMatrix();

	// render world with an unlit shader
	XMMATRIX w = worldMatrix * XMMatrixTranslation(2.0f, 1.0f, 5.0f);
	m_Cube->sendData(renderer->getDeviceContext());
	m_UnlitShader->setShaderParameters(renderer->getDeviceContext(), w, lightViewMatrix, lightProjectionMatrix);
	m_UnlitShader->render(renderer->getDeviceContext(), m_Cube->getIndexCount());

	w = worldMatrix * XMMatrixTranslation(6.0f, 1.0f, 5.0f);
	m_Sphere->sendData(renderer->getDeviceContext());
	m_UnlitShader->setShaderParameters(renderer->getDeviceContext(), w, lightViewMatrix, lightProjectionMatrix);
	m_UnlitShader->render(renderer->getDeviceContext(), m_Sphere->getIndexCount());

	m_Plane->sendData(renderer->getDeviceContext());
	m_UnlitShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, lightViewMatrix, lightProjectionMatrix);
	m_UnlitShader->render(renderer->getDeviceContext(), m_Plane->getIndexCount());
}

void App1::worldPass()
{
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();


	if (false) {
		XMMATRIX w = worldMatrix * m_TerrainTransform.GetMatrix();

		// Send geometry data, set shader parameters, render object with shader
		m_Terrain->SendData(renderer->getDeviceContext());
		m_TerrainShader->SetShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix, m_Terrain->GetSRV(), nullptr, camera);
		m_TerrainShader->Render(renderer->getDeviceContext(), m_Terrain->GetIndexCount());
	}

	if (true)
	{
		XMMATRIX w = worldMatrix * XMMatrixTranslation(2.0f, 1.0f, 5.0f);

		m_Cube->sendData(renderer->getDeviceContext());
		m_LightShader->setShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix, m_Lights.size(), m_Lights.data(), camera, &mat1);
		m_LightShader->render(renderer->getDeviceContext(), m_Cube->getIndexCount());


		w = worldMatrix * XMMatrixTranslation(6.0f, 1.0f, 5.0f);

		m_Sphere->sendData(renderer->getDeviceContext());
		m_LightShader->setShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix, m_Lights.size(), m_Lights.data(), camera, &mat2);
		m_LightShader->render(renderer->getDeviceContext(), m_Sphere->getIndexCount());

		m_Plane->sendData(renderer->getDeviceContext());
		m_LightShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, m_Lights.size(), m_Lights.data(), camera, &mat1);
		m_LightShader->render(renderer->getDeviceContext(), m_Plane->getIndexCount());
	}

	// draw skybox
	if (true)
	{
		XMFLOAT3 eye = camera->getPosition();
		XMMATRIX w = worldMatrix * XMMatrixTranslation(eye.x, eye.y, eye.z);
		m_Skybox->Render(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix);

		// this resets the raster/om state to normal
		renderer->setWireframeMode(false);
		renderer->setZBuffer(true);
	}
}

void App1::SwitchRenderTarget()
{
	RenderTarget* temp = m_SrcRenderTarget;
	m_SrcRenderTarget = m_DstRenderTarget;
	m_DstRenderTarget = temp;

	m_DstRenderTarget->Set(renderer->getDeviceContext());
}

void App1::waterPass()
{
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	{

		renderer->setZBuffer(false);
		m_WaterShader->setShaderParameters(renderer->getDeviceContext(), viewMatrix, projectionMatrix, m_SrcRenderTarget->GetColourSRV(), m_SrcRenderTarget->GetDepthSRV(), nullptr, camera, m_Time);
		m_WaterShader->Render(renderer->getDeviceContext());
		renderer->setZBuffer(true);
	}
}

void App1::renderLightDebugSpheres()
{
	XMMATRIX worldMatrix = renderer->getWorldMatrix() * XMMatrixScaling(0.1f, 0.1f, 0.1f);
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	m_LightDebugSphereMesh->sendData(renderer->getDeviceContext());

	for (auto& light : m_Lights)
	{
		if (!light->IsEnabled()) continue;

		XMFLOAT3 p = light->GetPosition();
		XMMATRIX w = worldMatrix * XMMatrixTranslation(p.x, p.y, p.z);

		m_UnlitShader->setShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix);
		m_UnlitShader->render(renderer->getDeviceContext(), m_LightDebugSphereMesh->getIndexCount());
	}
}

void App1::gui()
{
	if (ImGui::CollapsingHeader("General"))
	{
		static bool showDemo = false;
		ImGui::Checkbox("Show demo", &showDemo);
		if (showDemo)
		{
			ImGui::ShowDemoWindow();
			return;
		}

		ImGui::Text("FPS: %.2f", timer->getFPS());
		ImGui::Checkbox("Wireframe mode", &wireframeToggle);
		ImGui::Separator();

		XMFLOAT3 camPos = camera->getPosition();
		if (ImGui::DragFloat3("Camera Pos", &camPos.x, 0.1f))
			camera->setPosition(camPos.x, camPos.y, camPos.z);
		XMFLOAT3 camRot = camera->getRotation();
		if (ImGui::DragFloat3("Camera Rot", &camRot.x, 0.5f))
			camera->setRotation(camRot.x, camRot.y, camRot.z);
	}
	ImGui::Separator();

	/*
	if (ImGui::CollapsingHeader("Save/Load Settings"))
	{
		ImGui::InputText("Save file", m_SaveFilePath, IM_ARRAYSIZE(m_SaveFilePath));

		if (ImGui::Button("Save"))
			saveSettings(std::string(m_SaveFilePath));
		ImGui::SameLine();
		if (ImGui::Button("Open"))
		{
			loadSettings(std::string(m_SaveFilePath));
			applyFilterStack();
		}

		ImGui::Checkbox("Load On Open", &m_LoadOnOpen);
		ImGui::Checkbox("Save On Exit", &m_SaveOnExit);
	}
	ImGui::Separator();
	*/

	if (ImGui::CollapsingHeader("Lighting"))
	{
		if (ImGui::TreeNode("Debug"))
		{
			ImGui::Checkbox("Debug Spheres", &m_LightDebugSpheres);
			ImGui::Checkbox("Display Shadow Map", &m_ShowShadowMap);
			if (m_ShowShadowMap)
				ImGui::SliderInt("Shadow map", &m_SelectedShadowMap, 0, static_cast<int>(m_Lights.size() - 1));

			ImGui::TreePop();
		}
		ImGui::Separator();

		if (ImGui::TreeNode("Global"))
		{
			m_GlobalLighting->SettingsGUI();
			ImGui::TreePop();
		}

		ImGui::Separator();

		int index = 0;
		for (auto& light : m_Lights)
		{
			if (ImGui::TreeNode((void*)(intptr_t)index, "Light %d", index))
			{
				ImGui::Separator();

				light->SettingsGUI();

				ImGui::TreePop();
				ImGui::Separator();
			}

			index++;
		}
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Materials"))
	{
		if (ImGui::TreeNode("Material 1"))
		{
			mat1.SettingsGUI();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Material 2"))
		{
			mat2.SettingsGUI();
			ImGui::TreePop();
		}
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Post Processing"))
	{
		ImGui::Checkbox("Enable", &m_EnablePostProcessing);

		if (ImGui::TreeNode("Tone mapping"))
		{
			m_ToneMappingShader->SettingsGUI();
			ImGui::TreePop();
		}
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Ocean"))
	{
		m_WaterShader->SettingsGUI();
	}
	ImGui::Separator();
	
	if (ImGui::CollapsingHeader("Terrain"))
	{
		m_TerrainShader->GUI();
		ImGui::Separator();
		ImGui::Checkbox("Open Generation Settings", &m_TerrainSettingsOpen);
		if (m_TerrainSettingsOpen) terrainSettingsMenu();
	}
	ImGui::Separator();
}

void App1::terrainSettingsMenu()
{
	if (!ImGui::Begin("Terrain Settings", &m_TerrainSettingsOpen))
	{
		ImGui::End();
		return;
	}

	bool regenerateTerrain = false;

	struct FuncHolder { // to allow inline function declaration
		static bool ItemGetter(void* data, int idx, const char** out_str)
		{
			*out_str = ((IHeightmapFilter**)data)[idx]->Label();
			return true;
		}
	};

	ImGui::Text("Filter Stack:");
	ImGui::Combo("Filter", &m_SelectedHeightmapFilter, &FuncHolder::ItemGetter, m_HeightmapFilters.data(), static_cast<int>(m_HeightmapFilters.size()));

	if (ImGui::Button("+"))
		ImGui::OpenPopup("addfilter_popup");
	regenerateTerrain |= addTerrainFilterMenu();

	ImGui::SameLine();
	if (ImGui::Button("-"))
	{
		if (m_HeightmapFilters.size() > 0)
		{
			delete m_HeightmapFilters[m_SelectedHeightmapFilter];
			m_HeightmapFilters.erase(m_HeightmapFilters.begin() + m_SelectedHeightmapFilter);

			if (m_HeightmapFilters.empty()) m_SelectedHeightmapFilter = -1;
			if (m_SelectedHeightmapFilter > 0) --m_SelectedHeightmapFilter;

			regenerateTerrain = true;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("^"))
	{
		if (m_SelectedHeightmapFilter > 0 && m_HeightmapFilters.size() > 1)
		{
			std::iter_swap(m_HeightmapFilters.begin() + m_SelectedHeightmapFilter - 1, m_HeightmapFilters.begin() + m_SelectedHeightmapFilter);
			--m_SelectedHeightmapFilter;

			regenerateTerrain = true;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("v"))
	{
		if (m_SelectedHeightmapFilter < m_HeightmapFilters.size() - 1 && m_HeightmapFilters.size() > 1)
		{
			std::iter_swap(m_HeightmapFilters.begin() + m_SelectedHeightmapFilter, m_HeightmapFilters.begin() + m_SelectedHeightmapFilter + 1);
			++m_SelectedHeightmapFilter;

			regenerateTerrain = true;
		}
	}

	if (m_SelectedHeightmapFilter >= 0)
	{
		ImGui::Separator();
		ImGui::Text("Filter Settings");
		ImGui::Separator();

		regenerateTerrain |= m_HeightmapFilters[m_SelectedHeightmapFilter]->SettingsGUI();
	}

	if (regenerateTerrain)
		applyFilterStack();

	ImGui::End();
}

bool App1::addTerrainFilterMenu()
{
	if (ImGui::BeginPopup("addfilter_popup"))
	{
		int selected_filter = -1;

		ImGui::Text("New Filter...");
		ImGui::Separator();
		for (int i = 0; i < m_AllFilterNames.size(); i++)
		{
			if (ImGui::Selectable(m_AllFilterNames[i]))
				selected_filter = i;
		}
		ImGui::EndPopup();
		
		if (selected_filter == -1) return false;

		IHeightmapFilter* newFilter = createFilterFromIndex(selected_filter);

		m_SelectedHeightmapFilter = static_cast<int>(m_HeightmapFilters.size());
		m_HeightmapFilters.push_back(newFilter);

		return true;
	}
	return false;
}

void App1::applyFilterStack()
{
	for (auto filter : m_HeightmapFilters)
	{
		filter->Run(renderer->getDeviceContext(), m_Terrain->GetUAV(), m_Terrain->GetHeightmapResolution());
	}
}

IHeightmapFilter* App1::createFilterFromIndex(int index)
{
	IHeightmapFilter* newFilter = nullptr;
	switch (index)
	{
	case 0: newFilter = new SimpleNoiseFilter(renderer->getDevice()); break;
	case 1: newFilter = new RidgeNoiseFilter(renderer->getDevice()); break;
	case 2: newFilter = new WarpedSimpleNoiseFilter(renderer->getDevice()); break;
	case 3: newFilter = new TerrainNoiseFilter(renderer->getDevice()); break;
	default: break;
	}
	assert(newFilter != nullptr);

	return newFilter;
}

void App1::saveSettings(const std::string& file)
{
	nlohmann::json serialized;

	// serialize filter data
	serialized["filters"] = nlohmann::json::array();
	for (auto filter : m_HeightmapFilters)
	{
		serialized["filters"].push_back(filter->Serialize());
	}

	serialized["waterSettings"] = m_WaterShader->Serialize();
	serialized["terrainSettings"] = m_TerrainShader->Serialize();

	std::string serializedString = serialized.dump();
	std::ofstream outfile(file);

	outfile << serialized << std::endl;
	outfile.close();
	
}

void App1::loadSettings(const std::string& file)
{
	
	// clear out existing settings
	for (auto filter : m_HeightmapFilters)
		delete filter;
	m_HeightmapFilters.clear();

	// load data from file
	std::ifstream infile(file);
	nlohmann::json data;
	infile >> data;
	infile.close();

	// construct objects from data
	if (data.contains("filters"))
	{
		for (auto filter : data["filters"])
		{
			if (!filter.contains("name")) continue;
			// work out the filter index
			int index = 0;
			for (const auto& name : m_AllFilterNames)
			{
				if (name == filter["name"]) break;
				index++;
			}
			if (index == m_AllFilterNames.size()) continue; // saved filter name must be invalid

			IHeightmapFilter* newFilter = createFilterFromIndex(index);
			newFilter->LoadFromJson(filter);
			m_HeightmapFilters.push_back(newFilter);
		}
	}

	if (data.contains("waterSettings")) m_WaterShader->LoadFromJson(data["waterSettings"]);
	if (data.contains("terrainSettings")) m_TerrainShader->LoadFromJson(data["terrainSettings"]);
}
