#include "DXUT.h"
#include "Army.hpp"
#include "Block.hpp"
#include "Nation.hpp"

Army::Army(Nation* owner, Block* target) : owner(owner), ground(target) {
	tassert(CanMove(target));
	target->army = this;
}

void Army::Step() {
	if (energy < 15) energy += 10;
	else {
		Block* target = nullptr;
		BlockPropaganda target_prop = owner->strategy_map[ground->X][ground->Y].prop;
		for (auto& neighbor : ground->neighbors) {
			if (owner->strategy_map[neighbor.second->X][neighbor.second->Y].prop > target_prop) {
				target = neighbor.second;
				target_prop = owner->strategy_map[neighbor.second->X][neighbor.second->Y].prop;
			}
		}
		if (target != nullptr) {
			if (CanAttack(target)) Attack(target);
			else if (CanMove(target)) Move(target);
		}
	}
}

bool Army::CanMove(Block* target) {
	return target->army == nullptr && target->geo_desc->passable;
}

void Army::Move(Block* target) {
	tassert(CanMove(target) && energy >= 15);
	ground->army = nullptr;
	target->army = this;
	ground = target;
	if (ground->owner != nullptr && ground->owner != owner) {
		if (owner->relats.find(owner) != owner->relats.end()) {
			if (owner->relats.at(owner)->is_war) {
				owner->TakeBlock(ground);
			}
		}
	}
	energy = 0;
}

bool Army::CanAttack(Block* target) {
	if (target->army != nullptr && target->army->owner != this->owner && target->geo_desc->passable) {
		auto relat_itr = this->owner->relats.find(target->army->owner);
		if (relat_itr != this->owner->relats.end()) {
			return (*relat_itr).second->is_war;
		}
	}
	return false;
}

void Army::Attack(Block* target) {
	tassert(CanAttack(target) && energy >= 15);
	target->army->size -= ceilf(this->size / 10.f);
	this->size -= ceilf(target->army->size / 10.f);

	if (target->army->isDying()) target->army->Die();
	if (isDying()) Die();

	ground = target;
	energy = 0;
}

bool Army::isDying() {
	return this->size <= 0;
}

void Army::Die() {
	this->ground->owner = nullptr;
	this->owner->army_size -= 1;
	delete this;
}