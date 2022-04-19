#pragma once

#include <list>
#include <DirectXMath.h>
#include <iostream>
#include <DirectXMath.h>
#include <stdlib.h>
#include <ppl.h>
#include <functional>
#include <random>
#include <algorithm>
#include <map>
#include <fstream>


#undef max
#undef min


struct BlockPropaganda {
	int origin;
	int rank = 0;
	double power = 0.0;

	BlockPropaganda(int ID, int rank, double power) : origin(ID), rank(rank), power(power) {}
	BlockPropaganda(int ID) : BlockPropaganda(ID, 0, 0.f) {}
};

inline bool operator > (BlockPropaganda& self, BlockPropaganda other) { return (self.rank > other.rank || (self.rank == other.rank && self.power > other.power)) && self.power >= 0; }

inline BlockPropaganda& operator << (BlockPropaganda& self, BlockPropaganda other) {
	if (other > self) {
		self.rank = other.rank;
		self.power = other.power;
		self.origin = other.origin;
	}
	return self;
}

inline BlockPropaganda operator - (const BlockPropaganda self, double decrement) {
	BlockPropaganda new_obj = self;
	new_obj.power -= decrement;
	return new_obj;
}

inline BlockPropaganda operator -= (BlockPropaganda& self, double decrement) {
	self.power -= decrement;
	return self;
}

inline void tassert(bool res) {
	if (!res) throw res;
}


inline float IntoRange(float var, float min, float max)
{
	return (var - min) / (max - min);
}

template <typename T>
struct GetSet {
	T data = nullptr;
	const T get() {
		return data;
	}
	virtual const T set(T new_data) {
		return data = new_data;
	}

	template <typename T>
	bool operator == (T other) {
		return data == other;
	}

	template <typename T>
	bool operator != (T other) {
		return data != other;
	}	
};

class Nation;
class Block;
class Army;
class MainProgram;

#include "GameConst.hpp"

#include "../Object/Range.hpp"

#include "Block.hpp"
#include "Nation.hpp"
#include "Army.hpp"
#include "../main_program.hpp"