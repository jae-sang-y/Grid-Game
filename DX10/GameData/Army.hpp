#pragma once
#include "../GameData.h"
struct Army {
	Army(Nation* owner, Block* target);
	void Step();
	bool CanMove(Block* target);
	void Move(Block* target);
	bool CanAttack(Block* target);
	void Attack(Block* target);
	bool isDying();
	void Die();

	Block* ground;
	Nation* owner;
	int energy = 0;
	int size = 100;
	Forward forward = Forward::None;
};