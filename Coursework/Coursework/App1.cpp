#include "App1.h"

#include <nlohmann/json.hpp>

#include "LightShader.h"
#include "TerrainShader.h"
#include "WaterShader.h"
#include "UnlitShader.h"

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

	m_LightShader = new LightShader(renderer->getDevice(), hwnd);
	m_TerrainShader = new TerrainShader(renderer->getDevice());
	m_WaterShader = new WaterShader(renderer->getDevice(), textureMgr->getTexture(L"oceanNormalMapA"), textureMgr->getTexture(L"oceanNormalMapB"));
	m_UnlitShader = new UnlitShader(renderer->getDevice(), hwnd);

	m_RenderTarget = new RenderTarget(renderer->getDevice(), screenWidth, screenHeight);

	m_LightDebugSphereMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	m_Terrain = new TerrainMesh(renderer->getDevice());

	m_Cube = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	m_Sphere = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());

	camera->setPosition(0.0f, 0.0f, -3.0f);
	camera->setRotation(0.0f, 0.0f, 0.0f);

	// setup default light settings
	SceneLight& light = m_Lights[0];
	light.SetEnbled(true);
	light.SetYaw(XMConvertToRadians(30.0f));
	light.SetPitch(XMConvertToRadians(-40.0f));

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

	if (m_RenderTarget) delete m_RenderTarget;

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


	// render world to render texture
	//m_RenderTarget->Clear(renderer->getDeviceContext(), { 0.39f, 0.58f, 0.92f, 1.0f });
	//if (!wireframeToggle) m_RenderTarget->Set(renderer->getDeviceContext());
	worldPass();

	if (m_LightDebugSpheres) renderLightDebugSpheres();

	//if (!wireframeToggle)
	//{
	//	// water is a post-processing effect and rendered afterwards
	//	renderer->setBackBufferRenderTarget();
	//	
	//	waterPass();
	//}

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
		XMMATRIX w = worldMatrix * XMMatrixTranslation(-2.0f, 0.0f, 0.0f);

		m_Cube->sendData(renderer->getDeviceContext());
		m_LightShader->setShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix, m_Lights.size(), m_Lights.data(), camera, &mat1);
		m_LightShader->render(renderer->getDeviceContext(), m_Cube->getIndexCount());


		w = worldMatrix * XMMatrixTranslation(2.0f, 0.0f, 0.0f);

		m_Sphere->sendData(renderer->getDeviceContext());
		m_LightShader->setShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix, m_Lights.size(), m_Lights.data(), camera, &mat2);
		m_LightShader->render(renderer->getDeviceContext(), m_Sphere->getIndexCount());
	}
}

void App1::waterPass()
{
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	{

		renderer->setZBuffer(false);
		m_WaterShader->setShaderParameters(renderer->getDeviceContext(), viewMatrix, projectionMatrix, m_RenderTarget->GetColourSRV(), m_RenderTarget->GetDepthSRV(), nullptr, camera, m_Time);
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
		if (!light.IsEnabled()) continue;
		if (light.GetType() == SceneLight::LightType::Directional) continue;

		XMFLOAT3 p = light.GetPosition();
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
		ImGui::Text("Global");
		ImGui::Checkbox("Debug Spheres", &m_LightDebugSpheres);
		m_LightShader->GlobalLightSettingsGUI();
		ImGui::Separator();

		int index = 0;
		for (auto& light : m_Lights)
		{
			if (ImGui::TreeNode((void*)(intptr_t)index, "Light %d", index))
			{
				ImGui::Separator();

				light.SettingsGUI();

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
