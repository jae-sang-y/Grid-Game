#pragma warning(push)
#include "DXUT.h"
#pragma warning(pop)

#include <wrl/client.h>
#include "main.hpp"

using namespace std;
using Microsoft::WRL::ComPtr;

inline bool isOnBoard(const int& x, const int& y)
{
	return x >= 0 && x < MAP_W && y >= 0 && y < MAP_H;
}

void mouse_callback(int events, int x, int y, int flags, void* userdata)
{
	(*(function<void(int, int, int, int)>*)userdata)(events, x, y, flags);
}

enum class MapLens {
	Politic,
	Farm, Building, Man,
	Food, Product, Road
};

struct DX {
};
static std::unique_ptr<DX> dx = nullptr;

using std::string;

class Game {
private:
	mt19937_64 mt = mt19937_64(time(nullptr));

	MapLens map_lens = MapLens::Politic;
	Nation* selected_nation = nullptr;
	//Mat sprite_capital = imread("res/capital.png", CV_32FC4);
public:
	int day = 0, month = 0, year = 222;
	Block board[MAP_W][MAP_H] = {};
	std::list<Nation> nations = {};
	const int window_w = BLOCK_SIZE * MAP_W;
	const int window_h = BLOCK_SIZE * MAP_H;

	float factor_farm_output = 1.f;

	Game(Game&) = delete;
	Game(Game&&) = delete;
	Game() {
		SetGeoInfo();

		MappingBlocks();
		LoadMapImage();
		AllBlocks();
		TamedAllBlocks();
		SpreadNation();
	}

	void SpreadNation() {
		for (int k = 0; k < MAX_NATION_COUNT; ++k)
		{
			do
			{
				int x = mt() % MAP_W;
				int y = mt() % MAP_H;

				if (auto& block = board[x][y]; block.owner == nullptr && geo_descs[block.geo].livable)
				{
					float r = (mt() % 1000) / 1000.f,
						g = (mt() % 1000) / 1000.f,
						b = (mt() % 1000) / 1000.f;
					float m = max(max(r, g), b);
					r /= m;
					g /= m;
					b /= m;
					auto nation = Nation(int(r * 220), int(g * 220), int(b * 220));
					nation.capital = &block;
					nations.push_back(nation);

					block.owner = &(*nations.rbegin());
					block.man_level = 100;
					block.food = 100;
					block.product = 100;
					break;
				}
			} while (true);
		}
	}

	void MappingBlocks() {
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
							//block.thin_border.insert({ forward.first, Rect((x + 1) * BLOCK_SIZE - THIN_BORDER_SIZE, y * BLOCK_SIZE, THIN_BORDER_SIZE, BLOCK_SIZE) });
							//block.bold_border.insert({ forward.first, Rect((x + 1) * BLOCK_SIZE - BOLD_BORDER_SIZE, y * BLOCK_SIZE, BOLD_BORDER_SIZE, BLOCK_SIZE) });
							break;
						case Forward::N:
							//block.thin_border.insert({ forward.first, Rect(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, THIN_BORDER_SIZE) });
							//block.bold_border.insert({ forward.first, Rect(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BOLD_BORDER_SIZE) });
							break;
						case Forward::W:
							//block.thin_border.insert({ forward.first, Rect(x * BLOCK_SIZE, y * BLOCK_SIZE, THIN_BORDER_SIZE, BLOCK_SIZE) });
							//block.bold_border.insert({ forward.first, Rect(x * BLOCK_SIZE, y * BLOCK_SIZE, BOLD_BORDER_SIZE, BLOCK_SIZE) });
							break;
						case Forward::S:
							//block.thin_border.insert({ forward.first, Rect(x * BLOCK_SIZE, (y + 1) * BLOCK_SIZE - THIN_BORDER_SIZE, BLOCK_SIZE, THIN_BORDER_SIZE) });
							//block.bold_border.insert({ forward.first, Rect(x * BLOCK_SIZE, (y + 1) * BLOCK_SIZE - BOLD_BORDER_SIZE, BLOCK_SIZE, BOLD_BORDER_SIZE) });
							break;
						}
					}
				}

				//block.rect = Rect(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE);
				//Mat geo_rect = Mat(geo_img, block.rect);
			}
		}
	}

	void LoadMapImage() {
		//Mat bitmap = imread("res/map.jpg");
		//assert(bitmap.cols == BOARD_W && bitmap.rows == BOARD_H);
		for (int x = 0; x < MAP_W; ++x)
		{
			for (int y = 0; y < MAP_H; ++y)
			{
				Block& block = board[x][y];
				//Mat geo_rect = Mat(geo_img, block.rect);
				//auto& bitmap_color = bitmap.at<Vec3b>(y, x);
				for (auto geo : geo_list)
				{
					const auto geo_desc = geo_descs.at(geo);
					//if (auto color = geo_desc.color; bitmap_color == color)
					{
						block.geo = geo;
						block.geo_desc = &geo_descs.at(geo);
						block.farm_level = geo_desc.food_level;
						//geo_rect = Scalar(color[0], color[1], color[2]);
						//Mat(geo_rect, Rect(BLOCK_SIZE / 2, 0, BLOCK_SIZE / 2, BLOCK_SIZE / 2)) = Scalar(color[0] - 40, color[1] - 40, color[2] - 40);
						//Mat(geo_rect, Rect(0, BLOCK_SIZE / 2, BLOCK_SIZE / 2, BLOCK_SIZE / 2)) = Scalar(color[0] - 40, color[1] - 40, color[2] - 40);
						break;
					}
				}
			}
		}

		for (auto& block : AllBlocks()) {
			for (auto& neighbor : block->neighbors) {
				if ((block->geo_desc->livable != neighbor.second->geo_desc->livable && !block->geo_desc->livable) ||
					(block->geo_desc->passable != neighbor.second->geo_desc->passable && !block->geo_desc->livable)) {
					//Mat{ geo_img, block->thin_border.at(neighbor.first) } = Scalar(0.75f, 0.75f, 0.75f, 1.f);
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

	void UpdateDate() {
		++day;
		if (day > 30) {
			day = 0;
			++month;
			if (month > 11) {
				month = 0; ++year;
				factor_farm_output = max(5 - fabsf(10.f - month), 0.f) + 1;
			}
		}
	}

	void StageStep() {
		for (auto& block : AllBlocks()) if (block->geo_desc->passable) block->OnStep();
		for (Nation& nation : nations) nation.UpdateStat();
		UpdateDate();
	};

	void StageDraw() {
		//final_img = Scalar(80, 45, 40);
		//addWeighted(final_img, 0, geo_img, 1, 0, final_img);

		//con_img = Scalar(0, 0, 0, 0);
		//UI_img = Scalar(0, 0, 0, 0);

		float maximum = 0.0001f;
		const auto blocks = AllBlocks();
		std::list<Block*> target_blocks = {};

		switch (map_lens)
		{
		case  MapLens::Farm:
		{
			if (selected_nation == nullptr) maximum = FARM_LEVEL_MAX;
			else {
				copy_if(blocks.begin(), blocks.end(), target_blocks.begin(), [=](Block* block) { return block->owner == selected_nation; });
				const auto Pr = [=](Block const * lhs, Block const * rhs) {return lhs->farm_level < rhs->farm_level; };
				maximum = (float)(*max_element(blocks.begin(), blocks.end(), Pr))->farm_level;
			}

		}
		break;
		case  MapLens::Building:
		{
			if (selected_nation == nullptr) maximum = BUILDING_LEVEL_MAX;
			else {
				copy_if(blocks.begin(), blocks.end(), target_blocks.begin(), [=](Block* block) { return block->owner == selected_nation; });
				const auto Pr = [=](Block const * lhs, Block const * rhs) {return lhs->building_level < rhs->building_level; };
				maximum = (float)(*max_element(blocks.begin(), blocks.end(), Pr))->building_level;
			}
		}
		break;
		case  MapLens::Man:
		{
			if (selected_nation == nullptr) maximum = log10f(10 + MAN_LEVEL_MAX);
			else {
				copy_if(blocks.begin(), blocks.end(), target_blocks.begin(), [=](Block* block) { return block->owner == selected_nation; });
				const auto Pr = [=](Block const * lhs, Block const * rhs) {return lhs->man_level < rhs->man_level; };
				maximum = log10f(10 + (float)(*max_element(blocks.begin(), blocks.end(), Pr))->man_level);
			}
		}
		break;
		case MapLens::Food:
		{
			if (selected_nation == nullptr) target_blocks = blocks;
			else copy_if(blocks.begin(), blocks.end(), target_blocks.begin(), [=](Block* block) { return block->owner == selected_nation; });
			const auto Pr = [=](Block const * lhs, Block const * rhs) {return lhs->food < rhs->food; };
			maximum = log10f(10 + (float)(*max_element(blocks.begin(), blocks.end(), Pr))->food);
		}
		break;
		case MapLens::Product:
		{
			if (selected_nation == nullptr) target_blocks = blocks;
			else copy_if(blocks.begin(), blocks.end(), target_blocks.begin(), [=](Block* block) { return block->owner == selected_nation; });
			const auto Pr = [](Block const * lhs, Block const * rhs) {return lhs->product < rhs->product; };
			maximum = log10f(10 + (float)(*max_element(blocks.begin(), blocks.end(), Pr))->product);
		}
		break;
		case MapLens::Road:
		{
			if (selected_nation == nullptr) maximum = ROAD_LEVEL_MAX;
			else copy_if(blocks.begin(), blocks.end(), target_blocks.begin(), [=](Block* block) { return block->owner == selected_nation; });
			const auto Pr = [](Block const * lhs, Block const * rhs) {return lhs->road_level < rhs->road_level; };
			maximum = (float)(*max_element(blocks.begin(), blocks.end(), Pr))->road_level;
		}
		break;
		}
		for (const auto& block : AllBlocks())
		{
			if (block->geo_desc->livable)
			{
				auto res = XMColorRGBToHSV(XMVectorSet(1, 0, 0, 1));
				XMFLOAT3 res2;
				XMStoreFloat3(&res2, res);
				XMFLOAT4 rgba = XMFLOAT4(0, 0, 0, 0);
				if (map_lens == MapLens::Politic)
				{
					if (Nation* owner = block->owner; owner != nullptr)
						rgba = XMFLOAT4(owner->r, owner->g, owner->b, 255);
				}
				else if (map_lens == MapLens::Farm) {
					float color_factor = block->farm_level / maximum;
					auto new_color = MAP_LENS_FARM_MIN * (1 - color_factor) + MAP_LENS_FARM_MAX * color_factor;
					XMStoreFloat4(&rgba, new_color);
					if (color_factor < 0) { rgba = { 255, 0, 255, 255 }; }
					else if (color_factor > 1) { rgba = { 255, 255, 0, 255 }; }
					else  rgba.w = 255;
				}
				else if (map_lens == MapLens::Building) {
					float color_factor = block->building_level / maximum;
					auto new_color = MAP_LENS_BUILDING_MIN * (1 - color_factor) + MAP_LENS_BUILDING_MAX * color_factor;
					XMStoreFloat4(&rgba, new_color);
					if (color_factor < 0) { rgba = { 255, 0, 255, 255 }; }
					else if (color_factor > 1) { rgba = { 255, 255, 0, 255 }; }
					else  rgba.w = 255;
				}
				else if (map_lens == MapLens::Man) {
					float color_factor = log10f(10.f + block->man_level) / maximum;
					auto new_hsv = MAP_LENS_HSV_MIN * (1 - color_factor) + MAP_LENS_HSV_MAX * color_factor;
					auto new_rgb = XMColorHSVToRGB(new_hsv);
					XMStoreFloat4(&rgba, new_rgb * 255);
					if (color_factor < 0) { rgba = { 255, 0, 255, 255 }; }
					else if (color_factor > 1) { rgba = { 255, 255, 0, 255 }; }
					else  rgba.w = 255;
				}
				else if (map_lens == MapLens::Food) {
					float color_factor = log10f(10 + block->food) / maximum;
					auto new_hsv = MAP_LENS_HSV_MIN * (1 - color_factor) + MAP_LENS_HSV_MAX * color_factor;
					auto new_rgb = XMColorHSVToRGB(new_hsv);
					XMStoreFloat4(&rgba, new_rgb * 255);
					if (color_factor < 0) { rgba = { 255, 0, 255, 255 }; }
					else if (color_factor > 1) { rgba = { 255, 255, 0, 255 }; }
					else  rgba.w = 255;
				}
				else if (map_lens == MapLens::Product) {
					float color_factor = log10f(10 + block->product) / maximum;
					auto new_hsv = MAP_LENS_HSV_MIN * (1 - color_factor) + MAP_LENS_HSV_MAX * color_factor;
					auto new_rgb = XMColorHSVToRGB(new_hsv);
					XMStoreFloat4(&rgba, new_rgb * 255);

					if (color_factor < 0) { rgba = { 255, 0, 255, 255 }; }
					else if (color_factor > 1) { rgba = { 255, 255, 0, 255 }; }
					else  rgba.w = 255;
				}
				//else if (showmode == MapLens::Building) Blend(Hex(0xfcfcb5), Hex(0x050505), IntoRange((float)block->building_level, 0, maximum), r, g, b);
				//else if (showmode == MapLens::Man) Blend(Hex(0xffffff), Hex(0x0000ff), IntoRange(log10f((float)block->man_level), 0, maximum), r, g, b);
				//else if (showmode == MapLens::Food) Blend(Hex(0x028c00), Hex(0xa5a5a5), IntoRange(block->food, 0, maximum), r, g, b);
				//else if (showmode == MapLens::Product) Blend(Hex(0xef8009), Hex(0xa5a5a5), IntoRange(block->product, 0, maximum), r, g, b);
				//else if (showmode == MapLens::Road) Blend(Hex(0x7f7f7f), Hex(0xffffff), IntoRange((float)block->road_level, 0, maximum), r, g, b);
				//if (rgba.w > 0) Mat(con_img, block->rect) = Scalar(rgba.z, rgba.y, rgba.x, rgba.w);
			}
			if (block->garrison != nullptr)
			{
				if (map_lens == MapLens::Politic)
				{
					if (block->garrison->owner->capital != block)
					{
						//circle(UI_img, Point(int(BLOCK_SIZE * (block->X + 0.5f)), int(BLOCK_SIZE * (block->Y + 0.5f))), int(BLOCK_SIZE * 0.35f), Scalar(block->garrison->owner->b, block->garrison->owner->g, block->garrison->owner->r, 255), FILLED);
						//circle(UI_img, Point(int(BLOCK_SIZE * (block->X + 0.5f)), int(BLOCK_SIZE * (block->Y + 0.5f))), int(BLOCK_SIZE * 0.35f), Scalar(block->garrison->owner->b / 2, block->garrison->owner->g / 2, block->garrison->owner->r / 2, 255), 2, LINE_AA);
						if (block->garrison->forward != Forward::None)
						{
							//line(UI_img, Point(int(BLOCK_SIZE * (block->X + 0.5f)), int(BLOCK_SIZE * (block->Y + 0.5f))), Point(int(BLOCK_SIZE * (block->X + 0.5f + 0.5f * forward_dir.at(block->garrison->forward).x)), int(BLOCK_SIZE * (block->Y + 0.5f + 0.5f * forward_dir.at(block->garrison->forward).y))), Scalar(block->garrison->owner->b / 2, block->garrison->owner->g / 2, block->garrison->owner->r / 2, 255), 2, LINE_AA);
						}
						if (block->garrison->energy == 30)
						{
							//line(UI_img, Point(int(BLOCK_SIZE * (block->X + 0.5f)), int(BLOCK_SIZE * (block->Y + 0.0f))), Point(int(BLOCK_SIZE * (block->X + 0.0f)), int(BLOCK_SIZE * (block->Y + 0.5f))), Scalar(block->garrison->owner->b / 2, block->garrison->owner->g / 2, block->garrison->owner->r / 2, 255), 1, LINE_AA);
							//line(UI_img, Point(int(BLOCK_SIZE * (block->X + 1.0f)), int(BLOCK_SIZE * (block->Y + 0.5f))), Point(int(BLOCK_SIZE * (block->X + 0.5f)), int(BLOCK_SIZE * (block->Y + 0.0f))), Scalar(block->garrison->owner->b / 2, block->garrison->owner->g / 2, block->garrison->owner->r / 2, 255), 1, LINE_AA);
							//line(UI_img, Point(int(BLOCK_SIZE * (block->X + 0.5f)), int(BLOCK_SIZE * (block->Y + 1.0f))), Point(int(BLOCK_SIZE * (block->X + 1.0f)), int(BLOCK_SIZE * (block->Y + 0.5f))), Scalar(block->garrison->owner->b / 2, block->garrison->owner->g / 2, block->garrison->owner->r / 2, 255), 1, LINE_AA);
							//line(UI_img, Point(int(BLOCK_SIZE * (block->X + 0.0f)), int(BLOCK_SIZE * (block->Y + 0.5f))), 	Point(int(BLOCK_SIZE * (block->X + 0.5f)), int(BLOCK_SIZE * (block->Y + 1.0f))), Scalar(block->garrison->owner->b / 2, block->garrison->owner->g / 2, block->garrison->owner->r / 2, 255), 1, LINE_AA);
						}
					}
				}
			}
		}

		string text;
		switch (map_lens)
		{
		case MapLens::Politic:
			text = "Politic";
			break;
		case MapLens::Farm:
			text = "Farm Level";
			break;
		case MapLens::Building:
			text = "Building Level";
			break;
		case MapLens::Man:
			text = "Man";
			break;
		case MapLens::Food:
			text = "Food";
			break;
		case MapLens::Product:
			text = "Product";
			break;
		case MapLens::Road:
			text = "Road";
			break;
		}
		for (const auto& block : AllBlocks())
		{
			for (const auto& neighbor : block->neighbors)
			{
				int r = 0;
				int g = 0;
				int b = 0;
				int a = 0;
				if (map_lens == MapLens::Politic) {
					if (auto owner = block->owner; owner != nullptr) {
						r = owner->r;
						g = owner->g;
						b = owner->b;
						a = 255;
					}
				}
				//if (a > 0) { Mat(con_img, block->bold_border.at(neighbor.first)) = Scalar(b, g, r, a); }
			}
		}
		//putText(UI_img, text, Point(882, 332), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 0, 255), 2, 8, false);
		//putText(UI_img, text, Point(880, 330), FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255, 255), 2, 8, false);

		char buf[256] = {};
		sprintf_s(buf, "%s %2d/AD %4d", Month_Name[month].c_str(), day, year);

		//putText(UI_img, buf, Point(882, 392), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 0, 255), 2, 8, false);
		//putText(UI_img, buf, Point(880, 390), FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255, 255), 2, 8, false);

		/*Concurrency::parallel_for(0, final_img.cols, [&](int x) {
			for (int y = 0; y < final_img.rows; ++y)
			{
				auto& target = final_img.at<Vec3b>(y, x);

				//Blend ConMap
				{
					auto pixel = con_img.at<Vec4f>(y, x);
					float alpha = pixel[3] / 255, inv_alpha = 1 - alpha;
					target[0] = saturate_cast<unsigned char>(pixel[0] * alpha + target[0] * inv_alpha);
					target[1] = saturate_cast<unsigned char>(pixel[1] * alpha + target[1] * inv_alpha);
					target[2] = saturate_cast<unsigned char>(pixel[2] * alpha + target[2] * inv_alpha);
					pixel = UI_img.at<Vec4f>(y, x);
					alpha = pixel[3] / 255, inv_alpha = 1 - alpha;
					target[0] = saturate_cast<unsigned char>(pixel[0] * alpha + target[0] * inv_alpha);
					target[1] = saturate_cast<unsigned char>(pixel[1] * alpha + target[1] * inv_alpha);
					target[2] = saturate_cast<unsigned char>(pixel[2] * alpha + target[2] * inv_alpha);
				}
			}
			});

		return final_img;*/
	};

	/*function<void(int, int, int, int)> mouse = [&](int events, int mouse_x, int mouse_y, int flags) {
		switch (events)
		{
		case EVENT_LBUTTONDOWN:
		{
			int x = mouse_x / BLOCK_SIZE;
			int y = mouse_y / BLOCK_SIZE;

			selected_nation = board[x][y].owner;
		}break;
		}
	};*/

	void keyboard(int key)
	{
		switch (key)
		{
		case 'q':
			PostQuitMessage(0);
			break;
		case '`':
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
};
static Game* game;


void Block::ChangeOwnerByContract(Nation* new_owner) {
	Block* block = this;

	if (Nation* pre_owner = block->owner; pre_owner != nullptr)
	{
		if (block == pre_owner->capital)
		{
			game->action_give_new_capital(pre_owner);
		}
	}
	block->owner = new_owner;
}

void Block::ChangeOwnerByForce(Nation* new_owner) {
	Block* block = this;
	if (Nation* pre_owner = block->owner; pre_owner != nullptr)
	{
		if (block == pre_owner->capital)
		{
			if (owner != nullptr)
			{
				game->action_give_new_capital(pre_owner);
			}
		}
		if (owner != nullptr)
		{
			int spoiled_man = (int)floorf(block->man_level * SPOIL_PROPORTION);
			int spoiled_food = (int)floorf(block->food * SPOIL_PROPORTION);
			int spoiled_product = (int)floorf(block->product * SPOIL_PROPORTION);

			owner->capital->man_level += spoiled_man;
			owner->capital->food += spoiled_food;
			owner->capital->product += spoiled_product;

			block->man_level -= spoiled_man;
			block->food -= spoiled_food;
			block->product -= spoiled_product;
		}
	}
}

void Nation::UpdateStat() {

}

void Nation::UpdateStrategyMap() {

}

void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext) {
	game->StageUpdateInfo();
	game->StagePropagandaInfo();
	game->StageStep();
}
void CALLBACK OnD3D10FrameRender(ID3D10Device* D, double fTime, float fElapsedTime, void* pUserContext) {
	D->ClearDepthStencilView(DXUTGetD3D10DepthStencilView(), D3D10_CLEAR_DEPTH, 1.f, 0);
	game->StageDraw();
}
HRESULT CALLBACK OnD3D10CreateDevice(ID3D10Device* D, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext) {
	return S_OK;
}
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext) {
	return 0;
}
void CALLBACK OnD3D10DestroyDevice(void* pUserContext) {

}
HRESULT CALLBACK OnD3D10ResizedSwapChain(ID3D10Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext) {
	return S_OK;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	game = new Game;

	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackFrameMove(OnFrameMove);

	DXUTSetCallbackD3D10DeviceCreated(OnD3D10CreateDevice);
	DXUTSetCallbackD3D10SwapChainResized(OnD3D10ResizedSwapChain);
	DXUTSetCallbackD3D10DeviceDestroyed(OnD3D10DestroyDevice);
	DXUTSetCallbackD3D10FrameRender(OnD3D10FrameRender);

	DXUTInit(true, true, NULL);
	DXUTSetCursorSettings(true, true);
	DXUTCreateWindow(L"myGame");
	DXUTCreateDevice(true, game->window_w, game->window_h);
	DXUTMainLoop();
	delete game;
	return DXUTGetExitCode();
}