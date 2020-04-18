#pragma once
#include <raylib.h>

#include "Types.h"

struct Cloud
{
	Model* model = nullptr;
	Vector3 position = { 0,10, 0 };
	Vector3 velocity = { 0,0,-1 };
};

void PopulateClouds(std::vector<Cloud>& storage, Model& model, u32 count)
{
	for( int i = 0; i < count; i++)
	{
		Cloud new_cloud = {};
		new_cloud.position = { (f32)GetRandomValue(-100,100), (f32)GetRandomValue(10,25), (f32)GetRandomValue(-100,100) };
		new_cloud.model = &model;
		storage.push_back(new_cloud);
	}
}