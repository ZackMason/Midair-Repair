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
#include "Building.h"
#include "Puzzles.h"

enum class eScreen : u32
{
	START, GAME, RULES, SIZE
};
static eScreen gsCUR_SCREEN = eScreen::START;


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

void Rules()
{
	BeginDrawing();
	ClearBackground(BLUE);

	DrawText("Welcome to Mid-air Repair", 100, 100, 35, BLACK);
	DrawText("Use the mouse to steer the airplane", 100, 130, 35, BLACK);
	DrawText("Use the mouse to solve puzzles", 100, 160, 35, BLACK);
	DrawText("Wait how does that work?..", 100, 190, 35, BLACK);


	Rectangle back_rec = { 90, 395, 200, 50 };
	DrawRectangleRec(back_rec, GRAY);
	bool back_hover = CheckCollisionPointRec(GetMousePosition(), back_rec);
	DrawText("BACK", 100, 400, 45, back_hover ? RED : WHITE);
	if (back_hover && IsMouseButtonReleased(0)) gsCUR_SCREEN = eScreen::START;

	EndDrawing();
}

void Title(Model& plane, Camera& cam, Model& ground, Model& clouds, const Vector2& dm, f32 sDT)
{
	if (IsKeyPressed(KEY_ENTER))
	{
		gsCUR_SCREEN = eScreen::GAME;
		return;
	}


	// Rendering
	BeginDrawing();
	ClearBackground(BLUE);

	static f32 t = 0.0f, phi = DEG2RAD * 80.f, rho = 2.50f;
	t += sDT;
	
	//t += dm.x / (f32)SCREEN_WIDTH;
	//phi += dm.y / (f32)SCREEN_HEIGHT;

	Vector3 pos = { cosf(t) * sinf(phi) * rho, cosf(phi)* rho, sinf(t) * sinf(phi)* rho };

	//SetCameraMode(cam, CAMERA_ORBITAL);
	//UpdateCamera(&cam);
	cam.position = pos;
	cam.target = { 0,0,0 };


	BeginMode3D(cam);

	plane.transform = MatrixIdentity();
	ground.transform = MatrixIdentity();
	clouds.transform = MatrixIdentity();


	DrawModel(plane, Vector3Zero(), 1.0f, WHITE);
	DrawModel(ground, { 0,-3.5,0 }, 1.0f, WHITE);

	EndMode3D();

	Rectangle start_rec = {90, 195, 200, 50};
	DrawRectangleRec(start_rec, GRAY);
	bool start_hover = CheckCollisionPointRec(GetMousePosition(), start_rec);
	DrawText("START", 100, 200, 45, start_hover ? RED : WHITE);

	Rectangle rules_rec = { 90, 265, 200, 50 };
	DrawRectangleRec(rules_rec, GRAY);
	bool rules_hover = CheckCollisionPointRec(GetMousePosition(), rules_rec);
	DrawText("RULES", 100, 270, 45, rules_hover ? RED : WHITE);
	
	DrawText("Mid-air Repair", SCREEN_WIDTH/3, SCREEN_HEIGHT/6, 45, YELLOW);

	if (start_hover && IsMouseButtonReleased(0)) gsCUR_SCREEN = eScreen::GAME;
	if (rules_hover && IsMouseButtonReleased(0)) gsCUR_SCREEN = eScreen::RULES;
	
	EndDrawing();
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

	SetCameraMode(sCamera, CAMERA_CUSTOM);
	//SetCameraMode(sCamera, CAMERA_ORBITAL);

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
	Model f20_plane_model = LoadModel((MESH_PATH + "f20_s.obj").c_str());

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
	Model Ground = LoadModel((MESH_PATH + "Ground_smoothed.obj").c_str());  //LoadModelFromMesh(GenMeshHeightmap(Height_Map, {100,18,100}));
	Ground.materials[0].shader = shader;
	Ground.materials[0].maps[MAP_DIFFUSE].color = Color{ 55, 255, 55,255 };
	Ground.materials[0].maps[MAP_DIFFUSE].texture = Height_Tex;

	// base
	Model Base_Model = LoadModel((MESH_PATH + "buildings.obj").c_str());
	Texture Base_Tex = LoadTexture((TEX_PATH + "Building_Color_Pallet.png").c_str());
	Base_Model.materials[0].shader = shader;
	Base_Model.materials[0].maps[MAP_DIFFUSE].color = Color{ 255, 255, 255,255 };
	Base_Model.materials[0].maps[MAP_DIFFUSE].texture = Base_Tex;

	Model Pillar_Model = LoadModel((MESH_PATH + "Pillars.obj").c_str());
	Pillar_Model.materials[0].shader = shader;
	Pillar_Model.materials[0].maps[MAP_DIFFUSE].color = Color{ 155, 225, 55,255 };
	
	// buildings
	Model Building_Model = LoadModel((MESH_PATH + "building_01.obj").c_str());
	Texture Building_Tex = LoadTexture((TEX_PATH + "Building_Color_Pallet.png").c_str());
	Building_Model.materials[0].shader = shader;
	Building_Model.materials[0].maps[MAP_DIFFUSE].color = Color{ 255, 255, 255,255 };
	Building_Model.materials[0].maps[MAP_DIFFUSE].texture = Base_Tex;

	// images
	Texture blank = LoadTexture((TEX_PATH + "white.png").c_str());
	
	static f64 sCur_Time = GetTime(); // in seconds
	static f64 sLast_Time = sCur_Time; // in seconds
	static f64 sDT = 0.0f; // in seconds

	std::vector<Missile> PlayerMissles;
	std::vector<Plane> Enemies;
	std::vector<Cloud> Clouds;
	std::vector<Building> Buildings;
	PopulateBuildings(Buildings, Building_Model, Ground, 10);
	PopulateClouds(Clouds, Cloud_Model, 250);
	PopulateEnemyPlanes(Enemies, f20_plane_model, 10);

	Puzzle test_puzzle;
	MakePuzzle(test_puzzle, 10);
	
	//SetTargetFPS(60);
	//ToggleFullscreen();


	auto last_mouse = GetMousePosition();
	auto cur_mouse = GetMousePosition();
	auto delta_mouse = Vector2Subtract(last_mouse, cur_mouse);
	while (!WindowShouldClose())
	{
		// Time
		sCur_Time = GetTime();
		sDT = sCur_Time - sLast_Time;
		cur_mouse = GetMousePosition();
		delta_mouse = Vector2Subtract({SCREEN_WIDTH/2.f,SCREEN_HEIGHT/2.f}, cur_mouse);
		delta_mouse.x = delta_mouse.x / (f32)SCREEN_WIDTH * -1.5f;
		delta_mouse.y = delta_mouse.y / (f32)SCREEN_HEIGHT * 1.5f;

		last_mouse = cur_mouse;
		sLast_Time = sCur_Time;
		// End Time

		if(gsCUR_SCREEN == eScreen::START)
		{
			Title(f20_plane_model, sCamera, Ground, Cloud_Model, delta_mouse,sDT);
			continue;
		}

		if (gsCUR_SCREEN == eScreen::RULES)
		{
			Rules();
			continue;
		}
		
		if(PlayerPlane.health <= 0.f)
		{
			PlaySound(Explo);
			test_puzzle = Puzzle();
			MakePuzzle(test_puzzle,10);
			
			PlayerPlane = Plane();
			PlayerPlane.transform = MatrixIdentity();
			PlayerPlane.tag = eTeamTag::FRIENDLY;
			PlayerPlane.model = &f20_plane_model;
		}
		// Input
		constexpr f32 rot_speed = 3.f;

		PlayerPlane.rot.y += delta_mouse.x * sDT;
		PlayerPlane.rot.z += delta_mouse.x * sDT * 2.0f;
		PlayerPlane.rot.x += delta_mouse.y * sDT;

		if (IsMouseButtonPressed(0)) PlayerPlane.health -= 1;

		if (IsKeyDown('`')) 
		{
			sCamera.up = { 0,1,0 };
			gsCUR_SCREEN = eScreen::START;
			continue;
		}
		
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
			PlayerPlane.rot.z -= sDT * 2.0f;
			PlayerPlane.rot.y -= sDT;
		}
		if (IsKeyDown('D'))
		{
			PlayerPlane.rot.z += sDT * 2.0f;
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
			PlayerPlane.speed += sDT * 4.0f;
			//PlayerPlane.position = PlayerPlane.position + PlayerPlane.forward * 10.f * sDT;
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
		
		PlayerPlane.rot.z = Lerp(PlayerPlane.rot.z, 0.0f, 1.0f - powf(0.1f, sDT));
		PlayerPlane.rot.x = Lerp(PlayerPlane.rot.x, 0.0f, 1.0f - powf(0.15f,sDT));
		
		// End Input
		
		// Update

		PlaneUpdate(PlayerPlane, sDT);
		
		UpdateCamera(&sCamera);
		PlaneCameraFollow(PlayerPlane, sCamera,sDT);
		float cameraPos[3] = { sCamera.position.x, sCamera.position.y, sCamera.position.z };
		SetShaderValue(shader, shader.locs[LOC_VECTOR_VIEW], cameraPos, UNIFORM_VEC3);

		// End Update
		
		// Rendering
		BeginDrawing();
		ClearBackground(BLUE);

		BeginMode3D(sCamera);

		//for (auto& missile : PlayerMissles)
		PlayerMissles.erase(std::remove_if(PlayerMissles.begin(), PlayerMissles.end(), [&missile_model, &Explo, &Ground](auto& missile)
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
			
			if (missile.life < 0.0f) PlaySound(Explo);
			return missile.life < 0.0f;
		}),PlayerMissles.end());

		PlaneEnemyUpdate(PlayerPlane, Enemies, Buildings, sDT);
		
		PlaneDraw(PlayerPlane);
		//DrawModel(*PlayerPlane.model, PlayerPlane.position, {1.0f}, WHITE);

		DrawGrid(100, 1.0f);
		Ray trace = {};
		trace.position = PlayerPlane.position;
		trace.position.y += 1.0f;
		trace.direction = { 0,-1,0 }; //PlayerPlane.forward;
		

		for (int j = 0; j < 6; j++)
			for(int i = 0; i < 6; i++)
		{
			static Matrix x_flip = MatrixScale(-1,1,-1);//MatrixRotateY(DEG2RAD * 180.0f);
				
			Ground.transform = MatrixMultiply(x_flip, Ground.transform);
			Ground.transform = MatrixTranslate((f32)i * 200, 0, (f32)j * 200) * Ground.transform;

			if (Vector3Distance({ (f32)-i * 200, 0, (f32)-j * 200 }, PlayerPlane.position) < 150.f)
			{
				auto info = GetCollisionRayModel(trace, Ground);
				//DrawRay(trace, BLUE);
				if (info.hit)
				{
					DrawSphere(info.position, .1f, YELLOW);
					if (info.distance < 1.50f)
					{
						PlayerPlane.position.y = info.position.y;
						PlayerPlane.health -= 10.0f;
					}
				}
				//DrawModelWires(Ground, Vector3Zero(), 1, BLACK);

			}
			
			DrawModel(Ground, Vector3Zero(), 1, WHITE);
			DrawModelWires(Ground, Vector3Zero(), 1, BLACK);
			Ground.transform = MatrixIdentity();
		}
		Ground.transform = MatrixIdentity();

		DrawModel(Pillar_Model, Vector3Zero(), 1, WHITE);
		//DrawModel(Base_Model, Vector3Zero(), 1, WHITE);

		for(auto& building : Buildings)
		{
			building.model->transform = building.transform;
			DrawModel(*building.model, building.position, 1.0f, WHITE);
			DrawBillboardRec(sCamera, blank, {0,0,building.health,10}, building.position + Vector3{ 0,4,0 }, 1.0f, RED);
		}
		
		BeginBlendMode(BLEND_ADDITIVE);
		// sort for blending
		std::sort(Clouds.begin(), Clouds.end(), [&PlayerPlane](auto& a, auto& b) { return Vector3Distance(a.position, PlayerPlane.position) > Vector3Distance(b.position, PlayerPlane.position); });
		for (auto& cloud : Clouds)
		{
			if(Vector3Distance(cloud.position, PlayerPlane.position) > 380.f)
			{
				cloud.position = PlayerPlane.position + Vector3{ (f32)GetRandomValue(-300,300), 0., (f32)GetRandomValue(-300,300) };
				cloud.position.y = (f32)GetRandomValue(10, 25);
				cloud.velocity = Vector3{ (f32)GetRandomValue(-3,3), 0., (f32)GetRandomValue(-3,3) };
			}
			cloud.position = cloud.position + cloud.velocity * sDT;
			
			DrawModel(*cloud.model, cloud.position, 1.0f, Color{ 255,255,255,55});//WHITE);
		}
		EndBlendMode();
		
		EndMode3D();

		DrawPuzzle(test_puzzle, PlayerPlane);
		
		DrawFPS(10,10);
		DrawText(FormatText("sDT: %f", sDT), 10, 40, 10, GRAY);
		DrawText(FormatText("Speed: %.1f", PlayerPlane.speed), 10, 50, 30, RED);
		DrawText(FormatText("Health: %.0f", PlayerPlane.health), 10, 80, 30, RED);
		EndDrawing();

		// End Rendering

	}

	return 0;
}