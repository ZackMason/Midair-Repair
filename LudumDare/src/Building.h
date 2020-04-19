#pragma once

#include "Types.h"
#include <vector>

struct Building
{
	Model* model;
	Matrix transform;
	Vector3 position;
	
	f32 health = 100.0f;
};

void PopulateBuildings(std::vector<Building>& storage, Model& model, Model& ground, u32 count)
{
	for (int i = 0; i < count; i++)
	{
		Building new_building = {};
		new_building.transform = MatrixIdentity();
		new_building.position = { (f32)GetRandomValue(-100,100), 0, (f32)GetRandomValue(-100,100) };

		// find height of terrain
		Ray ground_ray = {};
		ground_ray.position = new_building.position + Vector3{ 0,50,0 };
		ground_ray.direction = { 0,-1,0 };
		
		auto info = GetCollisionRayModel(ground_ray, ground);
		if (info.hit)
			new_building.position = info.position;

		new_building.model = &model;
		storage.push_back(new_building);
	}
}