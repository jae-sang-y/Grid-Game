#pragma once
#include "../GameData.h"
struct Block {
	Range<int> farm_level{ 0, 0, FARM_LEVEL_MAX };
	Range<int> building_level{ 0, 0, BUILDING_LEVEL_MAX };
	Range<int> man_level{ 0, 0, MAN_LEVEL_MAX };
	Range<int> road_level = { 0, 0, ROAD_LEVEL_MAX };

	Range<float> food{ 1, 0, 1000000 };
	Range<float> product{ 1, 0, 1000000 };
	Range<float> manpower{ 1, 0, 1000000 };

	Range<float> road_growth{ 0, 0, 1 };
	Range<float> farm_growth{ 0, 0, 1 };
	Range<float> build_growth{ 0, 0, 1 };

	float multiply_by_demand = 1.f;

	Geo geo = Geo::Blocked;
	XMFLOAT4 base_color = {};
	XMFLOAT4 geo_color{};
	const GeoDesc* geo_desc = nullptr;
	std::map<Forward, Block*> neighbors = {};
	std::map<Forward, Rect> thin_border = {};
	std::map<Forward, Rect> bold_border = {};
	Rect base_rect{};
	Rect geo_rect[2]{};

	BlockPropaganda farm_info = { ID, 0, 0 };
	BlockPropaganda build_info = { ID, 0, 0 };
	BlockPropaganda man_info = { ID, 0, 0 };

	int ID = -1;
	size_t X = -1, Y = -1;

	Block* self = nullptr;
	Nation* owner = nullptr;
	Army* army = nullptr;

	Block(const int pos_x, const int pos_y) : ID(pos_x + pos_y * MAP_W), X(pos_x), Y(pos_y) {}
	Block() {}

	float Appraise_1() const {
		static float building_proportion = 1.f / BUILDING_LEVEL_MAX;
		static float farm_proportion = 1.f / FARM_LEVEL_MAX;
		static float man_proportion = 1.f / MAN_LEVEL_MAX;

		return building_level.data() * building_proportion
			+ farm_level.data() * farm_proportion +
			man_level.data() * man_proportion;
	}

	const float GetAvailableSpace() { return std::max({ powf((float)building_level.data(), 5.2f) + 300 - food.data() - product.data() - man_level.data(), 0.f }); }
	const float GetThroughOut() const { return geo_desc->mho * (1 + powf(road_level.data() * 1.f / ROAD_LEVEL_MAX, 2)); }

	void OnUpdateInfo();
	void OnPropaganda(Block* other);
	void OnStep();
};