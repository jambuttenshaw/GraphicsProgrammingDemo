#include "App1.h"

#include <nlohmann/json.hpp>

#include "LightShader.h"
#include "TerrainShader.h"
#include "UnlitShader.h"
#include "TextureShader.h"

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

	//textureMgr->loadTexture(L"grass_albedo", L"res/pbr/grass/albedo.png");
	//textureMgr->loadTexture(L"grass_normal", L"res/pbr/grass/normal.png");
	//textureMgr->loadTexture(L"grass_roughness", L"res/pbr/grass/roughness.png");
	//
	//textureMgr->loadTexture(L"dirt_albedo", L"res/pbr/dirt/albedo.png");
	//textureMgr->loadTexture(L"dirt_normal", L"res/pbr/dirt/normal.png");
	//textureMgr->loadTexture(L"dirt_roughness", L"res/pbr/dirt/roughness.png");
	//
	//textureMgr->loadTexture(L"sand_albedo", L"res/pbr/sand/albedo.png");
	//textureMgr->loadTexture(L"sand_normal", L"res/pbr/sand/normal.png");
	//textureMgr->loadTexture(L"sand_roughness", L"res/pbr/sand/roughness.png");

	textureMgr->loadTexture(L"rock_albedo", L"res/pbr/rock/albedo.png");
	textureMgr->loadTexture(L"rock_normal", L"res/pbr/rock/normal.png");
	textureMgr->loadTexture(L"rock_roughness", L"res/pbr/rock/roughness.png");

	//textureMgr->loadTexture(L"snow_albedo", L"res/pbr/snow/albedo.png");
	//textureMgr->loadTexture(L"snow_normal", L"res/pbr/snow/normal.png");
	//textureMgr->loadTexture(L"snow_roughness", L"res/pbr/snow/roughness.png");
	//
	//textureMgr->loadTexture(L"shiny_metal_albedo", L"res/pbr/worn_shiny_metal/albedo.png");
	//textureMgr->loadTexture(L"shiny_metal_roughness", L"res/pbr/worn_shiny_metal/roughness.png");

	//grassMat.SetAlbedoMap(textureMgr->getTexture(L"grass_albedo"));
	//grassMat.SetNormalMap(textureMgr->getTexture(L"grass_normal"));
	//grassMat.SetRoughnessMap(textureMgr->getTexture(L"grass_roughness"));
	//
	//dirtMat.SetAlbedoMap(textureMgr->getTexture(L"dirt_albedo"));
	//dirtMat.SetNormalMap(textureMgr->getTexture(L"dirt_normal"));
	//dirtMat.SetRoughnessMap(textureMgr->getTexture(L"dirt_roughness"));
	//
	//sandMat.SetAlbedoMap(textureMgr->getTexture(L"sand_albedo"));
	//sandMat.SetNormalMap(textureMgr->getTexture(L"sand_normal"));
	//sandMat.SetRoughnessMap(textureMgr->getTexture(L"sand_roughness"));

	rockMat.SetAlbedoMap(textureMgr->getTexture(L"rock_albedo"));
	rockMat.SetNormalMap(textureMgr->getTexture(L"rock_normal"));
	rockMat.SetRoughnessMap(textureMgr->getTexture(L"rock_roughness"));

	//snowMat.SetAlbedoMap(textureMgr->getTexture(L"snow_albedo"));
	//snowMat.SetNormalMap(textureMgr->getTexture(L"snow_normal"));
	//snowMat.SetRoughnessMap(textureMgr->getTexture(L"snow_roughness"));
	//
	//shinyMetalMat.SetAlbedoMap(textureMgr->getTexture(L"shiny_metal_albedo"));
	//shinyMetalMat.SetRoughnessMap(textureMgr->getTexture(L"shiny_metal_roughness"));
	//shinyMetalMat.SetMetalness(1.0f);


	m_GlobalLighting = new GlobalLighting(renderer->getDevice());

	m_LightShader = new LightShader(renderer->getDevice(), hwnd, m_GlobalLighting);
	m_TerrainShader = new TerrainShader(renderer->getDevice());
	m_UnlitShader = new UnlitShader(renderer->getDevice(), hwnd);
	m_TextureShader = new TextureShader(renderer->getDevice(), hwnd);
	
	m_WaterShader = new WaterShader(renderer->getDevice(), textureMgr->getTexture(L"oceanNormalMapA"), textureMgr->getTexture(L"oceanNormalMapB"));
	m_MeasureLuminenceShader = new MeasureLuminanceShader(renderer->getDevice(), screenWidth, screenHeight);
	m_BloomShader = new BloomShader(renderer->getDevice(), screenWidth / 2, screenHeight / 2, 5);
	m_FinalPassShader = new FinalPassShader(renderer->getDevice());

	m_SrcRenderTarget = new RenderTarget(renderer->getDevice(), screenWidth, screenHeight);
	m_DstRenderTarget = new RenderTarget(renderer->getDevice(), screenWidth, screenHeight);

	m_OutputMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);

	m_EnvironmentMap = new Cubemap(renderer->getDevice(), 
		"res/skybox/right.png", "res/skybox/left.png", 
		"res/skybox/top.png", "res/skybox/bottom.png",
		"res/skybox/front.png", "res/skybox/back.png");

	m_GlobalLighting->SetAndProcessEnvironmentMap(renderer->getDeviceContext(), m_EnvironmentMap);
	m_Skybox = new Skybox(renderer->getDevice(), m_EnvironmentMap);

	m_Terrain = new TerrainMesh(renderer->getDevice());

	m_CubeMesh = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	m_SphereMesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	m_PlaneMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext(), 40);
	m_ShadowMapMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), 300, 300, (screenWidth / 2) - 150, (screenHeight / 2) - 150);

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

	camera->setPosition(0.0f, 5.0f, -5.0f);
	camera->setRotation(0.0f, 0.0f, 0.0f);

	// create game objects
	m_GameObjects.push_back({ { -20, 0, -20 }, m_PlaneMesh, &rockMat });
	m_GameObjects.push_back({ { -3, 2, 3 }, m_SphereMesh, &rockMat });
	m_GameObjects.push_back({ { -1, 4, 0 }, m_SphereMesh, &rockMat });
	m_GameObjects.push_back({ { 2, 3, 2 }, m_SphereMesh, &rockMat });
	m_GameObjects.push_back({ { 4, 1, -2 }, m_CubeMesh, &rockMat });
	m_GameObjects.push_back({ { 0, 1, 1 }, m_CubeMesh, &rockMat });

	// create lights
	for (auto& light : m_Lights)
	{
		light = new SceneLight(renderer->getDevice());
		light->SetPosition({ 0.0f, 0.0f, -10.0f });
	}

	// setup default light settings
	SceneLight& light = *(m_Lights[0]);
	light.SetEnbled(true);
	light.SetColour({ 0.985f, 0.968f, 0.415f });
	light.SetType(SceneLight::LightType::Point);
	light.SetPosition({ 1, 4, -5 });
	light.SetIntensity(4.0f);
	light.SetRange(20.0f);
	light.EnableShadows();

	SceneLight& light2 = *(m_Lights[1]);
	light2.SetEnbled(true);
	//light2.SetColour({ 0.352f, 0.791f, 0.946f });
	light2.SetPosition({ 0, 10, 0 });
	light2.SetType(SceneLight::LightType::Directional);
	light2.SetYaw(XMConvertToRadians(-10.0f));
	light2.SetPitch(XMConvertToRadians(-70.0f));
	light2.SetIntensity(3.5f);
	light2.EnableShadows();
	
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

	renderer->getDeviceContext()->RSSetState(m_ShadowRasterizerState);
	for (auto light : m_Lights)
	{
		if (light->IsEnabled() && light->IsShadowsEnabled())
			depthPass(light);
	}
	renderer->resetViewport();
	renderer->setWireframeMode(wireframeToggle);


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

		m_MeasureLuminenceShader->Run(renderer->getDeviceContext(), m_SrcRenderTarget->GetColourSRV(), m_SrcRenderTarget->GetWidth(), m_SrcRenderTarget->GetHeight());
		m_BloomShader->Run(renderer->getDeviceContext(), m_SrcRenderTarget->GetColourSRV());

		m_FinalPassShader->setShaderParameters(renderer->getDeviceContext(), m_SrcRenderTarget->GetColourSRV(), m_SrcRenderTarget->GetDepthSRV(), m_MeasureLuminenceShader->GetResult(), m_SrcRenderTarget->GetWidth(), m_SrcRenderTarget->GetHeight(), m_BloomShader->GetSRV(), m_BloomShader->GetLevels(), m_BloomShader->GetStrength());
		m_FinalPassShader->Render(renderer->getDeviceContext());

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
	if (m_ShowShadowMap && m_Lights[m_SelectedShadowMap]->IsShadowsEnabled())
	{
		m_ShadowMapMesh->sendData(renderer->getDeviceContext());
		ID3D11ShaderResourceView* shadowmapSRV = nullptr;
		if (m_Lights[m_SelectedShadowMap]->GetType() == SceneLight::LightType::Point)
			shadowmapSRV = m_Lights[m_SelectedShadowMap]->GetShadowCubemap()->GetSRV(m_SelectedShadowCubemapFace);
		else
			m_Lights[m_SelectedShadowMap]->GetShadowMap()->getDepthMapSRV();
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
			go.mesh->sendData(renderer->getDeviceContext());
			m_UnlitShader->setShaderParameters(renderer->getDeviceContext(), w, lightViewMatrices[m], lightProjectionMatrix);
			m_UnlitShader->render(renderer->getDeviceContext(), go.mesh->getIndexCount());
		}
	}
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
		for (auto& go : m_GameObjects)
		{
			if (!go.castsShadows) continue;

			XMMATRIX w = worldMatrix * go.transform.GetMatrix();
			go.mesh->sendData(renderer->getDeviceContext());
			m_LightShader->setShaderParameters(renderer->getDeviceContext(), w, viewMatrix, projectionMatrix, m_Lights.size(), m_Lights.data(), camera, go.material);
			m_LightShader->render(renderer->getDeviceContext(), go.mesh->getIndexCount());
		}
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
			{
				ImGui::SliderInt("Shadow map", &m_SelectedShadowMap, 0, static_cast<int>(m_Lights.size() - 1));
				if (m_Lights[m_SelectedShadowMap]->GetType() == SceneLight::LightType::Point)
					ImGui::SliderInt("Cubemap face", &m_SelectedShadowCubemapFace, 0, 5);
			}

			int bias = m_ShadowRasterDesc.DepthBias;
			if (ImGui::DragInt("Bias", &bias, 1000))
			{
				m_ShadowRasterDesc.DepthBias = bias;
				m_ShadowRasterizerState->Release();
				renderer->getDevice()->CreateRasterizerState(&m_ShadowRasterDesc, &m_ShadowRasterizerState);
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
		if (ImGui::TreeNode("Rock"))
		{
			rockMat.SettingsGUI();
			ImGui::TreePop();
		}
		/*
		if (ImGui::TreeNode("Material 2"))
		{
			mat2.SettingsGUI();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Material 3"))
		{
			mat3.SettingsGUI();
			ImGui::TreePop();
		}
		*/
	}
	ImGui::Separator();

	if (ImGui::CollapsingHeader("Post Processing"))
	{
		ImGui::Checkbox("Enable", &m_EnablePostProcessing);

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
