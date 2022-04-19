#pragma once
#include "DXUT.h"

class Block {
public:
	Range<int> farm_level{ 0, 0, FARM_LEVEL_MAX };
	Range<int> building_level{ 0, 0, BUILDING_LEVEL_MAX };
	Range<int> man_level{ 0, 0, MAN_LEVEL_MAX };
	Range<int> road_level = { 0, 0, ROAD_LEVEL_MAX };

	Range<double> food{ 1, 0, 1000000 };
	Range<double> product{ 1, 0, 1000000 };
	Range<double> manpower{ 1, 0, 1000000 };

	Range<double> road_growth{ 0, 0, 1 };
	Range<double> farm_growth{ 0, 0, 1 };
	Range<double> build_growth{ 0, 0, 1 };

	double multiply_by_demand = 1.0;

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
	size_t X = 0, Y = 0;

	Block* self = nullptr;
	Nation* owner = nullptr;
	Army* army = nullptr;
	int distance_from_border = 0;

	Block(const int pos_x, const int pos_y) : ID(pos_x + pos_y * MAP_W), X(pos_x), Y(pos_y) {}
	Block() {}

	float Appraise_1() const {
		static double building_proportion = 1.0 / BUILDING_LEVEL_MAX;
		static double farm_proportion = 1.0 / FARM_LEVEL_MAX;
		static double man_proportion = 1.0 / MAN_LEVEL_MAX;

		return building_level.data() * building_proportion
			+ farm_level.data() * farm_proportion +
			man_level.data() * man_proportion;
	}

	const double GetAvailableSpace() { return std::max({ pow(building_level.data(), 5.2) + 300 - food.data() - product.data(), 0.0 }); }
	const double GetThroughOut() const { return 30 * geo_desc->mho * (1 + 99 * pow(road_level.data() * 1.0 / ROAD_LEVEL_MAX, 2)); }

	void OnUpdateInfo();
	void OnPropaganda(Block* other);
	void OnStep();
	void OnPostStep();
};