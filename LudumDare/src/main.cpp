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
	//BeginDrawing();
	//ClearBackground(BLUE);

	DrawText("Welcome to Mid-air Repair", 100, 100, 35, WHITE);
	DrawText("Use the mouse to steer the airplane.", 100, 130, 35, WHITE);
	DrawText("Use the mouse to solve puzzles.", 100, 160, 35, WHITE);
	DrawText("Every click costs health, so be careful.", 100, 190, 35, WHITE);
	DrawText("Solve puzzles to repair the plane.", 100, 220, 35, WHITE);
	DrawText("Puzzles consist of a series of dots, connected by lines.", 100, 250, 35, WHITE);
	DrawText("You must find the start of the path and click the dots in order.", 100, 280, 35, WHITE);
	DrawText("Some of the dots might be broken, don't let them trick you.", 100, 310, 35, WHITE);

	DrawText("Press ` to return to this menu", SCREEN_WIDTH / 3, SCREEN_HEIGHT / 6 * 5.f, 20, WHITE);
	DrawText("Press R to respawn your airplane", SCREEN_WIDTH / 3, SCREEN_HEIGHT / 6 * 5.f + 20, 20, WHITE);
	DrawText("Press space to shoot missiles (they don't do anything)", SCREEN_WIDTH / 3, SCREEN_HEIGHT / 6 * 5.f + 40, 20, WHITE);


	Rectangle back_rec = { 90, 395, 200, 50 };
	DrawRectangleRec(back_rec, GRAY);
	bool back_hover = CheckCollisionPointRec(GetMousePosition(), back_rec);
	DrawText("BACK", 100, 400, 45, back_hover ? RED : WHITE);
	if (back_hover && IsMouseButtonReleased(0)) gsCUR_SCREEN = eScreen::START;

	EndDrawing();
}

void Title(Model& plane, Camera& cam, Model& ground, Model& clouds, const Vector2& dm,f32 max_score, f32 sDT)
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
	t += sDT * 0.2f;
	
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

	if(gsCUR_SCREEN == eScreen::RULES)
	{
		Rules();
		return;;
	}

	Rectangle start_rec = {90, 195, 200, 50};
	DrawRectangleRec(start_rec, GRAY);
	bool start_hover = CheckCollisionPointRec(GetMousePosition(), start_rec);
	DrawText("START", 100, 200, 45, start_hover ? RED : WHITE);

	Rectangle rules_rec = { 90, 265, 340, 50 };
	DrawRectangleRec(rules_rec, GRAY);
	bool rules_hover = CheckCollisionPointRec(GetMousePosition(), rules_rec);
	DrawText("HOW TO PLAY", 100, 270, 45, rules_hover ? RED : WHITE);
	
	DrawText("Mid-air Repair", SCREEN_WIDTH / 3, SCREEN_HEIGHT / 6, 45, YELLOW);
	DrawText(FormatText("High Score:\n%.1f", max_score), SCREEN_WIDTH/6*5.0f, SCREEN_HEIGHT/16, 25, YELLOW);

	DrawText("Press ` to return to this menu", SCREEN_WIDTH / 3, SCREEN_HEIGHT / 6 * 5.f, 20, WHITE);
	
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
	Music Song_01 = LoadMusicStream((MUSIC_PATH + "Song_02.ogg").c_str());
	//PlayMusicStream(Song_01);
	
	// lighting / shader
	Shader light_shader = LoadShader("res/SHADERS/lighting.vs", "res/SHADERS/lighting.fs");
	light_shader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(light_shader, "matModel");
	light_shader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(light_shader, "viewPos");

	int ambientLoc = GetShaderLocation(light_shader, "ambient");
	float amb[4] = {
		0.42f, 0.42f, 0.42f, 1.0f
	};
	SetShaderValue(light_shader, ambientLoc, amb, UNIFORM_VEC4);
	
	Light lights[MAX_LIGHTS] = { 0 };
	lights[0] = CreateLight(LIGHT_DIRECTIONAL, { -3 ,2, -4 }, Vector3Zero(), WHITE, light_shader);

	// plane
	Texture Plane_Tex = LoadTexture((TEX_PATH + "Plane_Color_pallet.png").c_str());
	Model f20_plane_model = LoadModel((MESH_PATH + "f20_s.obj").c_str());

	f20_plane_model.materials[0].shader = light_shader;
	
	f20_plane_model.materials[0].maps[MAP_DIFFUSE].texture = Plane_Tex; //Color{ 255, 0, 0,255};
	f20_plane_model.materials[0].maps[MAP_DIFFUSE].color = Color{ 255, 255, 255,255 };
	
	Plane PlayerPlane;
	PlayerPlane.transform = MatrixIdentity();
	PlayerPlane.tag = eTeamTag::FRIENDLY;
	PlayerPlane.model = &f20_plane_model;

	// missile
	Model Missile_Model = LoadModel((MESH_PATH + "missile.obj").c_str());
	Missile_Model.materials[0].shader = light_shader;

	Missile_Model.materials[0].maps[MAP_DIFFUSE].texture = Plane_Tex;
	Missile_Model.materials[0].maps[MAP_DIFFUSE].color = Color{ 255, 255, 255,255 };

	//clouds
	Model Cloud_Model = LoadModel((MESH_PATH + "cloud_001.obj").c_str());
	Cloud_Model.materials[0].shader = light_shader;
	Cloud_Model.materials[0].maps[MAP_DIFFUSE].color = Color{ 255, 255, 255,255 };
	
	// ground
	// normals are broken, will work around
	GenImagePerlinNoise(100, 100, 0, 0, 1);
	Image Height_Map = GenImagePerlinNoise(100, 100, 0, 0, 1);
	Texture Height_Tex = LoadTextureFromImage(Height_Map);
	Model Ground_Model = LoadModel((MESH_PATH + "Ground_smoothed.obj").c_str());  //LoadModelFromMesh(GenMeshHeightmap(Height_Map, {100,18,100}));
	Ground_Model.materials[0].shader = light_shader;
	Ground_Model.materials[0].maps[MAP_DIFFUSE].color = Color{ 55, 255, 55,255 };
	Ground_Model.materials[0].maps[MAP_DIFFUSE].texture = Height_Tex;

	// base
	Model Base_Model = LoadModel((MESH_PATH + "buildings.obj").c_str());
	Texture Base_Tex = LoadTexture((TEX_PATH + "Building_Color_Pallet.png").c_str());
	Base_Model.materials[0].shader = light_shader;
	Base_Model.materials[0].maps[MAP_DIFFUSE].color = Color{ 255, 255, 255,255 };
	Base_Model.materials[0].maps[MAP_DIFFUSE].texture = Base_Tex;

	Model Pillar_Model = LoadModel((MESH_PATH + "Pillars.obj").c_str());
	Pillar_Model.materials[0].shader = light_shader;
	Pillar_Model.materials[0].maps[MAP_DIFFUSE].color = Color{ 155, 225, 55,255 };
	
	// buildings
	Model Building_Model = LoadModel((MESH_PATH + "building_01.obj").c_str());
	Texture Building_Tex = LoadTexture((TEX_PATH + "Building_Color_Pallet.png").c_str());
	Building_Model.materials[0].shader = light_shader;
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
	PopulateBuildings(Buildings, Building_Model, Ground_Model, 10);
	PopulateClouds(Clouds, Cloud_Model, 250);
	PopulateEnemyPlanes(Enemies, f20_plane_model, 100);

	Puzzle test_puzzle;
	MakePuzzle(test_puzzle, 10);
	
	//SetTargetFPS(60);
	//ToggleFullscreen();

	f32 Max_Score = 0.0f;
	
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

		Max_Score = std::max(PlayerPlane.score, Max_Score);
		// End Time

		if(gsCUR_SCREEN == eScreen::START || gsCUR_SCREEN == eScreen::RULES)
		{
			Title(f20_plane_model, sCamera, Ground_Model, Cloud_Model, delta_mouse,Max_Score, sDT);
			continue;
		}

#if 0
		if (gsCUR_SCREEN == eScreen::RULES)
		{
			Rules();
			continue;
		}
#endif

		// Death
		auto Out_Of_Bounds = [](const Vector3& pos)
		{
			return pos.x > 150.f || pos.z > 150.f || pos.x < -1150.f || pos.z < -1150.f;
		};
		if(PlayerPlane.health <= 0.f || IsKeyReleased('R') || Out_Of_Bounds(PlayerPlane.position))
		{
			PlaySound(Explo);
			test_puzzle = Puzzle();
			MakePuzzle(test_puzzle,10);
			
			PlayerPlane = Plane();
			PlayerPlane.transform = MatrixIdentity();
			PlayerPlane.position = { (f32)GetRandomValue(-900,50), (f32)GetRandomValue(10,25), (f32)GetRandomValue(-900,50) };
			PlayerPlane.tag = eTeamTag::FRIENDLY;
			PlayerPlane.model = &f20_plane_model;
			PlayerPlane.score = 0.0f;
		}

		static f32 smoke_timer = -.1;
		smoke_timer -= sDT;
		if(PlayerPlane.health <= 20.f && smoke_timer < .0f)
		{
			int r = GetRandomValue(0, Clouds.size());
			auto& cloud = Clouds[r];
			cloud.position = PlayerPlane.position;
			f32 rand_f = .2f + GetRandomValue(0, 100) / 100.f;
			cloud.transform = MatrixScale(rand_f, rand_f, rand_f) *
				MatrixRotateXYZ({ GetRandomValue(0, 100) / 100.f, GetRandomValue(0, 100) / 100.f, GetRandomValue(0, 100) / 100.f }) *
				MatrixIdentity();
			smoke_timer = .1f;
		}
		
		// Input
		constexpr f32 rot_speed = 3.f;

		PlayerPlane.rot.y += delta_mouse.x * sDT;
		PlayerPlane.rot.z += delta_mouse.x * sDT * 2.0f;
		PlayerPlane.rot.x += delta_mouse.y * sDT;

		if (IsMouseButtonPressed(0)) 
		{
			PlayerPlane.health -= 1;
			PlaySound(Hit);
		}
		
		if (IsKeyDown('`')) 
		{
			sCamera.up = { 0,1,0 };
			gsCUR_SCREEN = eScreen::START;
			continue;
		}
#if 0
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
#endif
		if (IsKeyDown(KEY_LEFT_SHIFT))
		{
			PlayerPlane.speed += sDT * 4.0f;
			//PlayerPlane.position = PlayerPlane.position + PlayerPlane.forward * 10.f * sDT;
		}
		if(IsKeyPressed(KEY_SPACE))
		{
			PlaySound(Shoot);
			PlaneFireMissle(PlayerPlane,PlayerMissles, &Missile_Model);
		}
		while (PlayerPlane.rot.x > 180) PlayerPlane.rot.z -= 360;
		while (PlayerPlane.rot.x < -180) PlayerPlane.rot.z += 360;
		
		PlayerPlane.transform = MatrixIdentity();
		
		PlayerPlane.transform = MatrixRotateZ(PlayerPlane.rot.z) * MatrixRotateX(PlayerPlane.rot.x) * MatrixRotateY(PlayerPlane.rot.y) * PlayerPlane.transform;
		//PlayerPlane.transform = MatrixRotateZ(PlayerPlane.rot.z) * PlayerPlane.transform;
		
		PlayerPlane.rot.z = Lerp(PlayerPlane.rot.z, 0.0f, 1.0f - powf(0.1f, sDT));
		PlayerPlane.rot.x = Lerp(PlayerPlane.rot.x, 0.0f, 1.0f - powf(0.15f, sDT));

		// End Input
		
		// Update

		PlaneUpdate(PlayerPlane, sDT);
		PlayerPlane.score += PlayerPlane.speed * sDT;

		if (PlayerPlane.position.y > 25.0f && PlayerPlane.rot.x < 0.0f)
		{
			PlayerPlane.rot.x = Lerp(PlayerPlane.rot.x, 0.0f, 1.0f - powf(0.15f, sDT));
		}
		
		UpdateCamera(&sCamera);
		PlaneCameraFollow(PlayerPlane, sCamera,sDT);
		float cameraPos[3] = { sCamera.position.x, sCamera.position.y, sCamera.position.z };
		SetShaderValue(light_shader, light_shader.locs[LOC_VECTOR_VIEW], cameraPos, UNIFORM_VEC3);

		// End Update
		
		// Rendering
		BeginDrawing();
		ClearBackground(BLUE);

		BeginMode3D(sCamera);

		//for (auto& missile : PlayerMissles)
		PlayerMissles.erase(std::remove_if(PlayerMissles.begin(), PlayerMissles.end(), [&Missile_Model, &Explo, &Ground_Model](auto& missile)
		{
			missile.position = missile.position + missile.velocity * sDT;
			Missile_Model.transform = missile.transform;
			DrawModel(*missile.model, missile.position, 1, WHITE);
			missile.life -= sDT;
			Ray trace = {};
			trace.position = missile.position;
			trace.direction = missile.forward;
			auto info = GetCollisionRayModel(trace, Ground_Model);
			//DrawRay(trace, BLUE);
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
				
			Ground_Model.transform = MatrixMultiply(x_flip, Ground_Model.transform);
			Ground_Model.transform = MatrixTranslate((f32)i * 200, 0, (f32)j * 200) * Ground_Model.transform;

			if (Vector3Distance({ (f32)-i * 200, 0, (f32)-j * 200 }, PlayerPlane.position) < 150.f)
			{
				auto info = GetCollisionRayModel(trace, Ground_Model);
				//DrawRay(trace, BLUE);
				if (info.hit)
				{
					DrawSphere(info.position, .1f, GRAY);
					if (info.distance < 1.20f)
					{
						PlayerPlane.position.y = info.position.y;
						PlayerPlane.health -= 10.0f;
					}
				}
				//DrawModelWires(Ground, Vector3Zero(), 1, BLACK);

			}
			
			DrawModel(Ground_Model, Vector3Zero(), 1, WHITE);
			DrawModelWires(Ground_Model, Vector3Zero(), 1, BLACK);
			Ground_Model.transform = MatrixIdentity();
		}
		Ground_Model.transform = MatrixIdentity();

		//DrawModel(Pillar_Model, Vector3Zero(), 1, WHITE);
		//DrawModel(Base_Model, Vector3Zero(), 1, WHITE);

#if 0
		for(auto& building : Buildings)
		{
			building.model->transform = building.transform;
			DrawModel(*building.model, building.position, 1.0f, WHITE);
			DrawBillboardRec(sCamera, blank, {0,0,building.health,10}, building.position + Vector3{ 0,4,0 }, 1.0f, RED);
		}
#endif
		
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

			cloud.model->transform = cloud.transform;
			DrawModel(*cloud.model, cloud.position, 1.0f, Color{ 255,255,255,55});//WHITE);
		}
		EndBlendMode();
		
		EndMode3D();

		DrawPuzzle(test_puzzle, PlayerPlane,Hit);
		
		DrawFPS(10,10);
		DrawText(FormatText("sDT: %f", sDT), 10, 40, 10, GRAY);
		DrawText(FormatText("Score:\n %.1f", PlayerPlane.score), SCREEN_WIDTH - 200, 40, 30, YELLOW);
		DrawText(FormatText("Speed: %.1f", PlayerPlane.speed), 10, 50, 30, BLACK);
		DrawText(FormatText("Pos: %.1f, %.1f, %.1f", PlayerPlane.position.x, PlayerPlane.position.y, PlayerPlane.position.z), 10, 70, 30, BLACK);
		DrawText(FormatText("Health: %.0f", PlayerPlane.health), SCREEN_WIDTH * 0.45f, SCREEN_HEIGHT * 0.8f, 30, BLACK);
		EndDrawing();

		// End Rendering

	}

	UnloadModel(f20_plane_model);
	UnloadModel(Building_Model);
	UnloadModel(Cloud_Model);
	UnloadModel(Ground_Model);
	UnloadModel(Pillar_Model);
	UnloadModel(Missile_Model);
	UnloadModel(Base_Model);

	UnloadTexture(Height_Tex);
	UnloadTexture(Plane_Tex);

	UnloadSound(Explo);
	UnloadSound(Shoot);

	UnloadShader(light_shader);
	return 0;
}