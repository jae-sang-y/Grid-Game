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

#undef max
#undef min

typedef std::wstring String;
const String MAP_PATH = L"res/map.bmp";
constexpr size_t MAP_W = 80;
constexpr size_t MAP_H = 50;

using namespace DirectX;

constexpr static float THIN_BORDER_SIZE = 1.25f;
constexpr static float BOLD_BORDER_SIZE = 1.5f;

constexpr static int FARM_LEVEL_MAX = 127;
constexpr static int BUILDING_LEVEL_MAX = 15;
constexpr static int MAN_LEVEL_MAX = 1000000;

constexpr static int ROAD_LEVEL_MIN = 1;
constexpr static int ROAD_LEVEL_MAX = 63;

constexpr static float SPOIL_PROPORTION = 1 / 3.f;

constexpr static int MAX_NATION_COUNT = 80;
constexpr static float MAN_CONSUME_PER_MAN = 1 / 180.f;

const static XMVECTOR MAP_LENS_FARM_MIN = XMVectorSet(203, 234, 229, 0);
const static XMVECTOR MAP_LENS_FARM_MAX = XMVectorSet(0, 69, 28, 0);
const static XMVECTOR MAP_LENS_BUILDING_MIN = XMVectorSet(255, 234, 204, 0);
const static XMVECTOR MAP_LENS_BUILDING_MAX = XMVectorSet(213, 122, 8, 0);

const static XMVECTOR MAP_LENS_HSV_MIN = XMVectorSet(4 / 360.f, 0.84f, 0.90f, 0);
const static XMVECTOR MAP_LENS_HSV_MAX = XMVectorSet(329 / 360.f, 0.84f, 0.90f, 0);

const static XMFLOAT3 MAP_LENS_ROAD_MIN = { 0 / 360.f, 190, 118 };
const static XMFLOAT3 MAP_LENS_ROAD_MAX = { 337 / 360.f, 108, 111 };

const static std::vector<std::string> Month_Name = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep","Oct", "Nov", "Dec" };

struct Rect {
	XMFLOAT2 pos{};
	XMFLOAT2 size{};
	Rect() {}
	Rect(XMFLOAT2 pos, XMFLOAT2 size) : pos(pos), size(size) {}
};

enum class Geo : int {
	Grassland = 0,
	River,
	Mountain,
	Blocked,
	ShallowOcean,
	DeepOcean,
	MediumOcean,
	Farmland,
	Max
};
struct GeoDesc {
	XMINT3 color = {};
	int farm_level = {};
	float mho = 0.f;
	bool livable = false;
	bool passable = false;

	void SetMhoByHalftime(int x) {
		if (x == 0) mho = 0;
		mho = 1 / powf(2, 1.f / x);
	}
};
static std::map<Geo, GeoDesc> geo_descs = {};
const static std::list<Geo> geo_list = {
	Geo::Grassland,
	Geo::River,
	Geo::Mountain,
	Geo::Blocked,
	Geo::ShallowOcean,
	Geo::DeepOcean,
	Geo::MediumOcean,
	Geo::Farmland,
};

enum class Forward : int {
	None,
	N, E, W, S
};

const std::map<Forward, XMINT2> forward_dir = {
	{Forward::E, XMINT2{+1, 00}},
	{Forward::N, XMINT2{00, -1}},
	{Forward::W, XMINT2{-1, 00}},
	{Forward::S, XMINT2{00, +1}}
};

inline void SetGeoInfo() {
	{
		geo_descs[Geo::Grassland].color = { 29, 230, 181 };
		geo_descs[Geo::Grassland].farm_level = 8;
		geo_descs[Geo::Grassland].SetMhoByHalftime(20);
		geo_descs[Geo::Grassland].livable = true;
		geo_descs[Geo::Grassland].passable = true;
	}
	{
		geo_descs[Geo::River].color = { 232, 162,   0 };
		geo_descs[Geo::River].farm_level = 0;
		geo_descs[Geo::River].SetMhoByHalftime(50);
		geo_descs[Geo::River].livable = false;
		geo_descs[Geo::River].passable = true;
	}
	{
		geo_descs[Geo::Mountain].color = { 87, 122, 185 };
		geo_descs[Geo::Mountain].farm_level = 3;
		geo_descs[Geo::Mountain].SetMhoByHalftime(10);
		geo_descs[Geo::Mountain].livable = true;
		geo_descs[Geo::Mountain].passable = true;
	}
	{
		geo_descs[Geo::Blocked].color = { 21,   0, 136 };
		geo_descs[Geo::Blocked].farm_level = 0;
		geo_descs[Geo::Blocked].SetMhoByHalftime(0);
		geo_descs[Geo::Blocked].livable = false;
		geo_descs[Geo::Blocked].passable = false;
	}
	{
		geo_descs[Geo::ShallowOcean].color = { 234, 217, 153 };
		geo_descs[Geo::ShallowOcean].farm_level = 0;
		geo_descs[Geo::ShallowOcean].SetMhoByHalftime(200);
		geo_descs[Geo::ShallowOcean].livable = false;
		geo_descs[Geo::ShallowOcean].passable = true;
	}
	{
		geo_descs[Geo::DeepOcean].color = { 204,  72,  63 };
		geo_descs[Geo::DeepOcean].farm_level = 0;
		geo_descs[Geo::DeepOcean].SetMhoByHalftime(50);
		geo_descs[Geo::DeepOcean].livable = false;
		geo_descs[Geo::DeepOcean].passable = true;
	}
	{
		geo_descs[Geo::MediumOcean].color = { 190, 146, 112 };
		geo_descs[Geo::MediumOcean].farm_level = 0;
		geo_descs[Geo::MediumOcean].SetMhoByHalftime(100);
		geo_descs[Geo::MediumOcean].livable = false;
		geo_descs[Geo::MediumOcean].passable = true;
	}
	{
		geo_descs[Geo::Farmland].color = { 76, 177,  34 };
		geo_descs[Geo::Farmland].farm_level = 10;
		geo_descs[Geo::Farmland].SetMhoByHalftime(4);
		geo_descs[Geo::Farmland].livable = true;
		geo_descs[Geo::Farmland].passable = true;
	}
}

struct BlockPropaganda {
	int origin;
	int rank = 0;
	float power = 0.f;

	BlockPropaganda(int ID, int rank, float power) : origin(ID), rank(rank), power(power) {}
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

inline BlockPropaganda operator - (BlockPropaganda& self, float decrement) {
	BlockPropaganda new_obj = self;
	new_obj.power -= decrement;
	return new_obj;
}

inline BlockPropaganda operator -= (BlockPropaganda& self, float decrement) {
	self.power -= decrement;
	return self;
}

inline void tassert(bool res) {
	if (!res) throw res;
}

template <typename T>
struct Range {
	Range(T core, T min, T max) : core(core), min(min), max(max) {}

	bool isValid() const {
		return min <= core && core <= max;
	}

	template <class O>
	Range<T>& operator = (O o) {
		core = o;
		tassert(isValid());
		return *this;
	}
	template <class O>
	Range<T>& operator += (O o) {
		core += o;
		tassert(isValid());
		return *this;
	}
	template <class O>
	Range<T>& operator -= (O o) {
		core -= o;
		tassert(isValid());
		return *this;
	}

	template <class O> bool operator <(O o) const { return core < o; }
	template <class O> bool operator <=(O o) const { return core <= o; }
	template <class O> bool operator >=(O o) const { return core >= o; }
	template <class O> bool operator >(O o) const { return core > o; }

	const T data() const {
		return core;
	}

private:
	T core = 0.f;
	T max = 0.f;
	T min = 0.f;
};

inline float IntoRange(float var, float min, float max)
{
	return (var - min) / (max - min);
}

struct Block;
struct Army;
struct Nation;
std::list<Block*> all_blocks();