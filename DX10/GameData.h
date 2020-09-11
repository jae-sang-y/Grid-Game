#pragma once

#include <list>
#include <DirectXMath.h>
#include <iostream>
#include <DirectXMath.h>
#include <stdlib.h>
#include <ppl.h>
#include <functional>
#include <random>
#include <algorithm>
#include <map>

#undef max
#undef min

typedef std::wstring String;
const String MAP_PATH = L"res/map.bmp";
constexpr size_t MAP_W = 160;
constexpr size_t MAP_H = 120;

using namespace DirectX;

constexpr static float THIN_BORDER_SIZE = 0.8f;
constexpr static float BOLD_BORDER_SIZE = 0.6f;

constexpr static int FARM_LEVEL_MAX = 127;
constexpr static int BUILDING_LEVEL_MAX = 15;
constexpr static int MAN_LEVEL_MAX = 1000000;

constexpr static int ROAD_LEVEL_MIN = 1;
constexpr static int ROAD_LEVEL_MAX = 63;

constexpr static float SPOIL_PROPORTION = 1 / 3.f;

constexpr static int MAX_NATION_COUNT = 80;
constexpr static float MAN_CONSUME_PER_MAN = 1 / 180.f;

const static XMVECTOR MAP_LENS_FARM_MIN = XMVectorSet(203, 234, 229, 0);
const static XMVECTOR MAP_LENS_FARM_MAX = XMVectorSet(0, 69, 28, 0);
const static XMVECTOR MAP_LENS_BUILDING_MIN = XMVectorSet(255, 234, 204, 0);
const static XMVECTOR MAP_LENS_BUILDING_MAX = XMVectorSet(213, 122, 8, 0);

const static XMVECTOR MAP_LENS_HSV_MIN = XMVectorSet(4 / 360.f, 0.84f, 0.90f, 0);
const static XMVECTOR MAP_LENS_HSV_MAX = XMVectorSet(329 / 360.f, 0.84f, 0.90f, 0);

const static XMFLOAT3 MAP_LENS_ROAD_MIN = { 0 / 360.f, 190, 118 };
const static XMFLOAT3 MAP_LENS_ROAD_MAX = { 337 / 360.f, 108, 111 };

const static std::vector<std::string> Month_Name = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep","Oct", "Nov", "Dec" };

struct Rect {
	XMFLOAT2 pos{};
	XMFLOAT2 size{};
	Rect() {}
	Rect(XMFLOAT2 pos, XMFLOAT2 size) : pos(pos), size(size) {}
};

enum class Geo : int {
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
struct GeoDesc {
	XMINT3 color = {};
	int food_level = {};
	float mho = 0.f;
	bool livable = false;
	bool passable = false;

	void SetMhoByHalftime(int x) {
		if (x == 0) mho = 0;
		mho = 1 / powf(2, 1.f / x);
	}
};
static std::map<Geo, GeoDesc> geo_descs = {};
const static std::list<Geo> geo_list = {
	Geo::Grassland,
	Geo::River,
	Geo::Mountain,
	Geo::Blocked,
	Geo::ShallowOcean,
	Geo::DeepOcean,
	Geo::MediumOcean,
	Geo::Farmland,
};

enum class Forward : int {
	None,
	N, E, W, S
};

const std::map<Forward, XMINT2> forward_dir = {
	{Forward::E, XMINT2{+1, 00}},
	{Forward::N, XMINT2{00, -1}},
	{Forward::W, XMINT2{-1, 00}},
	{Forward::S, XMINT2{00, +1}}
};

void SetGeoInfo() {
	{
		geo_descs[Geo::Grassland].color = { 29, 230, 181 };
		geo_descs[Geo::Grassland].food_level = 8;
		geo_descs[Geo::Grassland].SetMhoByHalftime(20);
		geo_descs[Geo::Grassland].livable = true;
		geo_descs[Geo::Grassland].passable = true;
	}
	{
		geo_descs[Geo::River].color = { 232, 162,   0 };
		geo_descs[Geo::River].food_level = 0;
		geo_descs[Geo::River].SetMhoByHalftime(50);
		geo_descs[Geo::River].livable = false;
		geo_descs[Geo::River].passable = true;
	}
	{
		geo_descs[Geo::Mountain].color = { 87, 122, 185 };
		geo_descs[Geo::Mountain].food_level = 3;
		geo_descs[Geo::Mountain].SetMhoByHalftime(10);
		geo_descs[Geo::Mountain].livable = true;
		geo_descs[Geo::Mountain].passable = true;
	}
	{
		geo_descs[Geo::Blocked].color = { 21,   0, 136 };
		geo_descs[Geo::Blocked].food_level = 0;
		geo_descs[Geo::Blocked].SetMhoByHalftime(0);
		geo_descs[Geo::Blocked].livable = false;
		geo_descs[Geo::Blocked].passable = false;
	}
	{
		geo_descs[Geo::ShallowOcean].color = { 234, 217, 153 };
		geo_descs[Geo::ShallowOcean].food_level = 0;
		geo_descs[Geo::ShallowOcean].SetMhoByHalftime(200);
		geo_descs[Geo::ShallowOcean].livable = false;
		geo_descs[Geo::ShallowOcean].passable = true;
	}
	{
		geo_descs[Geo::DeepOcean].color = { 204,  72,  63 };
		geo_descs[Geo::DeepOcean].food_level = 0;
		geo_descs[Geo::DeepOcean].SetMhoByHalftime(50);
		geo_descs[Geo::DeepOcean].livable = false;
		geo_descs[Geo::DeepOcean].passable = true;
	}
	{
		geo_descs[Geo::MediumOcean].color = { 190, 146, 112 };
		geo_descs[Geo::MediumOcean].food_level = 0;
		geo_descs[Geo::MediumOcean].SetMhoByHalftime(100);
		geo_descs[Geo::MediumOcean].livable = false;
		geo_descs[Geo::MediumOcean].passable = true;
	}
	{
		geo_descs[Geo::Farmland].color = { 76, 177,  34 };
		geo_descs[Geo::Farmland].food_level = 10;
		geo_descs[Geo::Farmland].SetMhoByHalftime(4);
		geo_descs[Geo::Farmland].livable = true;
		geo_descs[Geo::Farmland].passable = true;
	}
}

struct BlockPropaganda {
	enum class Rank {
		None,

		Farm,
		Man,
		Build


	} rank = Rank::None;
	float power = 0.f;

	BlockPropaganda(Rank rank, float power) : rank(rank), power(power) {}
	BlockPropaganda() : BlockPropaganda(BlockPropaganda::Rank::None, 0.f) {}
};

bool operator > (BlockPropaganda& self, BlockPropaganda other) { return (self.rank > other.rank || (self.rank == other.rank && self.power > other.power)) && self.power > 0; }

BlockPropaganda& operator << (BlockPropaganda& self, BlockPropaganda other) {
	if (other > self) {
		self.rank = other.rank;
		self.power = other.power;
	}
	return self;
}

BlockPropaganda operator - (BlockPropaganda& self, float decrement) {
	BlockPropaganda new_obj = self;
	new_obj.power -= decrement;
	return new_obj;
}

BlockPropaganda operator -= (BlockPropaganda& self, float decrement) {
	self.power -= decrement;
	return self;
}

struct NationalStatistics {
	int demesne_size = 0;
	int land_army_units_size = 0;
};

struct StrategyBlock {
	int rank = 0;
	float power = 0.f;
	int lifetime = 0;
	int distnace_from_border = 0;
	float capital_power = 0.f;
	bool passable = false;
};

struct Block;
struct Nation
{
	int ID = -1;
	XMFLOAT4 color = {};
	Block* capital = nullptr;

	bool willFall = false;

	Nation() {}
	Nation(XMFLOAT4 color) : color(color) {
		static int id_auto_increment = 0;
		ID = ++id_auto_increment;
	}

	void UpdateStrategyMap() {};
	void UpdateStat() {};
private:
	StrategyBlock strategy_map[MAP_W][MAP_H] = {};
	NationalStatistics stat = {};
};

struct Troop {
	Troop(Troop&&) = delete;
	Troop(Nation* owner) : owner(owner) {}

	Block* ground;
	Nation* owner;
	int energy = 0;
	int size = 100;
	Forward forward = Forward::None;
};

struct Block {
	int ID = -1, X = -1, Y = -1;
	Geo geo = Geo::Blocked;
	XMFLOAT4 color = {};
	const GeoDesc * geo_desc = nullptr;
	std::map<Forward, Block*> neighbors = {};
	std::map<Forward, Rect> thin_border = {};
	std::map<Forward, Rect> bold_border = {};
	Rect rect{};

	int farm_level = 0;
	int building_level = 0;
	int man_level = 0;
	int road_level = ROAD_LEVEL_MIN;

	float food = 100;
	float product = 1;
	float manpower = 1;
	float road_growth = 0.f;

	float farm_growth = 0.f;
	float build_growth = 0.f;

	BlockPropaganda farm_info = {};
	BlockPropaganda build_info = {};
	BlockPropaganda man_info = {};

	Nation* owner = nullptr;
	Troop* garrison = nullptr;

	Block(const int pos_x, const int pos_y) : ID(pos_x + pos_y * MAP_W), X(pos_x), Y(pos_y) {}
	Block() {}

	float Appraise_1() const {
		static float building_proportion = 1.f / BUILDING_LEVEL_MAX;
		static float farm_proportion = 1.f / FARM_LEVEL_MAX;
		static float man_proportion = 1.f / MAN_LEVEL_MAX;

		return building_level * building_proportion
			+ farm_level * farm_proportion +
			man_level * man_proportion;
	}

	void ChangeOwnerByContract(Nation*);
	void ChangeOwnerByForce(Nation*);

	void OnUpdateInfo() {
		static float building_proportion = 1.f / BUILDING_LEVEL_MAX;
		static float farm_proportion = 1.f / FARM_LEVEL_MAX;
		static float man_proportion = 1.f / MAN_LEVEL_MAX;

		farm_info -= 1.f;
		farm_info << BlockPropaganda(BlockPropaganda::Rank::Farm, powf(farm_level * farm_proportion, 2));

		build_info -= 1.f;
		build_info << BlockPropaganda(BlockPropaganda::Rank::Build, powf(building_level * building_proportion, 2));

		man_info -= 1.f;
		man_info << BlockPropaganda(BlockPropaganda::Rank::Man, powf(man_proportion * man_proportion, 2));
	}

	void OnPropaganda(Block* other) {
		float moving_limit = (this->GetThroughOut() + other->GetThroughOut()) / 2.f;
		if (this->farm_info > other->farm_info) {
			float moving_amount = other->food;
			moving_amount = std::min({
				moving_amount,
				moving_limit,
				this->GetAvailableSpace()
				});
			other->manpower -= moving_amount;
			this->manpower += moving_amount;
			road_growth += 0.001f * moving_amount;
		}
		if (this->build_info > other->build_info) {
			float moving_amount = other->food;
			moving_amount = std::min({
				moving_amount,
				moving_limit,
				this->GetAvailableSpace()
				});
			other->manpower -= moving_amount;
			this->manpower += moving_amount;
			road_growth += 0.001f * moving_amount;
		}
		if (this->man_info > other->man_info) {
			{
				float moving_amount = other->food;
				moving_amount = std::min({
					moving_amount,
					moving_limit,
					this->GetAvailableSpace()
					});
				other->food -= moving_amount;
				this->food += moving_amount;
				road_growth += 0.001f * moving_amount;
			}
			{
				float moving_amount = other->product;
				moving_amount = std::min({
					moving_amount,
					moving_limit,
					this->GetAvailableSpace()
					});
				other->product -= moving_amount;
				this->product += moving_amount;
				road_growth += 0.001f * moving_amount;
			}
		}
	}

	void OnStep() {
		float farm_growth_proprotion = std::min({ manpower, product }) * (1.f - food / (food + product + man_level)) / 3.f;

		farm_growth += farm_growth_proprotion;
		manpower -= farm_growth_proprotion;
		product -= farm_growth_proprotion;

		float build_growth_proprotion = std::min({ manpower, product }) *  (1.f - product / (food + product + man_level)) / 3.f;
		build_growth += build_growth_proprotion;
		manpower -= build_growth_proprotion;
		product -= build_growth_proprotion;

		float man_level_proportion = std::min({ food, product });
		if (man_level* MAN_CONSUME_PER_MAN > man_level_proportion * 2) {
			int fit_movement = 1;// (int)ceilf((min({ man_level_proportion * 360.f, GetAvailableSpace() }) - man_level) / 2);
			if (man_level + fit_movement >= MAN_LEVEL_MAX) man_level = MAN_LEVEL_MAX;
			else man_level += fit_movement;
			food -= man_level * MAN_CONSUME_PER_MAN * 1.5f;
			product -= man_level * MAN_CONSUME_PER_MAN * 1.5f;
		}
		else if (man_level * MAN_CONSUME_PER_MAN > man_level_proportion) {
			food -= man_level * MAN_CONSUME_PER_MAN;
			product -= man_level * MAN_CONSUME_PER_MAN;
		}
		else {
			int fit_movement = (int)floorf(-(man_level_proportion / MAN_CONSUME_PER_MAN - man_level) / 10.f);
			if (fit_movement > man_level) man_level = 0;
			else man_level -= fit_movement;
			if (food > man_level_proportion * MAN_CONSUME_PER_MAN)
				food -= man_level_proportion * MAN_CONSUME_PER_MAN;
			else food = 0;
			if (product > man_level_proportion * MAN_CONSUME_PER_MAN)
				product -= man_level_proportion * MAN_CONSUME_PER_MAN;
			else product = 0;
		}
		manpower = (float)man_level;

		if (build_growth > 0) {
			float product_amount = std::min({ build_growth , 1.f, GetAvailableSpace() });
			product += product_amount;
			build_growth -= product_amount;
			if (build_growth > 1 && building_level + 1 <= BUILDING_LEVEL_MAX) {
				if (float building_upgrade_cost = powf((float)building_level + 1, 2); product > (float)building_level && manpower > (float)building_level) {
					product -= building_upgrade_cost;
					manpower -= building_upgrade_cost;
					building_level += 1;
				}
			}
		}

		if (farm_growth > 0) {
			float food_amount = std::min({ farm_growth , 1.f, GetAvailableSpace() });
			food += food_amount;
			farm_growth -= food_amount;
			if (farm_growth > 1 && farm_level + 1 <= FARM_LEVEL_MAX) {
				if (float farm_upgrade_cost = powf((float)farm_level + 1, 2); product > (float)farm_level && manpower > (float)farm_level) {
					product -= farm_upgrade_cost;
					manpower -= farm_upgrade_cost;
					farm_level += 1;
				}
			}
		}
		road_growth -= 0.00001f * road_level;
		if (road_growth < 0) {
			road_growth = 0;
			if (road_level > 0) road_level -= 1;
		}
		else if (road_growth >= 1) {
			road_growth = 0;
			if (road_level + 1 <= ROAD_LEVEL_MAX) road_level += 1;
		}
	}

	const float GetAvailableSpace() { return std::max({ powf((float)building_level, 5.2f) - food - product - man_level, 0.f }); }
	const float GetThroughOut() const { return geo_desc->mho * powf(road_level * 1.f / ROAD_LEVEL_MAX, 2); }
};

inline float IntoRange(float var, float min, float max)
{
	return (var - min) / (max - min);
}