#pragma once
#include <raylib.h>

#include "Types.h"
#include <vector>
#include <raymath.h>
#include "Building.h"

struct Plane
{
	Model* model = nullptr;
	Matrix transform;
	Vector3 position = { -200,10, -200 };
	Vector3 velocity = { 0,0,-1 };
	Vector3 forward = { 0,0,-1 };
	Vector3 up = { 0,0,-1 };
	Vector3 target = { 0,0,0 };

	f32 health = 100.f;
	f32 score = 0.0f;
	
	u32 target_index = 0;
	f32 target_time = 10.f;
	f32 flee_time = 3.0f;
	f32 speed = 7.0f;

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


void PlaneCameraFollow(Plane& follow_plane, Camera& cam, f32 sDT)
{
	const Vector3 forward = Vector3Transform({ 0,0,-1 }, follow_plane.transform);
	const Vector3 up = Vector3Transform({ 0,1, 0 }, follow_plane.transform);

	cam.position = -8.0f * forward + (3.0f * up) + follow_plane.position;
	cam.target = follow_plane.position + (up * .75f) + follow_plane.forward;
	cam.up.x = Lerp(cam.up.x, up.x, 1.0f - powf(0.1f, sDT));
	cam.up.y = Lerp(cam.up.y, up.y, 1.0f - powf(0.1f, sDT));
	cam.up.z = Lerp(cam.up.z, up.z, 1.0f - powf(0.1f, sDT));
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

	plane.health -= sDT * 2.0f;
	
	plane.speed -= (plane.forward.y<0.f ? 2.0f : 1.0f) * (plane.forward.y * 20.0f) * sDT;
	plane.speed = std::clamp(plane.speed, 10.f, 55.0f);

	plane.position = plane.position + plane.forward * plane.speed * sDT;
}

void PlaneEnemyUpdate(Plane& player, std::vector<Plane>& storage, std::vector<Building>& buildings, f32 sDT)
{
	for (auto& Enemy : storage)
	{
		Enemy.target_time -= sDT;
		Enemy.flee_time -= sDT;

		//Enemy.target = player.position;
	
		Vector3 dir = Vector3Normalize(Enemy.target - Enemy.position) * Enemy.speed;
		if (Vector3Distance(Enemy.target, Enemy.position) < 4.0f)
		{
			Enemy.target = { (f32)GetRandomValue(-900,100), (f32)GetRandomValue(10,25), (f32)GetRandomValue(-900,100) };
			Enemy.flee_time = 3.0f;
		}
		if (Enemy.flee_time > 0.0f)
		{
			dir = dir * -1.0f;
		}

		//DrawSphere(Enemy.target, 0.5f, RED);
		
		Enemy.velocity = sDT * (dir - Enemy.velocity) + Enemy.velocity;
		
		//Enemy.transform = MatrixRotateZ(Enemy.rot.z) * MatrixRotateX(Enemy.rot.x) * MatrixRotateY(Enemy.rot.y);
		
		Enemy.transform = MatrixInvert(MatrixLookAt(Vector3Zero(), Vector3Normalize(Enemy.velocity), {0,1,0}));
		
		PlaneUpdate(Enemy, sDT);

		PlaneDraw(Enemy, false);
	}
}

void PopulateEnemyPlanes(std::vector<Plane>& storage, Model& model, u32 count)
{
	for( int i = 0; i < count; i++)
	{
		Plane new_plane;
		new_plane.position = { (f32)GetRandomValue(-900,100), (f32)GetRandomValue(10,25), (f32)GetRandomValue(-900,100) };
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
	fired_missile.velocity = plane.velocity + plane.forward * (plane.speed + 40.f);
	fired_missile.up = plane.up;
	fired_missile.model = model;
	fired_missile.transform = plane.model->transform;

	storage.push_back(fired_missile);
}

