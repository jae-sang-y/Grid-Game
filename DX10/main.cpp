#include "DXUT.h"
#include "View.h"
#include "GameData.h"

#include <fstream>
#include <sstream>
#include <random>
#include <time.h>


enum class MapLens {
	Politic,
	Farm, Building, Man,
	Food, Product, Road
};

bool operator == (XMINT3 a, XMINT3 b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

struct Game {
private:
	std::mt19937_64 rand = std::mt19937_64(time(nullptr));
	MapLens map_lens = MapLens::Politic;
	Nation* selected_nation = nullptr;
	Block board[MAP_W][MAP_H] = {};
	std::list<Nation*> nations = {};
	float block_w = 1.f;
	float block_h = 1.f;
public:
	inline bool isOnBoard(const int& x, const int& y)
	{
		return x >= 0 && x < MAP_W && y >= 0 && y < MAP_H;
	}

	void SpreadNation() {
		for (int k = 0; k < MAX_NATION_COUNT; ++k)
		{
			do
			{
				int x = rand() % MAP_W;
				int y = rand() % MAP_H;

				if (auto& block = board[x][y]; block.owner == nullptr && geo_descs[block.geo].livable)
				{
					XMFLOAT4 color{
						(rand() % 1000) / 1000.f,
						(rand() % 1000) / 1000.f,
						(rand() % 1000) / 1000.f,
						1
					};
					float m = std::max({ color.x, color.y, color.z });
					color.x /= m;
					color.y /= m;
					color.z /= m;
					auto nation = new Nation(color);
					nation->capital = &block;
					nations.push_back(nation);

					block.owner = *nations.rbegin();
					block.man_level = 100;
					block.food = 100;
					block.product = 100;
					break;
				}
			} while (true);
		}
	}

	void MappingBlocks() {
		block_w = (float)virtual_W / MAP_W;
		block_h = (float)virtual_H / MAP_H;

		for (int x = 0; x < MAP_W; ++x)
		{
			for (int y = 0; y < MAP_H; ++y)
			{
				board[x][y] = Block(x, y);
				Block& block = board[x][y];

				for (auto forward : forward_dir)
				{
					int x2 = x + forward.second.x;
					int y2 = y + forward.second.y;

					if (isOnBoard(x2, y2))
					{
						block.neighbors.insert({ forward.first, &board[x2][y2] });
						switch (forward.first)
						{
						case Forward::E:
							block.thin_border.insert({ forward.first, Rect({(x + 1) * block_w - THIN_BORDER_SIZE, y * block_h }, { THIN_BORDER_SIZE, block_h }) });
							block.bold_border.insert({ forward.first, Rect({(x + 1) * block_w - BOLD_BORDER_SIZE, y * block_h }, { BOLD_BORDER_SIZE, block_h }) });
							break;
						case Forward::N:
							block.thin_border.insert({ forward.first, Rect({x * block_w, y * block_h}, {block_w, THIN_BORDER_SIZE}) });
							block.bold_border.insert({ forward.first, Rect({x * block_w, y * block_h}, {block_w, BOLD_BORDER_SIZE }) });
							break;
						case Forward::W:
							block.thin_border.insert({ forward.first, Rect({x * block_w, y * block_h}, {THIN_BORDER_SIZE, block_h}) });
							block.bold_border.insert({ forward.first, Rect({x * block_w, y * block_h}, {BOLD_BORDER_SIZE, block_h}) });
							break;
						case Forward::S:
							block.thin_border.insert({ forward.first, Rect({x * block_w, (y + 1) * block_h - THIN_BORDER_SIZE}, {block_w, THIN_BORDER_SIZE}) });
							block.bold_border.insert({ forward.first, Rect({x * block_w, (y + 1) * block_h - BOLD_BORDER_SIZE}, {block_w, BOLD_BORDER_SIZE}) });
							break;
						}
					}
				}
				block.rect = Rect({ x * block_w, y * block_h }, { block_w, block_h });
			}
		}
	}

	void LoadMapImage() {

		std::vector<unsigned char> map_bmp{};
		std::ifstream file(MAP_PATH, std::ios::binary);

		file.seekg(0, std::ios_base::end);
		std::streampos file_size = file.tellg();
		map_bmp.resize(file_size);
		file.seekg(0, std::ios_base::beg);
		file.read((char*)map_bmp.data(), file_size);

		for (int x = 0; x < MAP_W; ++x)
		{
			for (int y = 0; y < MAP_H; ++y)
			{
				Block& block = board[x][y];
				auto bitmap_color = XMINT3{
					map_bmp.at(0x36 + 3 * (x + MAP_W * (MAP_H - 1 - y))),
					map_bmp.at(0x36 + 3 * (x + MAP_W * (MAP_H - 1 - y)) + 1),
					map_bmp.at(0x36 + 3 * (x + MAP_W * (MAP_H - 1 - y)) + 2)
				};
				for (auto geo : geo_list)
				{
					const auto geo_desc = geo_descs.at(geo);
					if (auto color = geo_desc.color; bitmap_color == color)
					{
						block.geo = geo;
						block.geo_desc = &geo_descs.at(geo);
						block.farm_level = geo_desc.food_level;
						block.color = {
							bitmap_color.z / 255.f,
							bitmap_color.y / 255.f,
							bitmap_color.x / 255.f,
							1 };
						break;
					}
				}
			}
		}

	}

	std::list<Block*> all_blocks = {};
	std::list<Block*>& AllBlocks() {
		if (all_blocks.size() == 0) {
			for (int x = 0; x < MAP_W; ++x)
				for (int y = 0; y < MAP_H; ++y)
					all_blocks.push_back(&board[x][y]);
		}
		return all_blocks;
	}

	std::list<Block*> tamed_all_blocks = {};
	std::list<Block*>& TamedAllBlocks() {
		if (tamed_all_blocks.size() == 0) {
			for (int x = 0; x < MAP_W; ++x)
			{
				for (int y = 0; y < MAP_H; ++y) tamed_all_blocks.push_back(&board[x][y]);
				for (int y = MAP_H - 1; y > -1; --y) tamed_all_blocks.push_back(&board[x][y]);
			}
			for (int x = MAP_W - 1; x > -1; --x)
			{
				for (int y = 0; y < MAP_H; ++y) tamed_all_blocks.push_back(&board[x][y]);
				for (int y = MAP_H - 1; y > -1; --y) tamed_all_blocks.push_back(&board[x][y]);
			}
		}
		return tamed_all_blocks;
	}

	void action_give_new_capital(Nation* me) {
		const auto& blocks = AllBlocks();

		std::list<Block*> match_blocks = {};
		for (auto& block : blocks) if (block->geo_desc->livable && block->owner == me) match_blocks.push_back(block);

		auto match_block = max_element(match_blocks.begin(), match_blocks.end(), [](Block const* lhs, Block const* rhs) { return lhs->Appraise_1() < rhs->Appraise_1(); });
		if (match_block != match_blocks.end()) {
			me->capital = *match_block;
		}
		else {
			me->willFall = true;
		}
	}

	void action_drain_troop(Nation* country, Block* location) {
		if (location->garrison == nullptr)
		{
			Troop* ptr = new Troop(country);
			ptr->ground = location;
			location->garrison = ptr;
		}
	};

	void StagePropagandaInfo() {
		for (auto& blockA : TamedAllBlocks()) {
			for (auto& neighbor : blockA->neighbors) {
				auto& blockB = neighbor.second;
				if (blockA->geo_desc->passable && blockB->geo_desc->passable) {
					blockA->OnPropaganda(blockB);
					blockB->OnPropaganda(blockA);
				}
			}
		}
	}

	void StageUpdateInfo() {
		for (auto& block : AllBlocks()) if (block->geo_desc->passable) block->OnUpdateInfo();
	}

	void StageStep() {
		for (auto& block : AllBlocks()) if (block->geo_desc->passable) block->OnStep();
		for (auto& nation : nations) nation->UpdateStat();
	};

	void MapLensPolitic() {

		for (const auto& block : AllBlocks()) {
			if (Nation* owner = block->owner; owner != nullptr)
			{
				XMFLOAT4 color = owner->color;
				V->color = color;
				V->DrawRect(block->rect.pos, block->rect.size);
				if (owner->capital == block) V->DrawImage(L"res/star.png", block->rect.pos, block->rect.size);
			}
		}
	}

	void MapLensStat(std::function<float(Block * const)> get_score, std::function<bool(Block*const)> is_target = [](Block*const) {return true; }) {
		float min = get_score(&board[0][0]);
		float max = min;

		XMFLOAT4 min_color = { 0, 0, 0 , 1 };
		XMFLOAT4 max_color = { 1, 1, 1 , 1 };
		for (const auto& block : AllBlocks()) {
			if (!is_target(block)) continue;
			float res = get_score(block);
			if (min > res) min = res;
			if (max < res) max = res;
		}
		for (const auto& block : AllBlocks()) {
			if (!is_target(block)) continue;
			float var = (get_score(block) - min) / (max - min);
			XMFLOAT4 color{};
			XMStoreFloat4(&color, XMLoadFloat4(&min_color) * (1 - var) + XMLoadFloat4(&max_color) * var);
			V->color = color;
			V->DrawRect(block->rect.pos, block->rect.size);
		}
	}

	void StageDraw() {
		float maximum = 0.0001f;
		const auto blocks = AllBlocks();
		std::list<Block*> target_blocks = {};

		for (const auto& block : AllBlocks()) {
			V->color = block->color;
			V->DrawRect(block->rect.pos, block->rect.size);
		}

		float stat_map[MAP_W][MAP_H]{};

		if (map_lens == MapLens::Politic) {
			MapLensPolitic();
			V->DrawStr({ 30, 30 }, "Politic");
		}
		else if (map_lens == MapLens::Farm) { 
			MapLensStat([=](Block* const block) {return block->farm_level; }, [=](Block* block) {return block->geo_desc->livable; }); 
			V->DrawStr({ 30, 30 }, "Farm");
		}
		else if (map_lens == MapLens::Building) { MapLensStat([=](Block* const block) {return block->building_level; }, [=](Block* block) {return block->geo_desc->livable; });
		V->DrawStr({ 30, 30 }, "Building");
		}
		else if (map_lens == MapLens::Man) { MapLensStat([=](Block* const block) {return log10f(block->man_level); }, [=](Block* block) {return block->geo_desc->livable; });
		V->DrawStr({ 30, 30 }, "Man");
		}
		else if (map_lens == MapLens::Food) { MapLensStat([=](Block* const block) {return log10f(block->food); }, [=](Block* block) {return block->geo_desc->livable; });
		V->DrawStr({ 30, 30 }, "Food");
		}
		else if (map_lens == MapLens::Product) { MapLensStat([=](Block* const block) {return log10f(block->product); }, [=](Block* block) {return block->geo_desc->livable; });
		V->DrawStr({ 30, 30 }, "Product");
		}
		else if (map_lens == MapLens::Road) { MapLensStat([=](Block* const block) {return block->road_level; }, [=](Block* block) {return block->geo_desc->livable; });
		V->DrawStr({ 30, 30 }, "Road");
		}

		for (const auto& block : AllBlocks())
		{
			for (const auto& neighbor : block->neighbors)
			{
				if ((block->geo_desc->livable != neighbor.second->geo_desc->livable && !block->geo_desc->livable) ||
					(block->geo_desc->passable != neighbor.second->geo_desc->passable && !block->geo_desc->livable)) {
					V->color = { 0.75f, 0.75f, 0.75f, 1.f };
					auto& thin_border = block->thin_border.at(neighbor.first);
					V->DrawRect(thin_border.pos, thin_border.size);
				}
				XMFLOAT4 color = {};
				if (map_lens == MapLens::Politic) {
					if (block->owner != neighbor.second->owner) {
						color = { 0, 0, 0, 1 };
					}
				}
				if (color.w > 0) {
					V->color = color;
					auto& thin_border = block->thin_border.at(neighbor.first);
					V->DrawRect(thin_border.pos, thin_border.size);
				}
			}
		}
	};

	void keyboard(int key)
	{
		switch (key)
		{
		case 'Q':
			PostQuitMessage(0);
			break;
		case VK_OEM_3:
			map_lens = MapLens::Politic;
			break;
		case '1':
			map_lens = MapLens::Farm;
			break;
		case '2':
			map_lens = MapLens::Building;
			break;
		case '3':
			map_lens = MapLens::Man;
			break;
		case '4':
			map_lens = MapLens::Food;
			break;
		case '5':
			map_lens = MapLens::Product;
			break;
		case '6':
			map_lens = MapLens::Road;
			break;
		}
	}
} game;

void main_start() {
	SetGeoInfo();
	game.MappingBlocks();
	game.LoadMapImage();
	game.SpreadNation();
};
void main_step() {
	game.StageUpdateInfo();
	game.StagePropagandaInfo();
	game.StageStep();
};
void main_draw() {
	game.StageDraw();
};
void main_end() {
};
void main_msg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_KEYDOWN) {
		game.keyboard(wParam);
	}
}