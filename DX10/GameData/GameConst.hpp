#pragma once
#include <list>

#define or ||
#define and &&
#define not !

typedef std::wstring String;
const String MAP_PATH = L"res/map.bmp";
constexpr size_t MAP_W = 80;
constexpr size_t MAP_H = 60;

using namespace DirectX;

constexpr static float THIN_BORDER_SIZE = 1.25f;
constexpr static float BOLD_BORDER_SIZE = 1.5f;

constexpr static int FARM_LEVEL_MAX = 127;
constexpr static int BUILDING_LEVEL_MAX = 15;
constexpr static int MAN_LEVEL_MAX = 1000000;
constexpr static int MAN_LEVEL_PER_BUILD_LEVEL = MAN_LEVEL_MAX / (1 + BUILDING_LEVEL_MAX);
constexpr static double MAN_BORN_RATE = 0.0001;

constexpr static int ROAD_LEVEL_MIN = 1;
constexpr static int ROAD_LEVEL_MAX = 63;
constexpr static double ROAD_LEVEL_GROWTH = 0.1;

constexpr static int ARMY_MAN_COST = 50;
constexpr static double ARMY_FOOD_COST = 0.3;
constexpr static double ARMY_FOOD_CONSUME = 0.1;
constexpr static double SPOIL_PROPORTION = 0.5;

constexpr static int MAX_NATION_COUNT = 100;
constexpr static double MAN_CONSUME_PER_MAN = 0.1;
constexpr static double FOOD_PER_FARM_RATE = 0.1;

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
	double mho = 0.0;
	bool livable = false;
	bool passable = false;

	void SetMhoByHalftime(int x) {
		if (x == 0) mho = 0;
		mho = pow(2, -1.0 / x);
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
		geo_descs[Geo::River].SetMhoByHalftime(300);
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
		geo_descs[Geo::ShallowOcean].SetMhoByHalftime(250);
		geo_descs[Geo::ShallowOcean].livable = false;
		geo_descs[Geo::ShallowOcean].passable = true;
	}
	{
		geo_descs[Geo::DeepOcean].color = { 204,  72,  63 };
		geo_descs[Geo::DeepOcean].farm_level = 0;
		geo_descs[Geo::DeepOcean].SetMhoByHalftime(5);
		geo_descs[Geo::DeepOcean].livable = false;
		geo_descs[Geo::DeepOcean].passable = true;
	}
	{
		geo_descs[Geo::MediumOcean].color = { 190, 146, 112 };
		geo_descs[Geo::MediumOcean].farm_level = 0;
		geo_descs[Geo::MediumOcean].SetMhoByHalftime(50);
		geo_descs[Geo::MediumOcean].livable = false;
		geo_descs[Geo::MediumOcean].passable = true;
	}
	{
		geo_descs[Geo::Farmland].color = { 76, 177,  34 };
		geo_descs[Geo::Farmland].farm_level = 10;
		geo_descs[Geo::Farmland].SetMhoByHalftime(17);
		geo_descs[Geo::Farmland].livable = true;
		geo_descs[Geo::Farmland].passable = true;
	}
}


enum class MapLens {
	Politic,
	Farm, Building, Man,
	Food, Product, Road, Strategymap
};


inline bool operator == (XMINT3 a, XMINT3 b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}
