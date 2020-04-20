#pragma once
#include <vector>

#include <raylib.h>

#include "Types.h"

struct Puzzle
{
	std::vector<Vector2> points;
	std::vector<bool> checked;
	std::vector<bool> hidden;
	std::vector<bool> fake;
	u32 current = 0;
};

void MakePuzzle(Puzzle& puzzle, u32 count)
{
	int start_x = GetRandomValue((int)SCREEN_WIDTH * 0.25f, SCREEN_WIDTH * 0.75f); // GetRandomValue(0, SCREEN_WIDTH);
	int start_y = GetRandomValue((int)SCREEN_HEIGHT * 0.25f, SCREEN_HEIGHT * 0.75f); // GetRandomValue(0, SCREEN_HEIGHT);
	
	for ( int i = 0; i < count; i++)
	{
		start_x = std::clamp(start_x, (int)(0.25f * (f32)SCREEN_WIDTH), (int)(0.75f * (f32)SCREEN_WIDTH));
		start_y = std::clamp(start_y, (int)(0.25f * (f32)SCREEN_HEIGHT), (int)(0.75f * (f32)SCREEN_HEIGHT));
		
		puzzle.points.push_back({ (f32)start_x,(f32)start_y });
		puzzle.checked.push_back(false);
		puzzle.hidden.push_back(false);
		puzzle.fake.push_back(false);
		
		if(GetRandomValue(0,1))
		{
			start_x += GetRandomValue(-150, 150);
		}
		else
		{
			start_y += GetRandomValue(-150, 150);
		}
	}
}

void DrawPuzzle(Puzzle& puzzle, Plane& player, Sound& Hit_Sound)
{
	auto m = GetMousePosition();

	bool res = true;
	f32 res_timer = 0.0f;
	for(int i = 0; i < puzzle.points.size();i++)
	{
		auto& p = puzzle.points[i];
		auto& np = puzzle.points[i+1 < puzzle.points.size() ? i + 1 : i];

		bool checked = puzzle.checked[i];
		bool hidden = puzzle.hidden[i];
		
		bool hovering = CheckCollisionPointCircle(m, p, 10.f);
		
		DrawLine(p.x, p.y, np.x, np.y, RED);
		if(!hidden)
			DrawCircle(p.x, p.y, 10.f + sinf(res_timer), puzzle.fake[i] ? YELLOW : checked ? RED : (hovering ? BLUE : YELLOW));

		if (!checked && i == puzzle.current && hovering && IsMouseButtonPressed(0))
		{
			StopSound(Hit_Sound);
			puzzle.current++;
			puzzle.checked[i] = true;
			if (GetRandomValue(0, 4) == 2) puzzle.hidden[i] = true;
			if (GetRandomValue(0, 4) == 2) puzzle.fake[i] = true;
			player.score += 1;
		}

		res &= puzzle.checked[i];
	}
	if(res)
	{
		player.score += 10000.f;
		player.health = std::clamp(player.health + 35.0f, 0.0f, 100.f);
		player.speed += 10.f;
		puzzle = Puzzle();
		MakePuzzle(puzzle, 10);
	}
	
}
