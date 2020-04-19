#pragma once
#include <cstdint>
#include <raymath.h>

//Typedefs
using f32 = float;
using f64 = double;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

const std::string RES_PATH = "res";
const std::string TEX_PATH = RES_PATH + "/TEXTURES/";
const std::string MESH_PATH = RES_PATH + "/MODELS/";
const std::string SHADER_PATH = RES_PATH + "/SHADERS/";
const std::string MUSIC_PATH = RES_PATH + "/MUSIC/";

enum class eTeamTag : u32
{
	FRIENDLY, ENEMY, SIZE
};

constexpr f32 SCREEN_SCALE = 1.0f;// 1.5f * 2;
constexpr u32 SCREEN_WIDTH = 1280;// 640 * SCREEN_SCALE;
constexpr u32 SCREEN_HEIGHT = 840;// 480 * SCREEN_SCALE;

Vector3 inline operator+(const Vector3& a, const Vector3& b)
{
	return Vector3Add(a, b);
}

Vector3 inline operator-(const Vector3& a, const Vector3& b)
{
	return Vector3Subtract(a, b);
}

Vector3 inline operator*(const Vector3& a, f32 s)
{
	return Vector3Scale(a, s);
}

Matrix inline operator*(const Matrix& A, const Matrix& B)
{
	return MatrixMultiply(A, B);
}

Vector3 inline operator*(f32 s, const Vector3& a)
{
	return Vector3Scale(a, s);
}