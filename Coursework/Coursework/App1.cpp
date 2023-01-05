#include "App1.h"

#include <nlohmann/json.hpp>

#include "LightShader.h"
#include "TerrainShader.h"
#include "TextureShader.h"

#include "UnlitShader.h"
#include "UnlitTerrainShader.h"

#include "WaterShader.h"
#include "MeasureLuminanceShader.h"
#include "BloomShader.h"
#include "FinalPassShader.h"

#include "GlobalLighting.h"
#include "Cubemap.h"
#include "Skybox.h"

#include "ShadowCubemap.h"

#include "HeightmapFilters.h"
#include "SerializationHelper.h"


App1::App1()
{
	m_TerrainMesh = nullptr;
	m_TerrainShader = nullptr;

	strcpy_s(m_SaveFilePath, "res/settings/earth.json");
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Load textures
	textureMgr->loadTexture(L"oceanNormalMapA", L"res/waterNormals1.png");
	textureMgr->loadTexture(L"oceanNormalMapB", L"res/waterNormals2.png");

	{
		Material* mat = m_MaterialLibrary.CreateMaterial("Grass");
		mat->LoadPBRFromDir(renderer->getDevice(), renderer->getDeviceContext(), L"res/pbr/grass");
	}
	{
		Material* mat = m_MaterialLibrary.CreateMaterial("Dirt");
		mat->LoadPBRFromDir(renderer->getDevice(), renderer->getDeviceContext(), L"res/pbr/dirt");
	}
	{
		Material* mat = m_MaterialLibrary.CreateMaterial("Sand");
		mat->LoadPBRFromDir(renderer->getDevice(), renderer->getDeviceContext(), L"res/pbr/sand");
	}
	{
		Material* mat = m_MaterialLibrary.CreateMaterial("Rock");
		mat->LoadPBRFromDir(renderer->getDevice(), renderer->getDeviceContext(), L"res/pbr/rock");
	}
	{
		Material* mat = m_MaterialLibrary.CreateMaterial("Snow");
		mat->LoadPBRFromDir(renderer->getDevice(), renderer->getDeviceContext(), L"res/pbr/snow");
	}
	{
		Material* mat = m_MaterialLibrary.CreateMaterial("Worn Shiny Metal");
		mat->LoadPBRFromDir(renderer->getDevice(), renderer->getDeviceContext(), L"res/pbr/worn_shiny_metal");
		mat->SetMetalness(1.0f);
	}
	{
		Material* mat = m_MaterialLibrary.CreateMaterial("Cement");
		mat->LoadPBRFromDir(renderer->getDevice(), renderer->getDeviceContext(), L"res/pbr/cement");
	}
	{
		Material* mat = m_MaterialLibrary.CreateMaterial("Gold");
		mat->LoadPBRFromDir(renderer->getDevice(), renderer->getDeviceContext(), L"res/pbr/gold");
		mat->SetMetalness(1.0f);
	}
	{
		Material* mat = m_MaterialLibrary.CreateMaterial("Copper");
		mat->LoadPBRFromDir(renderer->getDevice(), renderer->getDeviceContext(), L"res/pbr/copper");
		mat->SetMetalness(1.0f);
	}
	{
		Material* mat = m_MaterialLibrary.CreateMaterial("Granite");
		mat->LoadPBRFromDir(renderer->getDevice(), renderer->getDeviceContext(), L"res/pbr/granite");
	}

	m_GlobalLighting = new GlobalLighting(renderer->getDevice());

	m_LightShader = new LightShader(renderer->getDevice(), hwnd, m_GlobalLighting);
	m_TerrainShader = new TerrainShader(renderer->getDevice(), m_GlobalLighting);
	m_TextureShader = new TextureShader(renderer->getDevice(), hwnd);
	
	m_UnlitShader = new UnlitShader(renderer->getDevice(), hwnd);
	m_UnlitTerrainShader = new UnlitTerrainShader(renderer->getDevice());
	
	m_WaterShader = new WaterShader(renderer->getDevice(), m_GlobalLighting, textureMgr->getTexture(L"oceanNormalMapA"), textureMgr->getTexture(L"oceanNormalMapB"));
	m_MeasureLuminenceShader = new MeasureLuminanceShader(renderer->getDevice(), screenWidth, screenHeight);
	m_BloomShader = new BloomShader(renderer->getDevice(), screenWidth / 2, screenHeight / 2, 5);
	m_FinalPassShader = new FinalPassShader(renderer->getDevice());

	m_SceneRenderTexture = new RenderTarget(renderer->getDevice(), screenWidth, screenHeight);
	m_WaterRenderTexture = new RenderTarget(renderer->getDevice(), screenWidth, screenHeight);

	m_EnvironmentMap = new Cubemap(renderer->getDevice(), 
		"res/skybox/right.png", "res/skybox/left.png", 
		"res/skybox/top.png", "res/skybox/bottom.png",
		"res/skybox/front.png", "res/skybox/back.png");

	m_GlobalLighting->SetAndProcessEnvironmentMap(renderer->getDeviceContext(), m_EnvironmentMap);
	m_Skybox = new Skybox(renderer->getDevice(), m_EnvironmentMap);

	m_CubeMesh = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	m_SphereMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	m_PlaneMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 16);
	m_ShadowMapMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), 300, 300, (screenWidth / 2) - 150, (screenHeight / 2) - 150);
	m_TerrainMesh = new TerrainMesh(renderer->getDevice(), 50.0f);

	m_ShadowRasterDesc.FillMode = D3D11_FILL_SOLID;
	m_ShadowRasterDesc.CullMode = D3D11_CULL_BACK;
	m_ShadowRasterDesc.FrontCounterClockwise = true;
	m_ShadowRasterDesc.DepthBias = 50000;
	m_ShadowRasterDesc.DepthBiasClamp = 0.0f;
	m_ShadowRasterDesc.SlopeScaledDepthBias = 1.0f;
	m_ShadowRasterDesc.DepthClipEnable = true;
	m_ShadowRasterDesc.ScissorEnable = false;
	m_ShadowRasterDesc.MultisampleEnable = false;
	m_ShadowRasterDesc.AntialiasedLineEnable = false;
	renderer->getDevice()->CreateRasterizerState(&m_ShadowRasterDesc, &m_ShadowRasterizerState);

	//camera->setPosition(33.0f, 6.0f, -9.0f);
	camera->setPosition(0.0f, 3.0f, -2.0f);
	camera->setRotation(0.0f, 0.0f, 0.0f);

	// create game objects
	
	{
		m_GameObjects.push_back({ { 7, 5, 8 },		m_PlaneMesh,	m_MaterialLibrary.GetMaterial("Copper") });
		m_GameObjects.push_back({ { 15, 6, 19 },	m_SphereMesh,	m_MaterialLibrary.GetMaterial("Granite") });
		m_GameObjects.push_back({ { 12, 6.5f, 18 }, m_SphereMesh,	m_MaterialLibrary.GetMaterial("Worn Shiny Metal") });
		m_GameObjects.push_back({ { 11, 8, 21 },	m_CubeMesh,		m_MaterialLibrary.GetMaterial("Gold") });
		m_GameObjects.push_back({ { 9, 7, 15 },		m_CubeMesh,		m_MaterialLibrary.GetMaterial("Cement") });
	}
	
	m_GameObjects.push_back({ m_TerrainMesh, m_MaterialLibrary.GetMaterial("Sand") });
	GameObject& terrainGO = m_GameObjects.back();
	terrainGO.AddMaterial(m_MaterialLibrary.GetMaterial("Grass"));
	terrainGO.AddMaterial(m_MaterialLibrary.GetMaterial("Dirt"));
	terrainGO.AddMaterial(m_MaterialLibrary.GetMaterial("Rock"));
	terrainGO.AddMaterial(m_MaterialLibrary.GetMaterial("Snow"));

	// create lights
	for (auto& light : m_Lights)
	{
		light = new SceneLight(renderer->getDevice());
		light->SetPosition({ 0.0f, 0.0f, -10.0f });
	}

	// setup default light settings
	SceneLight& light = *(m_Lights[0]);
	light.SetEnbled(true);
	light.SetColour({ 1.0f, 0.815f, 0.231f });
	light.SetPosition({ -6.2f, 15.0f, 28.8f });
	light.SetType(SceneLight::LightType::Directional);
	light.SetYaw(XMConvertToRadians(166.0f));
	light.SetPitch(XMConvertToRadians(-16.0f));
	light.SetIntensity(2.0f);
	light.EnableShadows();

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


	if (m_LightShader) delete m_LightShader;
	if (m_TerrainShader) delete m_TerrainShader;
	if (m_TextureShader) delete m_TextureShader;

	if (m_UnlitShader) delete m_UnlitShader;
	if (m_UnlitTerrainShader) delete m_UnlitTerrainShader;

	if (m_WaterShader) delete m_WaterShader;
	if (m_MeasureLuminenceShader) delete m_MeasureLuminenceShader;
	if (m_BloomShader) delete m_BloomShader;
	if (m_FinalPassShader) delete m_FinalPassShader;

	if (m_CubeMesh) delete m_CubeMesh;
	if (m_SphereMesh) delete m_SphereMesh;
	if (m_PlaneMesh) delete m_PlaneMesh;
	if (m_TerrainMesh) delete m_TerrainMesh;
	if (m_ShadowMapMesh) delete m_ShadowMapMesh;

	if (m_SceneRenderTexture) delete m_SceneRenderTexture;
	if (m_WaterRenderTexture) delete m_WaterRenderTexture;

	if (m_GlobalLighting) delete m_GlobalLighting;
	if (m_EnvironmentMap) delete m_EnvironmentMap;
	if (m_Skybox) delete m_Skybox;

	for (auto& light : m_Lights)
	{
		if (light) delete light;
	}

	m_ShadowRasterizerState->Release();

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
	// Generate the view matrix based on the camera's position.
	camera->update();

	// shadow passes
	renderer->getDeviceContext()->RSSetState(m_ShadowRasterizerState);
	for (auto light : m_Lights)
	{
		if (light->IsEnabled() && light->IsShadowsEnabled())
			depthPass(light);
	}
	renderer->resetViewport();
	renderer->setWireframeMode(wireframeToggle); // resets raster state

	// render to a texture if post processing is enabled
	if (m_EnablePostProcessing && !wireframeToggle)
	{
		m_SceneRenderTexture->Clear(renderer->getDeviceContext(), m_ClearColour);
		m_SceneRenderTexture->Set(renderer->getDeviceContext());
	}
	else
	{
		renderer->beginScene(m_ClearColour.x, m_ClearColour.y, m_ClearColour.z, m_ClearColour.w);
		renderer->setBackBufferRenderTarget();
	}

	// draw everything in the world
	worldPass();

	if (m_LightDebugSpheres) renderLightDebugSpheres();

	// post processing
	renderer->setZBuffer(false);
	if (m_EnablePostProcessing && !wireframeToggle)
	{
		// water is a post-processing effect and rendered afterwards
		waterPass();
		RenderTarget* outputRT = m_WaterRenderTexture;

		// output to backbuffer for final pass
		renderer->setBackBufferRenderTarget();

		m_MeasureLuminenceShader->Run(renderer->getDeviceContext(), outputRT->GetColourSRV(), outputRT->GetWidth(), outputRT->GetHeight());
		m_BloomShader->Run(renderer->getDeviceContext(), outputRT->GetColourSRV());

		m_FinalPassShader->setShaderParameters(renderer->getDeviceContext(), outputRT->GetColourSRV(), outputRT->GetDepthSRV(), m_MeasureLuminenceShader->GetResult(), outputRT->GetWidth(), outputRT->GetHeight(), m_BloomShader->GetSRV());
		m_FinalPassShader->Render(renderer->getDeviceContext());
	}

	// Debug: draw shadow map on the screen
	if (m_ShowShadowMap && m_Lights[m_SelectedShadowMap]->IsShadowsEnabled())
	{
		m_ShadowMapMesh->sendData(renderer->getDeviceContext());
		ID3D11ShaderResourceView* shadowmapSRV = nullptr;
		if (m_Lights[m_SelectedShadowMap]->GetType() == SceneLight::LightType::Point)
			shadowmapSRV = m_Lights[m_SelectedShadowMap]->GetShadowCubemap()->GetSRV(m_SelectedShadowCubemapFace);
		else
			shadowmapSRV = m_Lights[m_SelectedShadowMap]->GetShadowMap()->getDepthMapSRV();

		XMMATRIX worldMatrix = renderer->getWorldMatrix();
		XMMATRIX orthoViewMatrix = camera->getOrthoViewMatrix();	
		XMMATRIX orthoMatrix = renderer->getOrthoMatrix();			

		m_TextureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, orthoViewMatrix, orthoMatrix, shadowmapSRV);
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
	assert(light->GetShadowMap() || light->GetShadowCubemap() && "Light doesnt have a shadow map!");

	// get world view projection matrices
	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	XMMATRIX lightViewMatrices[6];
	int matrixCount = 0;
	if (light->GetType() == SceneLight::LightType::Point)
	{
		light->GetPointLightViewMatrices(lightViewMatrices);
		matrixCount = 6;
	}
	else
	{
		light->GenerateViewMatrix();
		lightViewMatrices[0] = light->GetViewMatrix();
		matrixCount = 1;
	}
	XMMATRIX lightProjectionMatrix = light->GetProjectionMatrix();

	for (int m = 0; m < matrixCount; m++)
	{
		if (light->GetType() == SceneLight::LightType::Point)
			light->GetShadowCubemap()->BindDSV(renderer->getDeviceContext(), m);
		else
			light->GetShadowMap()->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

		// render world with an unlit shader
		for (auto& go : m_GameObjects)
		{
			if (!go.castsShadows) continue;

			XMMATRIX w = worldMatrix * go.transform.GetMatrix();
			switch (go.meshType)
			{
			case GameObject::MeshType::Regular:
				go.mesh.regular->sendData(renderer->getDeviceContext());
				m_UnlitShader->setShaderParameters(renderer->getDeviceContext(), w, lightViewMatrices[m], lightProjectionMatrix);
				m_UnlitShader->render(renderer->getDeviceContext(), go.mesh.regular->getIndexCount());
				break;
			case GameObject::MeshType::Terrain:
				go.mesh.terrain->SendData(renderer->getDeviceContext());
				m_UnlitTerrainShader->SetShaderParameters(renderer->getDeviceContext(), w, lightViewMatrices[m], lightProjectionMatrix, go.mesh.terrain, camera->getPosition(), m_TerrainShader->GetMinMaxDist(), m_TerrainShader->GetMinMaxLOD(), m_TerrainShader->GetMinMaxHeightDeviation(), m_TerrainShader->GetDistanceLODBlending());
				m_UnlitTerrainShader->Render(renderer->getDeviceContext(), go.mesh.terrain->GetIndexCount());
				break;
			default:
				break;
			}
		}
	}
}

void App1::worldPass()
{
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();


	for (auto& go : m_GameObjects)
	{
		XMMATRIX w = worldMatrix * go.transform.GetMatrix();
		switch (go.meshType)
		{
		case GameObject::MeshType::Regular:
			go.mesh.regular->sendData(renderer->getDeviceContext());
			m_LightShader->setShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix, m_Lights.size(), m_Lights.data(), camera, go.materials[0]);
			m_LightShader->render(renderer->getDeviceContext(), go.mesh.regular->getIndexCount());
			break;
		case GameObject::MeshType::Terrain:
			go.mesh.terrain->SendData(renderer->getDeviceContext());
			m_TerrainShader->SetShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix, go.mesh.terrain, m_Lights.size(), m_Lights.data(), camera, go.materials);
			m_TerrainShader->Render(renderer->getDeviceContext(), go.mesh.terrain->GetIndexCount());
			break;
		default:
			break;
		}
	}

	// draw skybox
	if (m_DrawSkybox)
	{
		XMFLOAT3 eye = camera->getPosition();
		XMMATRIX w = worldMatrix * XMMatrixTranslation(eye.x, eye.y, eye.z);
		m_Skybox->Render(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix);

		// this resets the raster/om state to normal
		renderer->setWireframeMode(false);
		renderer->setZBuffer(true);
	}
}


void App1::waterPass()
{
	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	m_WaterRenderTexture->Clear(renderer->getDeviceContext(), m_ClearColour);
	m_WaterRenderTexture->Set(renderer->getDeviceContext());

	m_WaterShader->setShaderParameters(renderer->getDeviceContext(), viewMatrix, projectionMatrix, m_SceneRenderTexture, m_Lights.data(), m_Lights.size(), camera, m_Time);
	m_WaterShader->Render(renderer->getDeviceContext());
}

void App1::renderLightDebugSpheres()
{
	XMMATRIX worldMatrix = renderer->getWorldMatrix() * XMMatrixScaling(0.1f, 0.1f, 0.1f);
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	m_SphereMesh->sendData(renderer->getDeviceContext());

	for (auto& light : m_Lights)
	{
		if (!light->IsEnabled()) continue;

		XMFLOAT3 p = light->GetPosition();
		XMMATRIX w = worldMatrix * XMMatrixTranslation(p.x, p.y, p.z);

		m_UnlitShader->setShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix);
		m_UnlitShader->render(renderer->getDeviceContext(), m_SphereMesh->getIndexCount());
	}
}

void App1::gui()
{
	if (ImGui::CollapsingHeader("General"))
	{
		ImGui::Text("FPS: %.2f", timer->getFPS());
		ImGui::Checkbox("Wireframe mode", &wireframeToggle);
		ImGui::Separator();

		XMFLOAT3 camPos = camera->getPosition();
		if (ImGui::DragFloat3("Camera Pos", &camPos.x, 0.1f))
			camera->setPosition(camPos.x, camPos.y, camPos.z);
		XMFLOAT3 camRot = camera->getRotation();
		if (ImGui::DragFloat3("Camera Rot", &camRot.x, 0.5f))
			camera->setRotation(camRot.x, camRot.y, camRot.z);

		ImGui::Checkbox("Draw Skybox", &m_DrawSkybox);
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Lighting"))
	{
		if (ImGui::TreeNode("Debug"))
		{
			ImGui::Checkbox("Debug Spheres", &m_LightDebugSpheres);
			ImGui::Checkbox("Display Shadow Map", &m_ShowShadowMap);
			if (m_ShowShadowMap)
			{
				ImGui::SliderInt("Shadow map", &m_SelectedShadowMap, 0, static_cast<int>(m_Lights.size() - 1));
				if (m_Lights[m_SelectedShadowMap]->GetType() == SceneLight::LightType::Point)
					ImGui::SliderInt("Cubemap face", &m_SelectedShadowCubemapFace, 0, 5);
			}

			ImGui::TreePop();
		}
		ImGui::Separator();

		if (ImGui::TreeNode("Global"))
		{
			int selectedSkybox = m_SelectedSkybox;
			ImGui::Text("Select Environment:");
			ImGui::RadioButton("Sky", &selectedSkybox, 0); ImGui::SameLine();
			ImGui::RadioButton("Trees", &selectedSkybox, 1);

			if (selectedSkybox != m_SelectedSkybox)
			{
				m_SelectedSkybox = selectedSkybox;
				delete m_EnvironmentMap;
				if (m_SelectedSkybox == 0)
				{
					m_EnvironmentMap = new Cubemap(renderer->getDevice(), 
						"res/skybox/right.png", "res/skybox/left.png", 
						"res/skybox/top.png", "res/skybox/bottom.png",
						"res/skybox/front.png", "res/skybox/back.png");
				}
				else if (m_SelectedSkybox == 1)
				{
					m_EnvironmentMap = new Cubemap(renderer->getDevice(),
						"res/skybox2/px.png", "res/skybox2/nx.png",
						"res/skybox2/py.png", "res/skybox2/ny.png",
						"res/skybox2/pz.png", "res/skybox2/nz.png");
				}
				m_GlobalLighting->SetAndProcessEnvironmentMap(renderer->getDeviceContext(), m_EnvironmentMap);
				m_Skybox->SetCubemap(m_EnvironmentMap);
			}

			int bias = m_ShadowRasterDesc.DepthBias;
			if (ImGui::DragInt("Bias", &bias, 1000))
			{
				m_ShadowRasterDesc.DepthBias = bias;
				m_ShadowRasterizerState->Release();
				renderer->getDevice()->CreateRasterizerState(&m_ShadowRasterDesc, &m_ShadowRasterizerState);
			}

			m_GlobalLighting->SettingsGUI();
			ImGui::TreePop();
		}

		ImGui::Separator();

		int index = 0;
		for (auto& light : m_Lights)
		{
			if (ImGui::TreeNode((void*)((intptr_t)index + 14), "Light %d", index))
			{
				ImGui::Separator();

				light->SettingsGUI();

				ImGui::Separator();
				if (ImGui::Button("Move to Camera"))
					light->SetPosition(camera->getPosition());

				ImGui::TreePop();
				ImGui::Separator();
			}

			index++;
		}
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Materials"))
	{
		m_MaterialLibrary.MaterialSettingsGUI();
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Post Processing"))
	{
		ImGui::Checkbox("Enable", &m_EnablePostProcessing);

		if (ImGui::TreeNode("Water"))
		{
			m_WaterShader->SettingsGUI();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Bloom"))
		{
			m_BloomShader->SettingsGUI();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Tone mapping"))
		{
			m_FinalPassShader->SettingsGUI();
			ImGui::TreePop();
		}
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Game Objects"))
	{
		int index = 0;
		for (auto& go : m_GameObjects)
		{
			const char* typeStr;
			switch (go.meshType)
			{
			case GameObject::MeshType::Regular: typeStr = typeid(*go.mesh.regular).name(); break;
			case GameObject::MeshType::Terrain: typeStr = typeid(*go.mesh.terrain).name(); break;
			default: typeStr = "class Unknown"; break;
			}
			if (ImGui::TreeNode((void*)((intptr_t)index + 537), "%s %d", typeStr + 6, index))
			{
				go.SettingsGUI(&m_MaterialLibrary);
				ImGui::TreePop();
			}
			index++;
		}
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
		filter->Run(renderer->getDeviceContext(), m_TerrainMesh->GetHeightmapUAV(), m_TerrainMesh->GetHeightmapResolution());
	}
	m_TerrainMesh->PreprocessHeightmap(renderer->getDeviceContext());
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
}
