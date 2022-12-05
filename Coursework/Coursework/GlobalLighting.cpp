#include "GlobalLighting.h"

#include "imGUI/imgui.h"


void GlobalLighting::SettingsGUI()
{
	ImGui::ColorEdit3("Global Ambient", &m_GlobalAmbient.x);
	ImGui::Checkbox("Enable Environment Map", &m_EnableEnvironmentMap);
}
