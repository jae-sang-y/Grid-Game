#include "DXUT.h"

void DeclareWar(Nation* actor, Nation* target) {
	actor->relats.at(target)->is_war = true;
	target->relats.at(actor)->is_war = true;
	actor->relats.at(target)->time_from_last_enemy_action = -100;
	target->relats.at(actor)->time_from_last_enemy_action = -100;
	actor->on_war = true;
	target->on_war = true;
	actor->peace_time = 0;
	target->peace_time = 0;
	std::ostringstream ss{};
	ss << "BEG " << actor->ID.data << " ~ " << target->ID.data << "\n";
	//ss << "BEG " << target->ID << " ~ " << actor->ID << "\n";
	OutputDebugStringA(ss.str().c_str());
}
void EndWar(Nation* actor, Nation* opponent) {
	actor->relats.at(opponent)->is_war = false;
	opponent->relats.at(actor)->is_war = false;
	std::ostringstream ss{};
	ss << "END " << actor->ID.data << " ~ " << opponent->ID.data << "\n";
	//ss << "END " << target->ID << " ~ " << actor->ID << "\n";
	OutputDebugStringA(ss.str().c_str());
}


bool Nation::HasWarWith(Nation* const opponent) {
	if (auto rel = this->get_relation_with(opponent); rel != nullptr)
		if (rel->is_war)
			return true;
	return false;
}

void Nation::OnBlockPropaganda(Block* blockA, Block* blockB) {
	if (!this->notFall) return;
	if (blockA->owner == this && blockB->owner != nullptr && blockB->owner != blockA->owner) {
		if (this->get_relation_with(blockB->owner) == nullptr) relats[blockB->owner] = new Relation(this, blockB->owner);
		this->get_relation_with(blockB->owner)->is_neigbor = true;
	}
	if (blockA->geo_desc->passable && blockB->geo_desc->passable) {
		auto& Sa = strategy_map[blockA->X][blockA->Y];
		auto& Sb = strategy_map[blockB->X][blockB->Y];
		double bias = 0;
		if (blockB->army != nullptr and blockB->army->is_belongs(this))
			bias = Sa.prop.power / 2;
		bias += 0.01 * (Sa.prop.power * (1 - (blockA->road_level.data() / (double)ROAD_LEVEL_MAX)));
		bias += 0.01 * (Sa.prop.power * blockA->geo_desc->mho);
		if (Sb.passable) Sa.prop << (Sb.prop - bias);
	}
}

void Nation::ClearStat() {
	if (!this->notFall) return;
	tassert(capital != nullptr && capital.data->owner == this && capital.data->geo_desc->livable);
	{
		Nation* war_target = nullptr;
		double war_cost = 1e10;
		double threshold = 30 - this->peace_time;
		for (auto& relat : relats) {
			if (auto& opponent = relat.second->target; opponent != this) {
				if (relat.second->is_war)
				{
					if (this->army_size < this->enemy_army_size and
						opponent->army_size < opponent->enemy_army_size) {
						EndWar(this, opponent);
					}
					threshold += relat.first->army_size;
				}
				else
				{
					const auto& capitalA = this->capital.get();
					const auto& capitalB = opponent->capital.get();
					double distance = sqrt(
						pow(capitalA->X - capitalB->X, 2) +
						pow(capitalA->Y - capitalB->Y, 2)
					);
					if (distance > 100) distance = 100;
					double cost =// abs(opponent->army_size - this->army_size) * 
						(0.5 + distance * 0.01);
					if (war_cost > cost) {
						war_cost = cost;
						war_target = opponent; 
					}
				}
			}
		}
		if (war_target != nullptr && this->army_size - threshold > war_cost) {
			DeclareWar(this, war_target);
		}
		else if (not this->on_war)
		{
			if (peace_time > 100)
			{
				for (auto& opponent : main_pgm->AllNations())
				{
					if (this == opponent) continue;
					if (this->get_relation_with(opponent) == nullptr)
					{
						this->relats[opponent] = new Nation::Relation(this, opponent);
						opponent->relats[this] = new Nation::Relation(opponent, this);
					}
				}
			}
			else peace_time += 1;
		}
	}
	if (this->on_war) 
	{
		for (auto& relat : relats) {
			if (auto& target = relat.second->target; target != this) {
				if (relat.second->is_war && relat.first->notFall) {
					if (target->army_size == 0 and this->army_size == 0 or relat.second->time_from_last_enemy_action > 100) {
						EndWar(this, target);
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
	Block* max_man_level_block = nullptr;
	int max_man_level = this->capital.get()->man_level.data();
	
	for (auto& block : main_pgm->AllBlocks()) {
		auto& S = strategy_map[block->X][block->Y];
		S.prop -= 1;

		int OX = S.prop.origin % MAP_W;
		int OY = S.prop.origin / MAP_W;
		const auto& O = strategy_map[OX][OY];

		if (not block->geo_desc->passable) {
			S.passable = false;
		}
		else
		{
			if (block->owner == nullptr or block->owner == this or HasWarWith(block->owner))
			{
				if (block->army == nullptr)
				{
					S.passable = true;
						
				}
				else 
				{
					if (block->army->is_belongs(this) && block->army->get_energy() >= 15)
						S.passable = false;
					else if (block->army->is_belongs(this) or HasWarWith(block->army->get_belong()))
						S.passable = true;
					else
						S.passable = false;
				}
			}
			else
			{
				S.passable = false;
			}
		}
		if (not O.passable or O.prop.rank != S.prop.rank) {
			S.prop = BlockPropaganda(block->ID, 0, 0);
		}

		S.prop.power -= 0.1;
		if (OX == block->X and OY == block->Y or S.prop.power < 0)
		{
			S.prop = BlockPropaganda(block->ID, 0, 0);
		}
		if (S.passable) {
			if (block->owner == this) {
				score += block->Appraise_1();
				if (capital.get() == block)
				{
					if(block->army == nullptr)
						S.prop << BlockPropaganda(block->ID, 2, 10000);
					else
						S.prop << BlockPropaganda(block->ID, 3, 0);
				}
				else if (block->distance_from_border == 0)
				{
					if (block->army == nullptr)
						S.prop << BlockPropaganda(block->ID, 1, 10000);
					else
						S.prop << BlockPropaganda(block->ID, 2, 0);
				}
				else S.prop << BlockPropaganda(block->ID, 0, 100000);
				//if (block->army != nullptr && block->army->owner == this) {
				//	S.prop << BlockPropaganda(block->ID, 3, 0);
				//}
			}
			else if (block->owner != nullptr) {
				if (HasWarWith(block->owner))
				{
					S.prop << BlockPropaganda(block->ID, 4, 3000);
					if (block->owner->capital.get() == block)
						S.prop << BlockPropaganda(block->ID, 4, 30000);
				}
			}

			if (block->army != nullptr) {
				if (block->army->is_belongs(this)) this->army_size += 1;
				if (not block->army->is_belongs(this) and HasWarWith(block->army->get_belong())) {
					S.prop << BlockPropaganda(block->ID, 4, 30000);
				}
			}
		}

		if (block->owner == this)
		{
			if (block->man_level.data() > max_man_level)
			{
				max_man_level = block->man_level.data();
				max_man_level_block = block;
			}
		}
	}

	if (not this->on_war && this->capital.get() != max_man_level_block && max_man_level_block != nullptr)
	{
		this->capital.data = max_man_level_block;
	}
};

void Nation::Step() {
	if (not this->notFall) return;
	// f ((this->army_size < this->nation_size / 5 + 1 or this->on_war) and  capital.get()->army == nullptr) {
	// 	if (capital.get()->man_level > ARMY_MAN_COST)
	// 		DraftArmy(capital.get());
	// 
};

void Nation::PostStep() {
	if (not this->notFall) return;
	this->army_size = 0;
	this->enemy_army_size = 0;
	for (auto& block : main_pgm->AllBlocks()) {
		if (block->army != nullptr) {
			if (block->army->is_belongs(this))
			{
				this->army_size += 1;
			}
			else if (this->HasWarWith(block->army->get_belong()))
			{
				this->enemy_army_size += 1;
			}
		}
	}

};

void Nation::DraftArmy(Block* target) {
	tassert(target->army == nullptr);
	auto army = new Army(this, target);
	this->army_size += 1;
	if (this->army_size > 3) army->size -= 10;
	if (this->army_size > 5) army->size -= 10;
	if (this->army_size > 7) army->size -= 10;
	target->man_level -= ARMY_MAN_COST;
	target->food -= ARMY_FOOD_COST;
}

void Nation::LostCapital() {
	Block* new_cap = nullptr;
	double cap_score = 0;
	for (auto& block : main_pgm->AllBlocks()) {
		if (block->owner == this && block->geo_desc->livable) {
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
	target->owner = this;
	nation_size += 1;
	if (former_owner != nullptr) {
		if (former_owner->notFall) {
			if (former_owner->capital.get() == target) {
				former_owner->LostCapital();
				if (former_owner->army_size == 0 && this->army_size > 0)
				{
					former_owner->Fall();
				}
			}
		}
	}
}

void Nation::SpoilBlock(Block* target) {
	Block* destination = capital.get();
	double ruin_food_amount = std::min({
		target->food.data() * SPOIL_PROPORTION, 
		destination->GetAvailableSpace() 
	});
	destination->food += ruin_food_amount;
	target->food -= ruin_food_amount;

	double ruin_product_amount = std::min({ 
		target->product.data()* SPOIL_PROPORTION ,
		destination->GetAvailableSpace() 
	});
	destination->product += ruin_product_amount;
	target->product -= ruin_product_amount;

	int ruin_man_amount = std::min({ 
		(int)ceil(target->man_level.data()* SPOIL_PROPORTION) ,
		MAN_LEVEL_PER_BUILD_LEVEL * (1 + destination->building_level.data()) - destination->man_level.data(),
		//MAN_LEVEL_MAX - destination->man_level.data()
	});
	destination->man_level += ruin_man_amount;
	target->man_level -= ruin_man_amount;

	if (target->farm_level > 1)
		target->farm_level = 1;
	if(target->building_level > 1)
		target->building_level = 1;
	target->manpower = 0;
}


void Nation::Fall() {
	this->notFall = false;
	for (auto& nation : main_pgm->AllNations())
		if (nation->get_relation_with(this) != nullptr)
		{
			nation->relats.erase(this);
		}
	this->relats.clear();

	for (auto& block : main_pgm->AllBlocks()) {
		if (block->owner == this) {
			block->owner = nullptr;
		}
		if (auto& army = block->army; army != nullptr && army->is_belongs(this)) {
			army->Die();
		}
	}
};