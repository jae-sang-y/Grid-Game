#pragma once
#include "DXUT.h"

struct StrategyBlock {
	BlockPropaganda prop{ 0, 0, 0 };

	int lifetime = 0;
	int distnace_from_border = 0;
	double capital_power = 0.0;
	bool passable = false;
};

struct NationID
{	const size_t data;
	NationID(size_t _data) :data(_data) {}
	static NationID createNewOne() {
		static int id_isequence = 0;
		return NationID(++id_isequence);
	}
};
class Nation
{
public:
	const NationID ID;
	XMFLOAT4 color = {};

	struct Capital : GetSet<Block*> {
		Block* const set(Block* new_data) override {
			assert(new_data != nullptr);
			assert(new_data->owner == handler);
			return data = new_data;
		}
		Nation* handler = nullptr;
	};
	
	Capital capital{};

	double score = 0;
	int army_size = 0;
	int enemy_army_size = 0;
	int nation_size = 0;
	bool notFall = true;
	bool revolt_war = false;
	int peace_time = 0;
	bool on_war = false;

	struct Relation {
		bool is_neigbor = false;
		bool is_war = false;
		bool is_revolt_war = false;
		Nation* const target;
		Nation* const owner;

		Relation(Nation* owner, Nation* target) : owner(owner), target(target) {}
		Relation() = delete;
		Relation(Relation&) = delete;
		Relation(Relation&&) = delete;
		int time_from_last_enemy_action = 0;
	};
	std::map<Nation*, Relation*> relats{};

	Nation(const XMFLOAT4 color, const NationID ID) : color(color), ID(ID) {
	}

	void DraftArmy(Block* target);

	void ClearStat();
	void OnBlockPropaganda(Block* blockA, Block* blockB);
	void Step();
	void PostStep();
	void LostCapital();
	void TakeBlock(Block* target);
	void SpoilBlock(Block* target);
	bool HasWarWith(Nation * const target);

	~Nation() {
		for (auto& relat : relats) delete relat.second;
	}
	void Fall();
	StrategyBlock strategy_map[MAP_W][MAP_H] = {};

	Relation* get_relation_with(Nation* const opponent)
	{
		if (auto relat = this->relats.find(opponent); relat != this->relats.end())
		{
			return relat->second;
		}
		else
		{
			return nullptr;
		}
	}
public:
	static void DeclareWar(Nation* actor, Nation* target);
	static void StartRelation(Nation* actor, Nation* target);
};


namespace std {
	template<>
	struct hash<const Nation*> {
		inline size_t operator()(const Nation* x) const {
			// size_t value = your hash computations over x
			return x->ID.data;
		}
	};
}