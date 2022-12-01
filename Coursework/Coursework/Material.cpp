#include "Material.h"

#include "imGUI/imgui.h"


void Material::SettingsGUI()
{
	ImGui::ColorEdit3("Albedo", &m_Albedo.x);
	ImGui::SliderFloat("Metalness", &m_Metalness, 0.0f, 1.0f);
	ImGui::SliderFloat("Roughness", &m_Roughness, 0.001f, 1.0f);
}
