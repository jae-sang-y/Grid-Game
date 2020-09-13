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

	bool died = false;

	Block* ground;
	Nation* belong;
	int energy = 0;
	Range<int> size = { 100, 0, 100 };
	Forward forward = Forward::None;
};