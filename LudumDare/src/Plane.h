#pragma once
#include <raylib.h>

#include "Types.h"
#include <vector>
#include <raymath.h>

struct Plane
{
	Model* model = nullptr;
	Matrix transform;
	Vector3 position = { 0,10, 0 };
	Vector3 velocity = { 0,0,-1 };
	Vector3 forward = { 0,0,-1 };
	Vector3 up = { 0,0,-1 };
	Vector3 target = { 0,0,0 };

	Vector3 rot = { 0,0,0 }; // yaw pitch roll
	
	eTeamTag tag;

	bool fire_left = true;
};

struct Missile
{
	Model* model = nullptr;
	Matrix transform;
	Vector3 position = { 0,10, 0 };
	Vector3 velocity = { 0,0,-1 };
	Vector3 forward = { 0,0,-1 };
	Vector3 up = { 0,0,-1 };

	eTeamTag tag;

	f32 hit_rad = 1.0f;
	f32 life = 1.f;

#if 0
	~Missile()
	{
		printf("Killed %i\n", this);
	}
#endif
};


void PlaneCameraFollow(Plane& follow_plane, Camera& cam)
{
	const Vector3 forward = Vector3Transform({ 0,0,-1 }, follow_plane.transform);
	const Vector3 up = Vector3Transform({ 0,1, 0 }, follow_plane.transform);

	cam.position = -8.0f * forward + (3.0f * up) + follow_plane.position;
	cam.target = follow_plane.position + (up * .75f);
	cam.up = up;
}

void PlaneDraw(Plane& plane, bool hasTranslation = false)
{
	plane.model->transform = plane.transform;
	DrawModel(*plane.model, hasTranslation ? Vector3Zero() : plane.position, 1.0f, WHITE);
}

void PlaneUpdate(Plane& plane, f32 sDT)
{
	const Vector3 forward = Vector3Transform({ 0,0,-1 }, plane.transform);
	const Vector3 up = Vector3Transform({ 0,1, 0 }, plane.transform);

	plane.forward = forward;
	plane.up = up;

	plane.position = plane.position + plane.forward * 7.0f * sDT;
}

void PlaneEnemyUpdate(Plane& player, std::vector<Plane>& storage, f32 sDT)
{
	for(auto& Enemy : storage)
	{
		Vector3 dir = Vector3Normalize(player.position - Enemy.position);

		Enemy.transform = MatrixInvert(MatrixLookAt(Vector3Zero(), dir, {0,1,0}));
		
		PlaneUpdate(Enemy, sDT);

		PlaneDraw(Enemy, false);
	}
}

void PopulateEnemyPlanes(std::vector<Plane>& storage, Model& model, u32 count)
{
	for( int i = 0; i < count; i++)
	{
		Plane new_plane;
		new_plane.position = { (f32)GetRandomValue(-100,100), (f32)GetRandomValue(10,25), (f32)GetRandomValue(-100,100) };
		new_plane.model = &model;
		new_plane.transform = MatrixIdentity();
		
		storage.push_back(new_plane);
	}
}

void PlaneFireMissle(Plane& plane, std::vector<Missile>& storage,  Model* model)
{
	Missile fired_missile;
	fired_missile.position = plane.position;
	fired_missile.forward = plane.forward;
	fired_missile.velocity = plane.velocity + plane.forward * 55.0f;
	fired_missile.up = plane.up;
	fired_missile.model = model;
	fired_missile.transform = plane.model->transform;

	storage.push_back(fired_missile);
}

