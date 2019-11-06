#pragma once

#include "opencv2/opencv.hpp"
#include <iostream>
#include <DirectXMath.h>
#include <stdlib.h>
#include <ppl.h>
#include <functional>
#include <random>

using namespace cv;
using namespace std;

constexpr static  int board_w = 80;
constexpr static  int board_h = 60;
constexpr static  int block_size = 16;
constexpr static  int big_border_size = 2;
constexpr static int small_border_size = 1;

constexpr static int FARM_LEVEL_MIN = 0;
constexpr static int FARM_LEVEL_MAX = 15;
constexpr static float FOOD_GAIN_PER_STEP = 6.6f;
constexpr static float FARM_CONSUME_MANPOWER_PER_STEP = 0.1f;
constexpr static float FARM_CONSUME_PRODUCT_PER_STEP = 9.f;
static float FARM_OUTPUT_FACTOR = 1;

constexpr static int BUILD_LEVEL_MIN = 0;
constexpr static int BUILD_LEVEL_MAX = 15;
constexpr static float PRODUCT_GAIN_PER_STEP = 0.0005f;
constexpr static float BUILD_CONSUME_MANPOWER_PER_STEP = 0.01f;
constexpr static float BUILD_CONSUME_PRODUCT_PER_STEP = 0.1f;

constexpr static int MAN_LEVEL_MIN = 0;
constexpr static int MAN_LEVEL_MAX = 1000;
constexpr static float MANPOWER_GAIN_PER_STEP = 0.01f;
constexpr static float MAN_CONSUME_FOOD_PER_STEP = 0.0005f;
constexpr static float MAN_UPGRADE_COST_FACTOR = 1.f;

constexpr static float INFO_FARM_POWER_DECREASE_PER_STEP = 0.1f;
constexpr static float INFO_BUILD_POWER_DECREASE_PER_STEP = 0.1f;
constexpr static float INFO_MAN_POWER_DECREASE_PER_STEP = 0.1f;
constexpr static float INFO_SPREAD_FACTOR_BUILD = 0.02f;

constexpr static int MAX_COUNTRY_COUNT = 80;

const static vector<string> Month_Name = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep","Oct", "Nov", "Dec" };

const std::vector<Vec3b> Geo_color = {
	Vec3b{  29, 230, 181},
	Vec3b{ 232, 162,   0},
	Vec3b{  87, 122, 185},
	Vec3b{  21,   0, 136},
	Vec3b{ 234, 217, 153},
	Vec3b{ 204,  72,  63},
	Vec3b{ 190, 146, 112},
	Vec3b{  76, 177,  34},
};

const std::vector<int> Geo_Food_Level = {
	2,
	0,
	1,
	0,
	0,
	0,
	0,
	7
};

const std::vector<float> Geo_Mho = {
	0.7f,
	0.97f,
	0.84f,
	0.f,
	0.995f,
	0.6f,
	0.72f,
	0.7f
};

const std::vector<bool> Geo_Livable = {
	true,
	true,
	true,
	false,
	false,
	false,
	false,
	true
};

const std::vector<bool> Geo_Accessable = {
	true,
	true,
	true,
	false,
	true,
	false,
	true,
	true
};

enum Geo {
	Grassland = 0,
	River,
	Mountain,
	Blocked,
	ShallowOcean,
	DeepOcean,
	MediumOcean,
	Farmland,
	Max
};

extern struct Block;

class Country
{
public:
	const int ID = 0;
	uint8_t r, g, b;
	int stat_demesne_size = 0;
	int stat_troop_size = 0;
	int stat_war_opponent = 0;
	int stat_war_opponent_troop_size = 0;
	int stat_sum_ruin = 0;
	float war_exp = 0;

	float trained_troop = 0;

	Block* capital = nullptr;
	int priority_rank[board_w][board_h] = { 0, };
	float priority_power[board_w][board_h] = { 0, };
	int priority_time[board_w][board_h] = { 0, };
	int distance_from_border[board_w][board_h] = { 0, };
	float capital_power[board_w][board_h] = { 0, };
	int access[board_w][board_h];
	bool isExist = true;

	Country(Country&&) = delete;
	Country(int ID, uint8_t R, uint8_t G, uint8_t B) : ID(ID), r(R), g(G), b(B) {}

	void ClearPriority() {
		for (int x = 0; x < board_w; ++x)
			for (int y = 0; y < board_h; ++y)
				priority_power[x][y] = 0;
	}
};

struct Troop {
	Troop(Troop&&) = delete;
	Troop(Country* owner) : owner(owner) {}

	Block* ground;
	Country* owner;
	int energy = 0;
	int size = 100;
};

class Block {
public:
	const int ID, X, Y;
	Rect rect;
	Geo geo;
	vector<Block*> neighbor;
	vector<Rect> small_border;
	vector<Rect> big_border;

	int farm_level = 1;
	int build_level = 0;
	int man_level = 0;

	float food = 0;
	float product = 0;
	float manpower = 1;

	int info_farm_rank = 0;
	float info_farm_power = 0;
	int info_build_rank = 0;
	float info_build_power = 0;
	int info_man_rank = 0;
	float info_man_power = 0;

	int info_loaf;

	int ruin = 0;
	

	Country* owner = nullptr;
	Troop* garrison = nullptr;

	Block(Block&&) = delete;
	Block(const int pos_x, const int pos_y) : ID(pos_x + pos_y * board_w), X(pos_x), Y(pos_y) {
	}
};

class Relation {
public:
	Country* const from = nullptr;
	Country* const to = nullptr;

	bool isNeighbor = false;
	bool isWar = false;
	bool isAccessable = false;
	int peace = 0;
	int WarDuration = 0;

	void Clear() {
		isNeighbor = false;
		isWar = false;
		isAccessable = false;
		peace = 0;
		WarDuration = 0;
	}

	Relation(Relation&&) = delete;
	Relation(Country* from, Country* to) : from(from), to(to) {
		Clear();
	}
};

inline float IntoRange(float var, float min, float max)
{
	return (var - min) / (max - min);
}

void Blend(Vec3f colorHigh, Vec3f colorLow, float var, float& r, float& g, float& b)
{
	r = colorHigh[0] * var + colorLow[0] * (1 - var);
	g = colorHigh[1] * var + colorLow[1] * (1 - var);
	b = colorHigh[2] * var + colorLow[2] * (1 - var);
}

inline const Vec3f Hex(const int& code)
{
	return Vec3f((code & 0xff0000) >> 16, (code & 0x00ff00) >> 8, code & 0x0000ff);
}