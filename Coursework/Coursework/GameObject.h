#pragma once

#include "Transform.h"
#include "BaseMesh.h"
#include "Material.h"
#include "TerrainMesh.h"


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
};
