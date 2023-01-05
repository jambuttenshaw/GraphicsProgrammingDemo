#include "Material.h"

#include "imGUI/imgui.h"
#include "DTK/include/DDSTextureLoader.h"
#include "DTK/include/WICTextureLoader.h"

#include <fstream>


Material::~Material()
{
	if (m_AlbedoMap) m_AlbedoMap->Release();
	if (m_RoughnessMap) m_RoughnessMap->Release();
	if (m_NormalMap) m_NormalMap->Release();
	if (m_MetalnessMap) m_MetalnessMap->Release();
}

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

	if (m_MetalnessMap)
		ImGui::Checkbox("Use Metalness Map", &m_UseMetalnessMap);
	if (!m_UseMetalnessMap || !m_MetalnessMap)
		ImGui::SliderFloat("Metalness", &m_Metalness, 0.0f, 1.0f);
}

void Material::LoadPBRFromDir(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& dir)
{
	std::wstring albedoPath = dir + L"/albedo.png";
	std::wstring normalPath = dir + L"/normal.png";
	std::wstring roughnessPath = dir + L"/roughness.png";
	std::wstring metalnessPath = dir + L"/metalness.png";

	if (DoesFileExist(albedoPath.c_str()))
	{
		m_UseAlbedoMap = true;
		LoadTexture(device, deviceContext, albedoPath.c_str(), &m_AlbedoMap);
	}
	else
		m_UseAlbedoMap = false;

	if (DoesFileExist(normalPath.c_str()))
	{
		m_UseNormalMap = true;
		LoadTexture(device, deviceContext, normalPath.c_str(), &m_NormalMap);
	}
	else
		m_UseNormalMap = false;

	if (DoesFileExist(roughnessPath.c_str()))
	{
		m_UseRoughnessMap = true;
		LoadTexture(device, deviceContext, roughnessPath.c_str(), &m_RoughnessMap);
	}
	else
		m_UseRoughnessMap = false;

	if (DoesFileExist(metalnessPath.c_str()))
	{
		m_UseMetalnessMap = true;
		LoadTexture(device, deviceContext, metalnessPath.c_str(), &m_MetalnessMap);
	}
	else
		m_UseMetalnessMap = false;
}

bool Material::DoesFileExist(const wchar_t* filename) const
{
	std::ifstream infile(filename);
	return infile.good();
}

void Material::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* filename, ID3D11ShaderResourceView** srv)
{
	HRESULT hr;

	// check file extension for correct loading function.
	std::wstring fn(filename);
	auto idx = fn.rfind('.');

	assert(idx != std::string::npos && "File extension not found");
	std::wstring extension = fn.substr(idx + 1);

	// Load the texture in.
	if (extension == L"dds")
		hr = CreateDDSTextureFromFile(device, deviceContext, filename, NULL, srv);
	else
		hr = CreateWICTextureFromFile(device, deviceContext, filename, NULL, srv, 0);

	assert(hr == S_OK && "Texture failed to load");
}
