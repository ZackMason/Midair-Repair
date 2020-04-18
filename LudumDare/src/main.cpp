//////////////////////////////////////
/// 4 - 17 - 2020
/// Ludum Dare 46
///
/// Keep it Alive
///
//////////////////////////////////////

// C/C++
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>

// Raylib
#include "raylib.h"
#include "raymath.h"


#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include "Cloud.h"
#include "Plane.h"
#include "glm/gtx/quaternion.hpp"

constexpr f32 SCREEN_SCALE = 1.5f;
constexpr u32 SCREEN_WIDTH = 640 * SCREEN_SCALE;
constexpr u32 SCREEN_HEIGHT = 480 * SCREEN_SCALE;

glm::vec3 v3(const Vector3& o)
{
	return { o.x,o.y,o.z};
}

glm::vec4 v4(const Vector3& o)
{
	return { o.x,o.y,o.z, 1.0f};
}

glm::mat4 m4(const Matrix& o)
{
	glm::mat4 res;
	int j = 0;
	for (int i = 0; i < 16; i+=4)
		res[j++] = { ((f32*)&o)[i],((f32*)&o)[i + 1],((f32*)&o)[i + 2],((f32*)&o)[i + 3] };
	return res;
}

Matrix m4(const glm::mat4& o)
{
	Matrix res;
	int j = 0;
	for (int i = 0; i < 16; i += 4)
	{
		memcpy(&((f32*)&res)[i], ((f32*)&o[j]), sizeof(float) * 4);
	}
	return res;
}

int main()
{
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Ludum Dare 46!");

	InitAudioDevice();
	
	
	static Camera sCamera = {};
	sCamera.fovy = 45;
	sCamera.up = { 0,1,0 };
	sCamera.type = CAMERA_PERSPECTIVE;

	SetCameraMode(sCamera, CAMERA_FREE);

	// sound
	Sound Explo = LoadSound((MUSIC_PATH + "Explosion.wav").c_str());
	Sound Hit = LoadSound((MUSIC_PATH + "Hit_Hurt.wav").c_str());
	Sound Shoot = LoadSound((MUSIC_PATH + "Laser_Shoot.wav").c_str());
	Music Song_01 = LoadMusicStream((MUSIC_PATH + "Song_01.mid").c_str());
	PlayMusicStream(Song_01);
	
	// lighting / shader
	Shader shader = LoadShader("res/SHADERS/base_lighting.vs", "res/SHADERS/lighting.fs");
	shader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
	shader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	int ambientLoc = GetShaderLocation(shader, "ambient");
	float amb[4] = {
		0.42f, 0.42f, 0.42f, 1.0f
	};
	SetShaderValue(shader, ambientLoc, amb, UNIFORM_VEC4);
	
	Light lights[MAX_LIGHTS] = { 0 };
	lights[0] = CreateLight(LIGHT_DIRECTIONAL, { -3 ,2, -4 }, Vector3Zero(), WHITE, shader);

	// plane
	Texture Plane_Tex = LoadTexture((TEX_PATH + "Plane_Color_pallet.png").c_str());
	Model f20_plane_model = LoadModel((MESH_PATH + "f20.obj").c_str());

	f20_plane_model.materials[0].shader = shader;
	
	f20_plane_model.materials[0].maps[MAP_DIFFUSE].texture = Plane_Tex; //Color{ 255, 0, 0,255};
	f20_plane_model.materials[0].maps[MAP_DIFFUSE].color = Color{ 255, 255, 255,255 };
	
	Plane PlayerPlane;
	PlayerPlane.transform = MatrixIdentity();
	PlayerPlane.tag = eTeamTag::FRIENDLY;
	PlayerPlane.model = &f20_plane_model;

	// missile
	Model missile_model = LoadModel((MESH_PATH + "missile.obj").c_str());
	missile_model.materials[0].shader = shader;

	missile_model.materials[0].maps[MAP_DIFFUSE].texture = Plane_Tex;
	missile_model.materials[0].maps[MAP_DIFFUSE].color = Color{ 255, 255, 255,255 };

	//clouds
	Model Cloud_Model = LoadModel((MESH_PATH + "cloud_001.obj").c_str());
	Cloud_Model.materials[0].shader = shader;
	Cloud_Model.materials[0].maps[MAP_DIFFUSE].color = Color{ 255, 255, 255,255 };
	
	// ground
	// normals are broken, will work around
	GenImagePerlinNoise(100, 100, 0, 0, 1);
	Image Height_Map = GenImagePerlinNoise(100, 100, 0, 0, 1);
	Texture Height_Tex = LoadTextureFromImage(Height_Map);
	Model Ground = LoadModel((MESH_PATH + "Ground_Base.obj").c_str());  //LoadModelFromMesh(GenMeshHeightmap(Height_Map, {100,18,100}));
	Ground.materials[0].shader = shader;
	Ground.materials[0].maps[MAP_DIFFUSE].color = Color{ 55, 255, 55,255 };
	Ground.materials[0].maps[MAP_DIFFUSE].texture = Height_Tex;

	// base
	Model Base_Model = LoadModel((MESH_PATH + "buildings.obj").c_str());
	Texture Base_Tex = LoadTexture((TEX_PATH + "Building_Color_Pallet.png").c_str());
	Base_Model.materials[0].shader = shader;
	Base_Model.materials[0].maps[MAP_DIFFUSE].color = Color{ 255, 255, 255,255 };
	Base_Model.materials[0].maps[MAP_DIFFUSE].texture = Base_Tex;
	
	static f64 sCur_Time = GetTime(); // in seconds
	static f64 sLast_Time = sCur_Time; // in seconds
	static f64 sDT = 0.0f; // in seconds

	std::vector<Missile> PlayerMissles;
	std::vector<Plane> Enemies;
	std::vector<Cloud> Clouds;
	PopulateClouds(Clouds, Cloud_Model, 250);
	PopulateEnemyPlanes(Enemies, f20_plane_model, 10);

	SetTargetFPS(60);
	
	while (!WindowShouldClose())
	{
		// Time
		sCur_Time = GetTime();
		sDT = sCur_Time - sLast_Time;
		// End Time
		
		// Input
		constexpr f32 rot_speed = 3.f;
		if (IsKeyDown('W'))
		{
			PlayerPlane.rot.x += sDT;
		}
		if (IsKeyDown('S'))
		{
			PlayerPlane.rot.x -= sDT;
		}
		if (IsKeyDown('A'))
		{
			PlayerPlane.rot.z -= sDT;
			PlayerPlane.rot.y -= sDT;
		}
		if (IsKeyDown('D'))
		{
			PlayerPlane.rot.z += sDT;
			PlayerPlane.rot.y += sDT;
		}
		if (IsKeyDown('Q'))
		{
			PlayerPlane.rot.z -= sDT;
		}
		if (IsKeyDown('E'))
		{
			PlayerPlane.rot.z += sDT;
		}
		if (IsKeyDown(KEY_LEFT_SHIFT))
		{
			PlayerPlane.position = PlayerPlane.position + PlayerPlane.forward * 10.f * sDT;
		}
		if(IsKeyPressed(KEY_SPACE))
		{
			PlaySound(Shoot);
			PlaneFireMissle(PlayerPlane,PlayerMissles, &missile_model);
		}
		while (PlayerPlane.rot.x > 180) PlayerPlane.rot.z -= 360;
		while (PlayerPlane.rot.x < -180) PlayerPlane.rot.z += 360;
		
		PlayerPlane.transform = MatrixIdentity();
		
		PlayerPlane.transform = MatrixRotateZ(PlayerPlane.rot.z) * MatrixRotateX(PlayerPlane.rot.x) * MatrixRotateY(PlayerPlane.rot.y) * PlayerPlane.transform;
		//PlayerPlane.transform = MatrixRotateZ(PlayerPlane.rot.z) * PlayerPlane.transform;
		
		PlayerPlane.rot.z = Lerp(PlayerPlane.rot.z, 0.0f, 1.0f - powf(0.1f,sDT));
		
		// End Input
		
		// Update

		PlaneUpdate(PlayerPlane, sDT);
		
		UpdateCamera(&sCamera);
		PlaneCameraFollow(PlayerPlane, sCamera);
		float cameraPos[3] = { sCamera.position.x, sCamera.position.y, sCamera.position.z };
		SetShaderValue(shader, shader.locs[LOC_VECTOR_VIEW], cameraPos, UNIFORM_VEC3);

		// End Update
		
		// Rendering
		BeginDrawing();
		ClearBackground(BLUE);

		BeginMode3D(sCamera);

		//for (auto& missile : PlayerMissles)
		PlayerMissles.erase(std::remove_if(PlayerMissles.begin(), PlayerMissles.end(), [&missile_model, &Explo, &Base_Model, &Ground](auto& missile)
		{
			missile.position = missile.position + missile.velocity * sDT;
			missile_model.transform = missile.transform;
			DrawModel(*missile.model, missile.position, 1, WHITE);
			missile.life -= sDT;
			Ray trace = {};
			trace.position = missile.position;
			trace.direction = missile.forward;
			auto info = GetCollisionRayModel(trace, Ground);
			DrawRay(trace, BLUE);
			if (info.hit && info.distance < 1.0f)
				missile.life = -1.0f;
			info = GetCollisionRayModel(trace, Base_Model);
			if (info.hit && info.distance < 1.0f)
				missile.life = -1.0f;
			
			if (missile.life < 0.0f) PlaySound(Explo);
			return missile.life < 0.0f;
		}),PlayerMissles.end());

		PlaneEnemyUpdate(PlayerPlane, Enemies, sDT);
		
		PlaneDraw(PlayerPlane);
		//DrawModel(*PlayerPlane.model, PlayerPlane.position, {1.0f}, WHITE);
		
		Ray trace = {};
		trace.position = PlayerPlane.position;
		trace.direction = PlayerPlane.forward;
		auto info = GetCollisionRayModel(trace, Ground);
		DrawRay(trace, BLUE);
		if(info.hit)
			DrawSphere(info.position, 1.f, YELLOW);
		
		DrawGrid(100, 1.0f);
		DrawModel(Ground, Vector3Zero(), 1, WHITE);
		DrawModel(Base_Model, Vector3Zero(), 1, WHITE);

		BeginBlendMode(BLEND_ADDITIVE);

		// sort for blending
		std::sort(Clouds.begin(), Clouds.end(), [&PlayerPlane](auto& a, auto& b) { return Vector3Distance(a.position, PlayerPlane.position) > Vector3Distance(b.position, PlayerPlane.position); });
		for (auto& cloud : Clouds)
		{
			DrawModel(*cloud.model, cloud.position, 1.0f, Color{ 255,255,255,55});//WHITE);
		}
		EndBlendMode();
		
		EndMode3D();
		
		DrawFPS(10,10);
		DrawText(FormatText("sDT: %f", sDT), 10, 40, 10, GRAY);
		EndDrawing();

		// End Rendering
		
		sLast_Time = sCur_Time;
	}

	return 0;
}