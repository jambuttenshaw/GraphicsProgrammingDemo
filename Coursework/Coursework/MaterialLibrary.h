#pragma once

#include <string>
#include <unordered_map>

class Material;


class MaterialLibrary
{
public:
	MaterialLibrary() = default;
	~MaterialLibrary();

	Material* CreateMaterial(const std::string& name);
	Material* GetMaterial(const std::string& matName) const;
	const std::string GetName(Material* mat) const;

	void MaterialSettingsGUI();
	void MaterialSelectGUI(Material** mat) const;
	void MaterialSelectGUI(std::vector<Material*>& mats) const;

private:
	std::unordered_map<std::string, Material*> m_Materials;
};