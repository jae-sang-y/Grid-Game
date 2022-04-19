#include "DXUT.h"

void Block::OnUpdateInfo() {
	static double building_proportion = 1.0 / BUILDING_LEVEL_MAX;
	static double farm_proportion = 1.0 / FARM_LEVEL_MAX;
	static double man_proportion = 1.0 / MAN_LEVEL_MAX;

	farm_info -= geo_desc->mho;
	build_info -= geo_desc->mho;
	man_info -= geo_desc->mho;
	//if (owner != nullptr)
	{
		farm_info << BlockPropaganda(ID, farm_level.data(), pow(farm_level.data() * farm_proportion, 2));
		build_info << BlockPropaganda(ID, building_level.data(), pow(building_level.data() * building_proportion, 2));

		man_info << BlockPropaganda(ID, man_level.data(), pow(man_level.data() * man_proportion, 2));
		///if (owner != nullptr && distance_from_border == 0 and army != nullptr)
		//{
		//	man_info << BlockPropaganda(ID, MAN_LEVEL_MAX, 1);
		//}
	}
}

void Block::OnPropaganda(Block* other) {
	double moving_limit = (this->GetThroughOut() + other->GetThroughOut()) / 2.0;
	int manpower_moving_limit = road_level.data();
	
	// if (this->man_level > 1000)
	// {
	// 	if (this->owner != nullptr and other->owner == nullptr and other->army == nullptr and other->geo_desc->livable)
	// 	{
	// 		int moving_amount = this->man_level.data();
	// 		moving_amount = std::min({
	// 			moving_amount,
	// 			manpower_moving_limit,
	// 			MAN_LEVEL_MAX - other->man_level.data(),
	// 			MAN_LEVEL_PER_BUILD_LEVEL * (1 + other->building_level.data()) - other->man_level.data()
	// 			});
	// 
	// 		other->man_level += moving_amount;
	// 		this->man_level -= moving_amount;
	// 		this->owner->TakeBlock(other);
	// 	}
	// }

	if (this->farm_info > other->farm_info) {
		other->farm_info << this->farm_info;
		int moving_amount = other->man_level.data() / 2;
		moving_amount = std::min({
			moving_amount,
			manpower_moving_limit,
			MAN_LEVEL_MAX - man_level.data(),
			MAN_LEVEL_PER_BUILD_LEVEL * (1 + this->building_level.data()) - this->man_level.data()
			});
		if (moving_amount > 0)
		{
			other->man_level -= moving_amount;
			this->man_level += moving_amount;
			road_growth += std::min({ ROAD_LEVEL_GROWTH * moving_amount / (1 + road_level.data()), 1 - road_growth.data() });
		}
	}
	if (this->build_info > other->build_info) {
		other->build_info << this->build_info;
		int moving_amount = other->man_level.data();
		moving_amount = std::min({
			moving_amount,
			manpower_moving_limit,
			MAN_LEVEL_MAX - man_level.data(),
			MAN_LEVEL_PER_BUILD_LEVEL* (1 + this->building_level.data()) - this->man_level.data()
			});
		if (moving_amount > 0)
		{
			other->man_level -= moving_amount;
			this->man_level += moving_amount;
			road_growth += std::min({ ROAD_LEVEL_GROWTH * moving_amount / (1 + road_level.data()), 1 - road_growth.data() });
		}
	}
	if (this->man_info > other->man_info) {
		other->man_info << this->man_info;
		{
			double moving_amount = other->food.data();
			moving_amount = std::min({
				moving_amount,
				moving_limit,
				this->GetAvailableSpace()
				});
			other->food -= moving_amount;
			this->food += moving_amount;
			road_growth += std::min({ ROAD_LEVEL_GROWTH * 3 * moving_amount / (1 + road_level.data()), 1 - road_growth.data() });
		}
		{
			double moving_amount = other->product.data();
			moving_amount = std::min({
				moving_amount,
				moving_limit,
				this->GetAvailableSpace()
				});
			other->product -= moving_amount;
			this->product += moving_amount;
			road_growth += std::min({ ROAD_LEVEL_GROWTH * 3 * moving_amount / (1 + road_level.data()), 1 - road_growth.data() });
		}
	}

	if (multiply_by_demand < 1.0 && 
		other->multiply_by_demand > multiply_by_demand &&
		man_level.data() > 0 &&
		other->geo_desc->livable &&
		other->man_level.data() + 1 < MAN_LEVEL_PER_BUILD_LEVEL * (1 + other->building_level.data())
	) {
		other->man_level += 1;
		this->man_level -= 1;
		if (man_level.data() > 0 && this->owner != nullptr && other->owner == nullptr  && !this->owner->on_war && other->army == nullptr) {
			this->owner->TakeBlock(other);
		}
	}
}

void Block::OnStep() {
	if (this->owner != nullptr)
	{
		if ((this->owner->army_size < sqrt(this->owner->nation_size) * 4 or this->owner->on_war) and this->army == nullptr) {
			if (this->man_level > ARMY_MAN_COST *2 && this->food > ARMY_FOOD_COST * 2)
				this->owner->DraftArmy(this);
		}
	}

	if (geo_desc->livable && man_level.data() > 0)
	{
		double farm_growth_proprotion = std::min({ manpower.data() / 2, product.data() / 2 + 1, 1.0 - farm_growth.data() });

		farm_growth += farm_growth_proprotion;
		manpower -= farm_growth_proprotion;
		if (farm_growth_proprotion >= 1)
			product -= farm_growth_proprotion - 1;

		double build_growth_proprotion = std::min({ manpower.data(), product.data() + 1, 1.0 - build_growth.data() });
		build_growth += build_growth_proprotion;
		manpower -= build_growth_proprotion;

		if (build_growth_proprotion >= 1)
			product -= build_growth_proprotion - 1;

		double food_supply = food.data();
		double food_demand = man_level.data() * MAN_CONSUME_PER_MAN;
		multiply_by_demand = food_supply / food_demand;
		if (multiply_by_demand > 2) {
			food -= food_demand;

			double man_increment = std::min({ 
				food.data() / MAN_CONSUME_PER_MAN,
				ceil(man_level.data() * MAN_BORN_RATE),
				MAN_LEVEL_PER_BUILD_LEVEL * (1.0 + building_level.data()) - man_level.data(),
				MAN_LEVEL_MAX - (double)man_level.data()
				});
			
			if (man_increment > 0)
			{
				man_level += round(man_increment);
				food -= man_increment * MAN_CONSUME_PER_MAN;
			}
		}
		else if (multiply_by_demand >= 1) {
			food -= food_demand;
		}
		else
		{
			if (food.data() > food_demand)
				food -= food_demand;
			else {
				food_demand -= food.data();
				food = 0;
				double man_decrement = std::min({
					(double)man_level.data(),
					floor(food_demand / MAN_CONSUME_PER_MAN)
				});
				man_level -= man_decrement;
			}
		}
		manpower = (double)man_level.data();

		{
			double product_amount = std::min(
				{
					1.2 * pow(building_level.data() / (double) BUILDING_LEVEL_MAX, 2)
					 * pow(man_level.data() / (double)MAN_LEVEL_MAX, 2),
					GetAvailableSpace()
				});
			product += product_amount;
		}
		if (build_growth > 0) {
			if (build_growth >= 0.9 && building_level.data() + 1 <= BUILDING_LEVEL_MAX) {
				if (double building_upgrade_cost = pow((double)building_level.data() + 1, 2); product > building_upgrade_cost && manpower.data() > building_upgrade_cost) {
					product -= building_upgrade_cost;
					manpower -= building_upgrade_cost;
					building_level += 1;
					build_growth = 0;
				}
			}
			// else
			// {
			// 	double product_amount = std::min({ build_growth.data() * 0.5 , 1 + (double)building_level.data(), GetAvailableSpace() });
			// 	product += product_amount;
			// 	build_growth -= product_amount;
			// }
		}

		{
			double food_amount = std::min({ 
				FOOD_PER_FARM_RATE 
				* geo_desc->farm_level
				* (1 + pow(farm_level.data() / (double)FARM_LEVEL_MAX, 2))
				* (1 + pow(man_level.data() / (double)MAN_LEVEL_MAX, 2))
				,
				GetAvailableSpace()
				});
			food += food_amount;
		}
		if (farm_growth > 0) {
			if (farm_growth >= 0.9 && farm_level.data() + 1 <= FARM_LEVEL_MAX) {
				if (double farm_upgrade_cost = 1 + pow((double)farm_level.data(), 2); product.data() > farm_upgrade_cost && manpower.data() > farm_upgrade_cost) {
					product -= farm_upgrade_cost;
					manpower -= farm_upgrade_cost;
					farm_level += 1;
					farm_growth -= 1;
				}
			}
			//else
			//{
			//	double food_amount = std::min({ farm_growth.data() * 0.5, (double)farm_level.data(), GetAvailableSpace() / (1 + farm_level.data()) });
			//	food += food_amount;
			//	farm_growth -= food_amount;
			//}
		}
	}
	else multiply_by_demand = FLT_MAX;

	if (geo_desc->passable) {
		double road_growth_decay = ROAD_LEVEL_GROWTH * 0.1 * road_level.data();
		if (road_growth < road_growth_decay) {
			road_growth = 0;
			//if (road_level > 0) road_level -= 1;
		}
		else if (road_growth >= 1) {
			road_growth = 0;
			if (road_level.data() + 1 <= ROAD_LEVEL_MAX) road_level += 1;
		}
		else {
			road_growth -= road_growth_decay;
		}
	}

	if (man_level.data() == 0 && owner != nullptr) {
		if (owner->capital.get() == self) {
			Nation* former_owner = owner;
			owner = nullptr;
			former_owner->LostCapital();
		}
		// owner = nullptr;
	}

	if (army != nullptr) {
		army->Step();
	}
}
void Block::OnPostStep() {
	if (self->army != nullptr)
	{
		self->army->moved = false;
		tassert(not self->army->is_died());
	}
	this->distance_from_border = 0x7fffffff;
}