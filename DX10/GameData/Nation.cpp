#include "DXUT.h"
#include "Army.hpp"
#include "Block.hpp"
#include "Nation.hpp"

void Nation::OnBlockPropaganda(Block* blockA, Block* blockB) {
	if (!self->notFall) return;
	if (blockA->owner == self && blockB->owner != nullptr && blockB->owner != blockA->owner) {
		if (relats.find(blockB->owner) == relats.end()) relats[blockB->owner] = new Relation(self, blockB->owner);
		relats[blockB->owner]->is_neigbor = true;
	}
	if (blockA->geo_desc->passable && blockB->geo_desc->passable) {
		auto& Sa = strategy_map[blockA->X][blockA->Y];
		auto& Sb = strategy_map[blockB->X][blockB->Y];
		bool blocked = false;
		if (blockB->army != nullptr) {
			if (blockB->army->owner == self) {
				if (blockB->army->energy >= 15) blocked = true;
			}
			else if (relats.find(blockB->owner) != relats.end()) {
				if (!relats.at(blockB->owner)->is_war)
					blocked = true;
			}
		}
		if (!blocked) Sa.prop << Sb.prop - 1.f;
	}
}

std::list<Nation*> all_nations();
void Nation::ClearStat() {
	if (!self->notFall) return;
	score = 0;
	on_war = false;
	for (auto& relat : relats) {
		relat.second->is_neigbor = false;
		if (relat.second->is_war) on_war = true;
	}
	for (auto& block : all_blocks()) {
		auto& S = strategy_map[block->X][block->Y];
		S.prop -= 1;

		if (block->owner == self) {
			score += block->Appraise_1();
			S.prop << BlockPropaganda(block->ID, 1, 100);
			//if (block->army != nullptr && block->army->owner == self) {
			//	S.prop << BlockPropaganda(block->ID, 3, 0);
			//}
		}
		else if (block->owner != nullptr) {
			if (relats.find(block->owner) != relats.end()) {
				if (relats.at(block->owner)->is_war)
					S.prop << BlockPropaganda(block->ID, 2, 100);
			}
		}
	}
};

void Nation::Step() {
	if (!self->notFall) return;
	if (self->army_size < self->nation_size / 5 + 1 && capital->army == nullptr) {
		if (capital->man_level > 10)
			DraftArmy(capital);
	}
	if (!self->on_war) {
		Nation* war_target = nullptr;
		float war_score = 0;
		for (auto& relat : relats) {
			if (auto& nation = relat.second->target; nation != self) {
				float score = nation->army_size + nation->nation_size - self->army_size - self->nation_size;
				if (score > war_score) {
					war_score = score;
					war_target = nation;
				}
			}
		}
		if (war_target != nullptr) {
			relats.at(war_target)->is_war = true;
			war_target->relats.at(self)->is_war = true;
		}
	}
};

void Nation::DraftArmy(Block* target) {
	tassert(target->army == nullptr);
	auto army = new Army(self, target);
	self->army_size += 1;
	target->man_leveldml w -= 10;
}

void Nation::TakeBlock(Block* target) {
	if (target->owner != nullptr) {
		target->owner->nation_size -= 1;
		if (target->owner->nation_size == 0)
			target->owner->Fall();
		else if (target->owner->capital == target) {
			Block* new_cap = nullptr;
			float cap_score = 0;
			for (auto& block : all_blocks()) {
				if (block->owner == target->owner && block != target) {
					if (cap_score < block->Appraise_1()) {
						cap_score = block->Appraise_1();
						new_cap = block;
					}
				}
			}
			if (new_cap == nullptr) target->owner->Fall();
			else {
				target->owner->capital = new_cap;
			}
		}
	}
	target->owner = self;
	nation_size += 1;
}

void Nation::Fall() {
	self->notFall = false;
	for (auto& nation : all_nations())
		if (nation->relats.find(self) != nation->relats.end())
			nation->relats.erase(self);

	for (auto& block : all_blocks()) {
		if (block->owner == self) {
			block->owner = nullptr;
		}
		if (auto& army = block->army; army != nullptr && army->owner == self) {
			army->Die();
		}
	}
};