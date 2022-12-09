#pragma once

#include "Transform.h"
#include "BaseMesh.h"
#include "Material.h"


struct GameObject
{
	Transform transform;
	BaseMesh* mesh = nullptr;
	Material* material = nullptr;

	bool castsShadows = true;

	GameObject(BaseMesh* mesh, Material* mat)
	{
		this->mesh = mesh;
		this->material = mat;
	}
	GameObject(const XMFLOAT3& position, BaseMesh* mesh, Material* mat)
	{
		this->transform.SetTranslation(position);
		this->mesh = mesh;
		this->material = mat;
	}
};
