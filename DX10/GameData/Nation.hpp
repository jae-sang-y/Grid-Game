#pragma once
#include "../GameData.h"
struct StrategyBlock {
	BlockPropaganda prop{ 0, 0, 0 };

	int lifetime = 0;
	int distnace_from_border = 0;
	float capital_power = 0.f;
	bool passable = false;
};

struct Nation
{
	Nation* self = nullptr;
	int ID = -1;
	XMFLOAT4 color = {};
	Block* capital = nullptr;

	float score = 0;
	int army_size = 0;
	int nation_size = 0;
	bool notFall = true;
	bool on_war = false;

	struct Relation {
		bool is_neigbor = false;
		bool is_war = false;
		Nation* const target;
		Nation* const owner;

		Relation(Nation* owner, Nation* target) : owner(owner), target(target) {}
	};
	std::map<Nation*, Relation*> relats{};

	Nation() {}
	Nation(XMFLOAT4 color) : color(color) {
		static int id_auto_increment = 0;
		ID = ++id_auto_increment;
	}

	void DraftArmy(Block* target);

	void ClearStat();
	void OnBlockPropaganda(Block* blockA, Block* blockB);
	void Step();
	void TakeBlock(Block* target);

	Nation* Nation::nation(int target);
	~Nation() {
		for (auto& relat : relats) delete relat.second;
	}
	void Nation::Fall();
	StrategyBlock strategy_map[MAP_W][MAP_H] = {};
};