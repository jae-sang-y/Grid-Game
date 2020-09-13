#include "DXUT.h"
#include "View.h"
#include "GameData.h"
#include "GameData/Army.hpp"
#include "GameData/Block.hpp"
#include "GameData/Nation.hpp"

#include <fstream>
#include <sstream>
#include <random>
#include <time.h>

enum class MapLens {
	Politic,
	Farm, Building, Man,
	Food, Product, Road, Strategymap
};

bool operator == (XMINT3 a, XMINT3 b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

struct Game {
private:
	std::mt19937_64 rand = std::mt19937_64(time(nullptr));
	MapLens map_lens = MapLens::Politic;
	Nation* selected_nation = nullptr;
	float block_w = 1.f;
	float block_h = 1.f;
	Nation nations[MAX_NATION_COUNT]{};
	Block board[MAP_W][MAP_H] = {};
public:
	std::list<Nation*> nation_list = {};
	inline bool isOnBoard(const int& x, const int& y)
	{
		return x >= 0 && x < MAP_W&& y >= 0 && y < MAP_H;
	}

	void RebornNation() {
		for (auto& nation : nation_list) {
			if (!nation->notFall) {
				int x = rand() % MAP_W;
				int y = rand() % MAP_H;
				if (auto& block = board[x][y]; block.owner == nullptr && geo_descs[block.geo].livable && block.army == nullptr)
				{
					block.owner = nation;
					nation->capital.set(&block);
					nation->nation_size = 1;

					block.man_level = rand() % 100 + 1;
					block.road_level = 1;
					block.road_growth = 0.5f;
					block.building_level = 1;
					block.food = (float)block.man_level.data();
					block.product = (float)block.man_level.data();

					nation->notFall = true;
					break;
				}
			}
		}
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
					nations[k] = Nation(color);
					nation_list.push_back(&nations[k]);
					auto& nation = (*nation_list.rbegin());
					nation->capital.set(&block);
					nation->nation_size = 1;
					nations[k].SetSelf(nation);

					block.owner = nation;
					block.man_level = rand() % 100 + 1;
					block.road_level = 1;
					block.road_growth = 0.5f;
					block.building_level = 1;
					block.food = (float)block.man_level.data();
					block.product = (float)block.man_level.data();
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
				board[x][y].self = &block;

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
				block.base_rect = Rect({ x * block_w, y * block_h }, { block_w, block_h });
				block.geo_rect[0] = Rect({ x * block_w, y * block_h }, { block_w / 2, block_h / 2 });
				block.geo_rect[1] = Rect({ x * block_w + block_w / 2, y * block_h + block_h / 2 }, { block_w / 2, block_h / 2 });
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

		for (volatile int x = 0; x < MAP_W; ++x)
		{
			for (volatile int y = 0; y < MAP_H; ++y)
			{
				Block& block = board[x][y];
				volatile XMINT3 bitmap_color = XMINT3{
					map_bmp.at(0x36 + 3 * (x + MAP_W * (MAP_H - 1 - y))),
					map_bmp.at(0x36 + 3 * (x + MAP_W * (MAP_H - 1 - y)) + 1),
					map_bmp.at(0x36 + 3 * (x + MAP_W * (MAP_H - 1 - y)) + 2)
				};
				for (auto geo : geo_list)
				{
					const auto& geo_desc = geo_descs.at(geo);
					if (auto color = geo_desc.color; XMINT3(bitmap_color.x, bitmap_color.y, bitmap_color.z) == color)
					{
						block.geo = geo;
						block.geo_desc = &geo_descs.at(geo);
						block.farm_level = geo_desc.farm_level;
						block.base_color = {
							bitmap_color.z / 255.f,
							bitmap_color.y / 255.f,
							bitmap_color.x / 255.f,
							1 };
						block.geo_color = {
							block.base_color.x * 0.9f,
							block.base_color.y * 0.9f,
							block.base_color.z * 0.9f,
							1,
						};
						break;
					}
				}
				if (block.geo_desc == nullptr) {
					throw block.geo_desc;
				}
			}
		}
	}

	std::list<Block*> all_blocks = {};
	std::list<Block*>& AllBlocks() {
		if (all_blocks.size() == 0) {
			for (int y = 0; y < MAP_H; ++y)
				for (int x = 0; x < MAP_W; ++x)
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

	std::vector<std::vector<std::pair<Block*, Block*>>> random_all_passes = {};
	std::vector<std::pair<Block*, Block*>>& RandomAllPasses() {
		static size_t count = 0xff;
		if (random_all_passes.size() == 0) {
			std::vector<std::pair<Block*, Block*>> all_passes{};
			for (Block* block : AllBlocks()) {
				for (auto& other : block->neighbors) {
					if (other.first == Forward::E || other.first == Forward::S)
						if (block->geo_desc->passable && other.second->geo_desc->passable)
							all_passes.push_back({ block, other.second });
				}
			}
			for (size_t k = 0; k <= count; ++k)
			{
				std::shuffle(all_passes.begin(), all_passes.end(), rand);
				random_all_passes.push_back(all_passes);
			}
		}
		return random_all_passes.at(rand() % (count + 1));
	}

	void StagePropagandaInfo() {
		for (auto& pass : RandomAllPasses()) {
			auto blockA = pass.first;
			auto blockB = pass.second;
			blockA->OnPropaganda(blockB);
			blockB->OnPropaganda(blockA);
			for (auto& nation : nation_list) {
				nation->OnBlockPropaganda(blockA, blockB);
				nation->OnBlockPropaganda(blockB, blockA);
			}
		}
	}

	void StageUpdateInfo() {
		RebornNation();
		for (auto& block : AllBlocks()) if (block->geo_desc->passable) block->OnUpdateInfo();
		for (auto& nation : nation_list) nation->ClearStat();
	}

	void StageStep() {
		for (auto& block : AllBlocks()) if (block->geo_desc->passable)
			block->OnStep();
		for (auto& nation : nation_list) nation->Step();
	};

	void MapLensPolitic() {
		for (const auto& block : AllBlocks()) {
			if (Nation* owner = block->owner; owner != nullptr)
			{
				XMFLOAT4 color = owner->color;
				V->color = color;
				V->DrawRect(block->base_rect.pos, block->base_rect.size);
				if (owner->capital == block) V->DrawImage(L"res/star.png", block->base_rect.pos, block->base_rect.size);
			}
			if (Army* army = block->army; army != nullptr) {
				V->color = army->belong->color;
				std::wstring image = L"res/circle_0.png";
				for (int k = 1; k <= 10; ++k) {
					if (army->size.data() > k * 10) {
						image = L"res/circle_" + std::to_wstring(k) + L".png";
					}
				}
				if (army->size.data() <= 0) {
					image = L"res/circle.png";
				}
				V->DrawImage(image.c_str(), block->base_rect.pos, block->base_rect.size);
				if (army->energy > 15) {
					V->DrawImage(L"res/circle_fort.png", block->base_rect.pos, block->base_rect.size);
				}
			}
		}
	}

	void MapLensStat(XMFLOAT4 min_color, XMFLOAT4 max_color, std::function<float(Block*)> get_score, std::function<bool(Block* const)> is_target = [](Block*) {return true; }) {
		float min = +FLT_MAX;
		float max = -FLT_MAX;

		for (const auto& block : AllBlocks()) {
			if (!is_target(block)) continue;
			float res = get_score(block);
			if (min > res) min = res;
			if (max < res) max = res;
		}
		for (const auto& block : AllBlocks()) {
			if (!is_target(block)) {
				V->color = { 0, 0, 0, 0.5f };
				V->DrawRect(block->base_rect.pos, block->base_rect.size);
				continue;
			};
			float var = (get_score(block) - min) / (max - min);
			if (var < 0) {
				V->color = { 1, 0, 1, 1.f };
				V->DrawRect(block->base_rect.pos, block->base_rect.size);
				continue;
			};
			if (var > 1) {
				V->color = { 1, 0.5f, 1.f, 0.5f };
				V->DrawRect(block->base_rect.pos, block->base_rect.size);
				continue;
			};
			XMFLOAT4 color{};
			XMStoreFloat4(&color, XMLoadFloat4(&min_color) * (1 - var) + XMLoadFloat4(&max_color) * var);
			V->color = color;
			V->DrawRect(block->base_rect.pos, block->base_rect.size);
		}
	}

	void DrawNaturalBorder(const Block* block, const std::pair<const Forward, Block*>& neighbor) {
		if ((block->geo_desc->livable != neighbor.second->geo_desc->livable) ||
			(block->geo_desc->passable != neighbor.second->geo_desc->passable && !block->geo_desc->passable)) {
			V->color = { 0.5f, 0.5f, 0.5f, 1.f };
			auto& thin_border = block->thin_border.at(neighbor.first);
			V->DrawRect(thin_border.pos, thin_border.size);
		}
	}

	void StageDraw() {
		for (const auto& block : AllBlocks()) {
			V->color = block->base_color;
			V->DrawRect(block->base_rect.pos, block->base_rect.size);

			V->color = block->geo_color; V->DrawRect(block->geo_rect[0].pos, block->geo_rect[0].size);
			V->color = block->geo_color; V->DrawRect(block->geo_rect[1].pos, block->geo_rect[1].size);
		}

		float stat_map[MAP_W][MAP_H]{};

		if (map_lens == MapLens::Politic) {
			MapLensPolitic();
			for (auto& block : AllBlocks()) for (auto& neighbor : block->neighbors) {
				DrawNaturalBorder(block, neighbor);
				if (block->owner != neighbor.second->owner) {
					auto& thin_border = block->thin_border.at(neighbor.first);
					V->color = { 0, 0, 0, 1 };
					if (block->owner != nullptr && block->owner->HasWarWith(neighbor.second->owner)) {
						V->color = { 1, 0, 0, 1 };
					}
					V->DrawRect(thin_border.pos, thin_border.size);
				}
			}
			V->DrawStr({ 30, 30 }, "Politic");
		}
		else if (map_lens == MapLens::Farm) {
			MapLensStat(XMFLOAT4(0.5f, 0.5f, 0.5f, 1), XMFLOAT4(0, 1, 0, 1), [=](Block* const block) {return block->farm_level.data(); }, [=](Block* block) {return block->geo_desc->livable; });
			for (const auto& block : AllBlocks()) for (const auto& neighbor : block->neighbors) {
				DrawNaturalBorder(block, neighbor);
			}
			V->DrawStr({ 30, 30 }, "Farm");
		}
		else if (map_lens == MapLens::Building) {
			MapLensStat(XMFLOAT4(0, 0, 0, 1), XMFLOAT4(1, 1, 0.5f, 1), [=](Block* const block) {return block->building_level.data(); }, [=](Block* block) {return block->geo_desc->livable; });
			for (const auto& block : AllBlocks()) for (const auto& neighbor : block->neighbors) {
				DrawNaturalBorder(block, neighbor);
			}
			V->DrawStr({ 30, 30 }, "Building");
		}
		else if (map_lens == MapLens::Man) {
			MapLensStat(XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 0, 1.f, 1), [=](Block* const block) {return log10f(1 + block->man_level.data()); }, [=](Block* block) {return block->geo_desc->livable; });
			for (const auto& block : AllBlocks()) {
				if (block->man_info.origin == block->ID) V->DrawImage(L"res/star.png", block->base_rect.pos, block->base_rect.size);
			}
			for (const auto& block : AllBlocks()) for (const auto& neighbor : block->neighbors) {
				DrawNaturalBorder(block, neighbor);
				if (block->man_info.origin != neighbor.second->man_info.origin) {
					auto& bold_border = block->bold_border.at(neighbor.first);
					V->color = { 1, 0, 1, 1 };
					V->DrawRect(bold_border.pos, bold_border.size);
				}
			}
			V->DrawStr({ 30, 30 }, "Man");
		}
		else if (map_lens == MapLens::Food) {
			MapLensStat(XMFLOAT4(0, 0, 0, 1), XMFLOAT4(0, 1, 0, 1), [=](Block* const block) {return log10f(1 + block->food.data()); }, [=](Block* block) {return block->geo_desc->livable; });
			for (const auto& block : AllBlocks()) {
				if (block->farm_info.origin == block->ID) V->DrawImage(L"res/star.png", block->base_rect.pos, block->base_rect.size);
			}
			for (const auto& block : AllBlocks()) for (const auto& neighbor : block->neighbors) {
				DrawNaturalBorder(block, neighbor);
				if (block->farm_info.origin != neighbor.second->farm_info.origin) {
					auto& bold_border = block->bold_border.at(neighbor.first);
					V->color = { 1, 0, 1, 1 };
					V->DrawRect(bold_border.pos, bold_border.size);
				}
			}
			V->DrawStr({ 30, 30 }, "Food");
		}
		else if (map_lens == MapLens::Product) {
			MapLensStat(XMFLOAT4(1, 1, 1, 1), XMFLOAT4(1, 0.75f, 0, 1), [=](Block* const block) {return log10f(1 + block->product.data()); }, [=](Block* block) {return block->geo_desc->livable; });
			for (const auto& block : AllBlocks()) {
				if (block->build_info.origin == block->ID) V->DrawImage(L"res/star.png", block->base_rect.pos, block->base_rect.size);
			}
			for (const auto& block : AllBlocks()) for (const auto& neighbor : block->neighbors) {
				DrawNaturalBorder(block, neighbor);
				if (block->build_info.origin != neighbor.second->build_info.origin) {
					auto& bold_border = block->bold_border.at(neighbor.first);
					V->color = { 1, 0, 1, 1 };
					V->DrawRect(bold_border.pos, bold_border.size);
				}
			}
			V->DrawStr({ 30, 30 }, "Product");
		}
		else if (map_lens == MapLens::Road) {
			MapLensStat(XMFLOAT4(1, 0, 0, 1), XMFLOAT4(0, 0, 1, 1), [=](Block* const block) {return block->road_level.data(); }, [=](Block* block) {return block->geo_desc->passable; });
			for (const auto& block : AllBlocks()) for (const auto& neighbor : block->neighbors) {
				DrawNaturalBorder(block, neighbor);
			}
			V->DrawStr({ 30, 30 }, "Road");
		}
		else if (map_lens == MapLens::Strategymap) {
			//MapLensPolitic();
			for (auto& block : AllBlocks()) for (auto& neighbor : block->neighbors) {
				DrawNaturalBorder(block, neighbor);
				if (block->owner != neighbor.second->owner && block->owner != nullptr) {
					auto& thin_border = block->thin_border.at(neighbor.first);
					V->color = block->owner->color;
					V->DrawRect(thin_border.pos, thin_border.size);
				}
			}
			V->DrawStr({ 30, 30 }, "Strategymap");
			if (selected_nation != nullptr) {
				for (const auto& block : AllBlocks()) {
					for (auto& neighbor : block->neighbors) {
						if (selected_nation->strategy_map[block->X][block->Y].prop.origin != selected_nation->strategy_map[neighbor.second->X][neighbor.second->Y].prop.origin) {
							auto& bold_border = block->bold_border.at(neighbor.first);
							V->color = { 0, 0, 0, 1 };
							V->DrawRect(bold_border.pos, bold_border.size);
						}
					}
					if (block->geo_desc->passable && selected_nation->strategy_map[block->X][block->Y].prop.origin == block->ID) V->DrawImage(L"res/star.png", block->base_rect.pos, block->base_rect.size);
					if (!selected_nation->strategy_map[block->X][block->Y].passable) {
						V->color = { 1, 0, 1, 0.75f };
						V->DrawRect(block->base_rect.pos, block->base_rect.size);
					}
				}
			}
		}

		//auto& p = V->GetMousePos();
		//V->DrawRect(p, { 32, 32 });
	};

	void mousedown() {
		auto mouse = V->GetMousePos();
		int bx = mouse.x / block_w;
		int by = mouse.y / block_h;
		if (isOnBoard(bx, by)) {
			selected_nation = board[bx][by].owner;
		}
	}

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
		case '7':
			map_lens = MapLens::Strategymap;
			break;
		}
	}
} game;

std::list<Block*> all_blocks() {
	return game.all_blocks;
}
std::list<Nation*> all_nations() {
	return game.nation_list;
}
Nation* Nation::nation(int target) {
	return *std::next(game.nation_list.begin(), target);
}

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
	else if (uMsg == WM_LBUTTONDOWN) {
		game.mousedown();
	}
}