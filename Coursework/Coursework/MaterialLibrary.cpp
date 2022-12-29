#include "MaterialLibrary.h"

#include "Material.h"
#include "imGUI/imgui.h"


MaterialLibrary::~MaterialLibrary()
{
	for (auto& mat : m_Materials)
		delete mat.second;
}

Material* MaterialLibrary::CreateMaterial(const std::string& name)
{
	assert(m_Materials.find(name) == m_Materials.end());

	Material* newMat = new Material;
	m_Materials.insert({ name, newMat });

	return newMat;
}

Material* MaterialLibrary::GetMaterial(const std::string& matName) const
{
	assert(m_Materials.find(matName) != m_Materials.end());
	return m_Materials.at(matName);
}

const std::string MaterialLibrary::GetName(Material* mat) const
{
	for (auto& m : m_Materials)
	{
		if (m.second == mat)
			return m.first;
	}

	assert(false && "material not found");
	return std::string();
}

void MaterialLibrary::MaterialSettingsGUI()
{
	for (auto& material : m_Materials)
	{
		if (ImGui::TreeNode(material.first.c_str()))
		{
			material.second->SettingsGUI();
			ImGui::TreePop();
		}
	}
}

void MaterialLibrary::MaterialSelectGUI(Material** mat) const
{
	// get an array of all material names
	const char** matNames = new const char*[m_Materials.size()];

	const std::string& currentMatName = GetName(*mat);

	int i = 0, currentIndex = -1;
	for (auto& m : m_Materials)
	{
		matNames[i] = m.first.c_str();
		if (m.first == currentMatName)
			currentIndex = i;
		i++;
	}

	if (ImGui::Combo("Select Material", &currentIndex, matNames, static_cast<int>(m_Materials.size())))
	{
		std::string name(matNames[currentIndex]);
		*mat = GetMaterial(name);
	}

	delete[] matNames;
}

void MaterialLibrary::MaterialSelectGUI(std::vector<Material*>& mats) const
{
	const char** matNames = new const char* [m_Materials.size()];

	for (int i = 0; i < mats.size(); i++)
	{
		const std::string& currentMatName = GetName(mats[i]);

		int j = 0, currentIndex = -1;
		for (auto& m : m_Materials)
		{
			matNames[j] = m.first.c_str();
			if (m.first == currentMatName)
				currentIndex = j;
			j++;
		}

		char format[] = "Select Material %d";
		char label[20];
		sprintf_s(label, format, i);

		if (ImGui::Combo(label, &currentIndex, matNames, static_cast<int>(m_Materials.size())))
		{
			std::string name(matNames[currentIndex]);
			mats[i] = GetMaterial(name);
		}
	}

	delete[] matNames;
}
