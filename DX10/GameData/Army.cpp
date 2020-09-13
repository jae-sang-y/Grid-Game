#include "DXUT.h"
#include "Army.hpp"
#include "Block.hpp"
#include "Nation.hpp"

Army::Army(Nation* owner, Block* target) : belong(owner), ground(target) {
	tassert(CanMove(target));
	target->army = this;
}

void Army::Step() {
	if (died) return;
	tassert(ground != nullptr && belong != nullptr);
	if (isDying()) {
		Die();
		return;
	}
	if (ground->owner != belong) {
		if (belong->HasWarWith(ground->owner)) {
			belong->TakeBlock(ground);
			belong->SpoilBlock(ground);
		}
	}

	if (energy < 15) {
		energy += 10;
	}
	else {
		Block* target = nullptr;
		BlockPropaganda target_prop = belong->strategy_map[ground->X][ground->Y].prop;
		//if (!owner->strategy_map[ground->X][ground->Y].passable) 
			target_prop = { ground->ID, 0, 0 };
		for (auto& neighbor : ground->neighbors) {
			auto neighbor_sblock = belong->strategy_map[neighbor.second->X][neighbor.second->Y];
			if (neighbor_sblock.prop > target_prop && (CanMove(neighbor.second) || CanAttack(neighbor.second))) {
				target = neighbor.second;
				target_prop << neighbor_sblock.prop;
			}
		}
		if (target != nullptr) {
			if (target->army != nullptr) {
				if (target->army->belong != belong) Attack(target);
			}
			else Move(target);
		}
	}
}

bool Army::CanMove(Block* target) {
	return target != nullptr && target->army == nullptr && target->geo_desc->passable;
}

void Army::Move(Block* target) {
	tassert(CanMove(target) && energy >= 15);
	ground->army = nullptr;
	target->army = this;
	ground = target;
	if (ground->owner != nullptr && ground->owner != belong) {
		if (belong->relats.find(ground->owner) != belong->relats.end()) {
			if (belong->relats.at(ground->owner)->is_war) {
				belong->TakeBlock(ground);
				belong->SpoilBlock(ground);
				//ground->man_level = 0;
			}
		}
	}
	energy = 0;
}

bool Army::CanAttack(Block* target) {
	if (target == nullptr) return false;
	if(target->army == nullptr) return false;
	if(!target->geo_desc->passable) return false;
	if (!this->belong->HasWarWith(target->army->belong)) return false;
	return true;	
}

void Army::Attack(Block* target) {
	tassert(CanAttack(target));
	target->army->size -= std::min({ (int)ceilf(this->size.data() / 2.f), target->army->size.data() });
	this->size -= std::min({ (int)ceilf(target->army->size.data() / 2.f), this->size.data() });

	if (target->army->isDying()) target->army->Die();
	if (isDying()) Die();

	ground = target;
	energy = 0;
}

bool Army::isDying() {
	return this->size.data() <= 0;
}

void Army::Die() {
	if (!died) {
		this->ground->army = nullptr;
		this->ground = nullptr;
		this->belong->army_size -= 1;
		died = true;
	}
}