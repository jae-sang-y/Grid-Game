#pragma once

class Army {
public:
	Army(Nation* owner, Block* target);
	void Step();
	const bool CanMove(const Block* target);
	void Move(Block* target);
	const bool CanAttack(const Block* target);
	void Attack(Block* target);
	const bool isDying();
	void Die();

	const bool is_belongs(const Nation* nation) { return this->belong == nation; };
	Nation* get_belong() { return this->belong; };
	void set_belong(Nation* nation) { this->belong = nation; };
	const int get_size() { return this->size.data(); };
	const bool is_died() { return this->died; };
	const int get_energy() { return this->energy; }

	bool moved = true;
	Range<int> size = { 100, 0, 100 };
private:
	bool died = false;
	
	Block* ground;
	Nation* belong;
	int energy = 0;
	Forward forward = Forward::None;
	int decremnt_time = 0;
};