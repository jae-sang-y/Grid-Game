#include "DXUT.h"
#include "Army.hpp"
#include "Block.hpp"
#include "Nation.hpp"

#include <ostream>
#include <sstream>

void DeclareWar(Nation* actor, Nation* target) {
	actor->relats.at(target)->is_war = true;
	target->relats.at(actor)->is_war = true;
	std::ostringstream ss{};
	ss << "BEG " << actor->ID << " ~ " << target->ID << "\n";
	//ss << "BEG " << target->ID << " ~ " << actor->ID << "\n";
	OutputDebugStringA(ss.str().c_str());
}
void EndWar(Nation* actor, Nation* target) {
	actor->relats.at(target)->is_war = false;
	target->relats.at(actor)->is_war = false;
	std::ostringstream ss{};
	ss << "END " << actor->ID << " ~ " << target->ID << "\n";
	//ss << "END " << target->ID << " ~ " << actor->ID << "\n";
	OutputDebugStringA(ss.str().c_str());
}


bool Nation::HasWarWith(Nation* target) {
	if (relats.find(target) != relats.end())
		if (relats.at(target)->is_war)
			return true;
	return false;
}

void Nation::OnBlockPropaganda(Block* blockA, Block* blockB) {
	if (!self->notFall) return;
	if (blockA->owner == self && blockB->owner != nullptr && blockB->owner != blockA->owner) {
		if (relats.find(blockB->owner) == relats.end()) relats[blockB->owner] = new Relation(self, blockB->owner);
		relats[blockB->owner]->is_neigbor = true;
	}
	if (blockA->geo_desc->passable && blockB->geo_desc->passable) {
		auto& Sa = strategy_map[blockA->X][blockA->Y];
		auto& Sb = strategy_map[blockB->X][blockB->Y];
		if (Sb.passable) Sa.prop << Sb.prop - 1.f;
	}
}

std::list<Nation*> all_nations();
void Nation::ClearStat() {
	if (!self->notFall) return;
	tassert(capital != nullptr && capital.data->owner == self && capital.data->geo_desc->livable);
	if (!self->on_war) {
		Nation* war_target = nullptr;
		float war_score = 0;
		for (auto& relat : relats) {
			if (auto& nation = relat.second->target; nation != self && !nation->on_war) {
				float score = nation->army_size + nation->nation_size - self->army_size - self->nation_size;
				if (score > war_score) {
					war_score = score;
					war_target = nation;
				}
			}
		}
		if (war_target != nullptr && self->army_size > 0) {
			DeclareWar(self, war_target);
		}
	}
	else {
		for (auto& relat : relats) {
			if (auto& target = relat.second->target; target != self) {
				if (relat.second->is_war) {
					if (target->army_size == 0 && self->army_size == 0) {
						EndWar(self, target);
					}
				}
			}
		}
	}

	score = 0;
	on_war = false;
	for (auto& relat : relats) {
		relat.second->is_neigbor = false;
		if (relat.second->is_war) on_war = true;
	}
	for (auto& block : all_blocks()) {
		auto& S = strategy_map[block->X][block->Y];
		S.prop -= 1;

		int OX = S.prop.origin % MAP_W;
		int OY = S.prop.origin / MAP_W;
		if (!strategy_map[OX][OY].passable) {
			S.prop = BlockPropaganda(block->ID, 0, 0);
		}

		if (S.passable) {

			if (block->owner == self) {
				score += block->Appraise_1();
				S.prop << BlockPropaganda(block->ID, 1, 100);
				if (capital.get() == block)
					S.prop << BlockPropaganda(block->ID, 1, 100);
				//if (block->army != nullptr && block->army->owner == self) {
				//	S.prop << BlockPropaganda(block->ID, 3, 0);
				//}
			}
			if (block->owner != nullptr) {
				if (HasWarWith(block->owner))
				{
					S.prop << BlockPropaganda(block->ID, 2, 300);
					if (block->owner->capital.get() == block)
						S.prop << BlockPropaganda(block->ID, 2, 300);
				}
			}

			if (block->army != nullptr) {
				if (HasWarWith(block->army->belong)) {
					S.prop << BlockPropaganda(block->ID, 3, 10);
				}
			}
		}
		S.prop << BlockPropaganda(block->ID, 0, 0);

		S.passable = true;
		if (!block->geo_desc->passable) {
			S.passable = false;
		}
		if (block->army != nullptr) {
			if (block->army->belong == self) {
				if (block->army->energy >= 15) S.passable = false;
			}
			else if (!self->HasWarWith(block->army->belong)) S.passable = false;
		}
		if (block->owner != nullptr && (!HasWarWith(block->owner)) && block->owner->capital == block) {
			S.passable = false;
		}
	}
};

void Nation::Step() {
	if (!self->notFall) return;
	if (self->army_size < self->nation_size / 5 + 1 && capital.get()->army == nullptr) {
		if (capital.get()->man_level > ARMY_MAN_COST)
			DraftArmy(capital.get());
	}
};

void Nation::DraftArmy(Block* target) {
	tassert(target->army == nullptr);
	auto army = new Army(self, target);
	self->army_size += 1;
	target->man_level -= ARMY_MAN_COST;
}

void Nation::LostCapital() {
	Block* new_cap = nullptr;
	float cap_score = 0;
	for (auto& block : all_blocks()) {
		if (block->owner == self && block->geo_desc->livable) {
			if (cap_score < block->Appraise_1()) {
				cap_score = block->Appraise_1();
				new_cap = block;
			}
		}
	}
	if (new_cap == nullptr) {
		Fall();
	}
	else capital.set(new_cap);
}

void Nation::TakeBlock(Block* target) {
	Nation* former_owner = target->owner;
	if (target->owner != nullptr) {
		target->owner->nation_size -= 1;
		if (target->owner->nation_size == 0) target->owner->Fall();
	}
	target->owner = self;
	nation_size += 1;
	if (former_owner != nullptr) {
		if (former_owner->notFall) {
			if (former_owner->capital.get() == target) {
				former_owner->LostCapital();
			}
		}
	}
}

void Nation::SpoilBlock(Block* target) {
	float ruin_food_amount = std::min({ target->food.data() * SPOIL_PROPORTION, capital.get()->GetAvailableSpace() });
	capital.get()->food += ruin_food_amount;
	target->food -= ruin_food_amount;

	float ruin_product_amount = std::min({ target->product.data()* SPOIL_PROPORTION , capital.get()->GetAvailableSpace() });
	capital.get()->product += ruin_product_amount;
	target->product -= ruin_product_amount;

	int ruin_man_amount = std::min({ (int)ceilf(target->man_level.data()* SPOIL_PROPORTION) , MAN_LEVEL_MAX - capital.get()->man_level.data() });
	capital.get()->man_level += ruin_man_amount;
	target->man_level -= ruin_man_amount;
}


void Nation::Fall() {
	self->notFall = false;
	for (auto& nation : all_nations())
		if (nation->relats.find(self) != nation->relats.end())
		{
			nation->relats.erase(self);
		}
	relats.clear();

	for (auto& block : all_blocks()) {
		if (block->owner == self) {
			block->owner = nullptr;
		}
		if (auto& army = block->army; army != nullptr && army->belong == self) {
			army->Die();
			army = nullptr;
		}
	}
};