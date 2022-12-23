#pragma once

#include "Transform.h"
#include "BaseMesh.h"
#include "Material.h"
#include "MaterialLibrary.h"
#include "TerrainMesh.h"

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
	Material* material = nullptr;

	bool castsShadows = true;

	GameObject() = default;
	GameObject(BaseMesh* mesh, Material* mat)
	{
		this->meshType = MeshType::Regular;
		this->mesh.regular = mesh;
		this->material = mat;
	}
	GameObject(const XMFLOAT3& position, BaseMesh* mesh, Material* mat)
	{
		this->meshType = MeshType::Regular;
		this->transform.SetTranslation(position);
		this->mesh.regular = mesh;
		this->material = mat;
	}
	GameObject(TerrainMesh* mesh, Material* mat)
	{
		this->meshType = MeshType::Terrain;
		this->mesh.terrain = mesh;
		this->material = mat;
	}

	void SettingsGUI(const MaterialLibrary* materialLibrary)
	{
		ImGui::Text("Transform");

		auto t = transform.GetTranslation();
		if (ImGui::DragFloat3("Position", &t.x, 0.01f))
			transform.SetTranslation(t);

		auto s = transform.GetScale();
		if (ImGui::DragFloat3("Scale", &s.x, 0.01f))
			transform.SetScale(s);

		ImGui::Separator();

		ImGui::Text("Material");
		materialLibrary->MaterialSelectGUI(&material);
	}
};
