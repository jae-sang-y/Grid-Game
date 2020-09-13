#include "DXUT.h"
#include "Army.hpp"
#include "Block.hpp"
#include "Nation.hpp"

void Block::OnUpdateInfo() {
	static float building_proportion = 1.f / BUILDING_LEVEL_MAX;
	static float farm_proportion = 1.f / FARM_LEVEL_MAX;
	static float man_proportion = 1.f / MAN_LEVEL_MAX;

	farm_info -= 1;
	build_info -= 1;
	man_info -= 1;
	if (owner != nullptr) {
		farm_info << BlockPropaganda(ID, farm_level.data(), powf(farm_level.data() * farm_proportion, 2));
		build_info << BlockPropaganda(ID, building_level.data(), powf(building_level.data() * building_proportion, 2));
		man_info << BlockPropaganda(ID, man_level.data(), powf(man_level.data() * man_proportion, 2));
	}
}

void Block::OnPropaganda(Block* other) {
	float moving_limit = (this->GetThroughOut() + other->GetThroughOut()) / 2.f;
	int manpwoer_moving_limit = roundf(moving_limit);
	if (this->farm_info > other->farm_info) {
		other->farm_info << this->farm_info;
		int moving_amount = other->manpower.data();
		moving_amount = std::min({
			moving_amount,
			manpwoer_moving_limit,
			MAN_LEVEL_MAX - man_level.data()
			});
		other->manpower -= moving_amount;
		this->manpower += moving_amount;
		road_growth += std::min({ 0.01f * moving_amount / (1 + road_level.data()), 1 - road_growth.data() });
	}
	if (this->build_info > other->build_info) {
		other->build_info << this->build_info;
		int moving_amount = other->manpower.data();
		moving_amount = std::min({
			moving_amount,
			manpwoer_moving_limit,
			MAN_LEVEL_MAX - man_level.data()
			});
		other->manpower -= moving_amount;
		this->manpower += moving_amount;
		road_growth += std::min({ 0.01f * moving_amount / (1 + road_level.data()), 1 - road_growth.data() });
	}
	if (this->man_info > other->man_info) {
		other->man_info << this->man_info;
		{
			float moving_amount = other->food.data();
			moving_amount = std::min({
				moving_amount,
				moving_limit,
				this->GetAvailableSpace()
				});
			other->food -= moving_amount;
			this->food += moving_amount;
			road_growth += std::min({ 0.01f * moving_amount / (1 + road_level.data()), 1 - road_growth.data() });
		}
		{
			float moving_amount = other->product.data();
			moving_amount = std::min({
				moving_amount,
				moving_limit,
				this->GetAvailableSpace()
				});
			other->product -= moving_amount;
			this->product += moving_amount;
			road_growth += std::min({ 0.01f * moving_amount / (1 + road_level.data()), 1 - road_growth.data() });
		}
	}

	if (multiply_by_demand < 1.f && other->multiply_by_demand > multiply_by_demand && man_level.data() > 0 && other->geo_desc->livable) {
		other->man_level += 1;
		this->man_level -= 1;
		if (man_level.data() > 0 && this->owner != nullptr && other->owner == nullptr  && !this->owner->on_war && other->army == nullptr) {
			this->owner->TakeBlock(other);
		}
	}
}

void Block::OnStep() {
	if (geo_desc->livable && man_level.data() > 0)
	{
		float farm_growth_proprotion = std::min({ manpower.data(), product.data(), 1.f - farm_growth.data() });

		farm_growth += farm_growth_proprotion;
		manpower -= farm_growth_proprotion;
		product -= farm_growth_proprotion;

		float build_growth_proprotion = std::min({ manpower.data(), product.data(), 1.f - build_growth.data() });
		build_growth += build_growth_proprotion;
		manpower -= build_growth_proprotion;
		product -= build_growth_proprotion;

		float man_level_proportion = std::min({ food.data() });
		multiply_by_demand = man_level_proportion / (man_level.data() * MAN_CONSUME_PER_MAN);
		if (multiply_by_demand > 1.5f) {
			food -= man_level.data() * MAN_CONSUME_PER_MAN * 1.5f;

			int fit_movement = roundf((multiply_by_demand - 1) * 0.01f * man_level.data());
			if (fit_movement > 1 && fit_movement > man_level.data() / 3) fit_movement = man_level.data() / 3;
			if (man_level.data() + fit_movement >= MAN_LEVEL_MAX) man_level = MAN_LEVEL_MAX;
			else
				man_level += fit_movement;
		}
		else if (multiply_by_demand >= 1) {
			food -= man_level.data() * MAN_CONSUME_PER_MAN;
		}
		else {
			if (food.data() > man_level_proportion * MAN_CONSUME_PER_MAN)
				food -= man_level_proportion * MAN_CONSUME_PER_MAN;
			else food = 0;

			int fit_movement = roundf((1 - multiply_by_demand) / 2 * man_level.data());
			if (fit_movement > man_level.data())
				man_level = 0;
			else
				man_level -= fit_movement;
		}
		manpower = (float)man_level.data();

		if (build_growth > 0) {
			float product_amount = std::min({ build_growth.data() , (float)building_level.data(), GetAvailableSpace() });
			product += product_amount;
			build_growth -= product_amount;
			if (build_growth > 1 && building_level.data() + 1 <= BUILDING_LEVEL_MAX) {
				if (float building_upgrade_cost = powf((float)building_level.data() + 1, 2); product > building_upgrade_cost && manpower.data() > building_upgrade_cost) {
					product -= building_upgrade_cost;
					manpower -= building_upgrade_cost;
					building_level += 1;
				}
			}
		}

		{
			float food_amount = std::min({ geo_desc->farm_level / 10.f, GetAvailableSpace() });
			food += food_amount;
		}
		if (farm_growth > 0) {
			{
				float food_amount = std::min({ farm_growth.data(), (float)farm_level.data(), GetAvailableSpace() / (1 + farm_level.data()) });
				food += food_amount;
				farm_growth -= food_amount;
			}
			if (farm_growth > 1 && farm_level.data() + 1 <= FARM_LEVEL_MAX) {
				if (float farm_upgrade_cost = powf((float)farm_level.data() + 1, 2); product.data() > farm_upgrade_cost && manpower.data() > farm_upgrade_cost) {
					product -= farm_upgrade_cost;
					manpower -= farm_upgrade_cost;
					farm_level += 1;
				}
			}
		}
	}
	else multiply_by_demand = FLT_MAX;

	if (geo_desc->passable) {
		float road_growth_decay = 0.00001f * road_level.data();
		if (road_growth < road_growth_decay) {
			road_growth = 0;
			if (road_level > 0) road_level -= 1;
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
		owner = nullptr;
	}

	if (army != nullptr) army->Step();
}