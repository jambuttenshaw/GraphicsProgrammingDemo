#pragma once

#include "Transform.h"
#include "BaseMesh.h"
#include "Material.h"
#include "MaterialLibrary.h"
#include "TerrainMesh.h"
#include "ShaderUtility.h"

#include "imGUI/imgui.h"


struct GameObject
{
	enum class MeshType
	{
		Regular,
		Terrain
	};

	Transform transform;
	MeshType meshType = MeshType::Regular;
	union
	{
		BaseMesh* regular = nullptr;
		TerrainMesh* terrain;
	} mesh;

	std::vector<Material*> materials;

	bool castsShadows = true;

	GameObject() = default;
	GameObject(BaseMesh* mesh, Material* mat)
	{
		this->meshType = MeshType::Regular;
		this->mesh.regular = mesh;

		this->materials.push_back(mat);
	}
	GameObject(const XMFLOAT3& position, BaseMesh* mesh, Material* mat)
	{
		this->transform.SetTranslation(position);

		this->meshType = MeshType::Regular;
		this->mesh.regular = mesh;

		this->materials.push_back(mat);
	}
	GameObject(TerrainMesh* mesh, Material* mat)
	{
		this->meshType = MeshType::Terrain;
		this->mesh.terrain = mesh;

		this->materials.push_back(mat);
	}

	void AddMaterial(Material* mat)
	{
		materials.push_back(mat);
	}

	void SettingsGUI(const MaterialLibrary* materialLibrary)
	{
		ImGui::Text("Transform");
		transform.SettingsGUI();

		ImGui::Separator();

		ImGui::Text("Materials");
		materialLibrary->MaterialSelectGUI(materials);
	}
};
