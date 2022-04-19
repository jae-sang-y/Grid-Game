#include "DXUT.h"

Army::Army(Nation* owner, Block* target) : belong(owner), ground(target) {
	tassert(CanMove(target));
	target->army = this;
}

void Army::Step() {
	if (died) return;
	tassert(ground != nullptr && belong != nullptr);
	if (decremnt_time == 0) {
		if (energy < 15)
			size -= 1;
		decremnt_time = 7;
	}
	else decremnt_time -= 1;
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
	if (energy>= 15 && not moved)
	{
		Block* target = nullptr;
		BlockPropaganda target_prop = belong->strategy_map[ground->X][ground->Y].prop;
		//if (!owner->strategy_map[ground->X][gro<und->Y].passable) 
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

	if (this->ground != nullptr && energy < 1500)
	{
		int food_amount = std::min({
			140.0,
			this->ground->food.data() / ARMY_FOOD_CONSUME,
			1500.0 - energy - 1
			});
		this->ground->food -= food_amount * ARMY_FOOD_CONSUME;
		energy += food_amount + 5;
	}
}

const bool Army::CanMove(const Block* target) {
	if (target->owner != nullptr and target->owner != this->belong)
	{
		Nation* me = this->belong;
		Nation* opponent = target->owner;
		if (me->get_relation_with(opponent) == nullptr)
		{
			me->relats[opponent] = new Nation::Relation(me, opponent);
			opponent->relats[me] = new Nation::Relation(opponent, me);
		}
	}
	return target != nullptr && target->army == nullptr && target->geo_desc->passable && (
		target->owner == nullptr or target->owner == this->belong or this->belong->HasWarWith(target->owner)
		);
}

void Army::Move(Block* target) {
	tassert(CanMove(target) && energy >= 15 && died == false);
	ground->army = nullptr;
	target->army = this;
	ground = target;
	if (ground->owner != nullptr && ground->owner != belong) {
		if (auto relat = belong->relats.find(ground->owner); relat != belong->relats.end()) {
			if (relat->second->is_war) {
				belong->TakeBlock(ground);
				belong->SpoilBlock(ground);
				relat->second->time_from_last_enemy_action = 0;
				//ground->man_level = 0;
			}
		}
	}
	//else if (ground->owner == nullptr and ground->geo_desc->livable and ground->man_level > 0)
	//{
	//	belong->TakeBlock(ground);
	//	belong->SpoilBlock(ground);
	//}
	energy -= 15;
	moved = true;
}

const bool Army::CanAttack(const Block* target) {
	if (target == nullptr) return false;
	if(target->army == nullptr) return false;
	if(!target->geo_desc->passable) return false;
	if (!this->belong->HasWarWith(target->army->belong)) return false;
	return true;	
}

void Army::Attack(Block* target) {
	tassert(CanAttack(target));
	target->army->size -= std::min({ (int)ceil(this->size.data() / 20.0), target->army->size.data() });
	this->size -= std::min({ (int)ceil(target->army->size.data() / 20.0), this->size.data() });
	belong->get_relation_with(target->army->belong)->time_from_last_enemy_action = 0;

	if (target->army->isDying())
	{
		target->army->Die();
		if (isDying()) {
			Die();
			return;
		}
		else
		{
			this->Move(target);
		}
	}
	else
	{
		if (isDying()) {
			Die();
			return;
		}
	}

	energy -= 15;
	moved = true;
}

const bool Army::isDying() {
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