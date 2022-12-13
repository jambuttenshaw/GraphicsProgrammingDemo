#include "Material.h"

#include "imGUI/imgui.h"


void Material::SettingsGUI()
{
	if (m_AlbedoMap)
		ImGui::Checkbox("Use Albedo Map", &m_UseAlbedoMap);
	if (!m_UseAlbedoMap || !m_AlbedoMap)
		ImGui::ColorEdit3("Albedo", &m_Albedo.x);

	if (m_RoughnessMap)
		ImGui::Checkbox("Use Roughness Map", &m_UseRoughnessMap);
	if (!m_UseRoughnessMap || !m_RoughnessMap)
		ImGui::SliderFloat("Roughness", &m_Roughness, 0.001f, 1.0f);
	
	if (m_NormalMap)
		ImGui::Checkbox("Use Normal Map", &m_UseNormalMap);

	ImGui::SliderFloat("Metalness", &m_Metalness, 0.0f, 1.0f);

}

void Material::LoadFromDirectory(TextureManager* texManager, const std::string& dir)
{

}
