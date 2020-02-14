#include "main.hpp"

using namespace cv;
using namespace std;

inline bool isOnBoard(const int& x, const int& y)
{
	return x >= 0 && x < board_w && y >= 0 && y < board_h;
}

void mouse_callback(int events, int x, int y, int flags, void* userdata)
{
	(*(function<void(int, int, int, int)>*)userdata)(events, x, y, flags);
}

extern class Game;
static Game* mainGame;
class Game {
private:
	mt19937_64 mt = mt19937_64(time(nullptr));

	std::list<Block*> board_data, board4_data;
	std::array<std::array<Block*, board_h>, board_w> board;
	std::array<Country*, MAX_COUNTRY_COUNT> countries;
	std::list<Country*> countries_data;

	std::array<std::array<Relation*, MAX_COUNTRY_COUNT>, MAX_COUNTRY_COUNT> relations;
	std::list<Relation*> relations_data;

	Mat final_img = Mat(board_h * block_size, board_w * block_size, CV_8UC3);
	Mat geo_img = Mat(board_h * block_size, board_w * block_size, CV_8UC3);
	Mat con_img = Mat(board_h * block_size, board_w * block_size, CV_32FC4);
	Mat UI_img = Mat(board_h * block_size, board_w * block_size, CV_32FC4);

	int showmode = 0;
	int day = 0, month = 0, year = 230;
	int* GameRun = nullptr;
	Country* select_country = nullptr;
public:
	Game() = delete;
	Game(Game&&) = delete;
	Game(int* GameRun) : GameRun(GameRun) {
		assert(mainGame == nullptr);
		mainGame = this;

		Mat map = imread("map.jpg");
		assert(map.cols == board_w && map.rows == board_h);

		for (int x = 0; x < board_w; ++x)
		{
			for (int y = 0; y < board_h; ++y)
			{
				Block* ptr = new Block(x, y);
				board_data.push_back(ptr);
				board[x][y] = ptr;
			}
		}

		for (int x = 0; x < board_w; ++x)
		{
			for (int y = 0; y < board_h; ++y)
				board4_data.push_back(board[x][y]);
			for (int y = board_h - 1; y > -1; --y)
				board4_data.push_back(board[x][y]);
		}
		for (int x = board_w - 1; x > -1; --x)
		{
			for (int y = 0; y < board_h; ++y)
				board4_data.push_back(board[x][y]);
			for (int y = board_h - 1; y > -1; --y)
				board4_data.push_back(board[x][y]);
		}

		int W[4][2] = { {1, 0}, {0, -1}, {-1, 0}, {0, 1} };
		for (int x = 0; x < board_w; ++x)
		{
			for (int y = 0; y < board_h; ++y)
			{
				Block* block = board[x][y];

				//Add Neighbor
				for (int w = 0; w < 4; ++w)
				{
					int x2 = x + W[w][0];
					int y2 = y + W[w][1];

					if (isOnBoard(x2, y2))
					{
						block->neighbor.push_back(board[x2][y2]);
						switch (w)
						{
						case 0:
							block->small_border.push_back(Rect((x + 1) * block_size - small_border_size, y * block_size, small_border_size, block_size));
							block->big_border.push_back(Rect((x + 1) * block_size - big_border_size, y * block_size, big_border_size, block_size));
							break;
						case 1:
							block->small_border.push_back(Rect(x * block_size, y * block_size, block_size, small_border_size));
							block->big_border.push_back(Rect(x * block_size, y * block_size, block_size, big_border_size));
							break;
						case 2:
							block->small_border.push_back(Rect(x * block_size, y * block_size, small_border_size, block_size));
							block->big_border.push_back(Rect(x * block_size, y * block_size, big_border_size, block_size));
							break;
						case 3:
							block->small_border.push_back(Rect(x * block_size, (y + 1) * block_size - small_border_size, block_size, small_border_size));
							block->big_border.push_back(Rect(x * block_size, (y + 1) * block_size - big_border_size, block_size, big_border_size));
							break;
						}
					}
				}

				//Get GeoData from GeoImage
				block->rect = Rect(x * block_size, y * block_size, block_size, block_size);
				Mat geo_rect = Mat(geo_img, block->rect);
				auto geo = map.at<Vec3b>(y, x);
				for (int i = 0; i < Geo::Max; ++i)
				{
					if (auto color = Geo_color[i]; geo == color)
					{
						block->geo = (Geo)i;
						block->farm_level = Geo_Food_Level[i];
						geo_rect = Scalar(color[0], color[1], color[2]);
						Mat(geo_rect, Rect(block_size / 2, 0, block_size / 2, block_size / 2)) = Scalar(color[0] - 40, color[1] - 40, color[2] - 40);
						Mat(geo_rect, Rect(0, block_size / 2, block_size / 2, block_size / 2)) = Scalar(color[0] - 40, color[1] - 40, color[2] - 40);
						break;
					}
				}
			}
		}

		for (const auto& blockA : board_data)
		{
			int i = 0;
			for (const auto& blockB : blockA->neighbor)
			{
				//Draw Geo Border
				if (Geo_Livable[blockA->geo] != Geo_Livable[blockB->geo])
				{
					Mat(geo_img, blockA->small_border[i]) = Scalar(0, 0, 0);
				}
				++i;
			}
		}
		for (int i = 0; i < MAX_COUNTRY_COUNT; ++i)
		{
			do
			{
				int x = mt() % board_w;
				int y = mt() % board_h;

				if (Block* block = board[x][y]; block->owner == nullptr && Geo_Livable[block->geo] && block->geo == Geo::River)
				{
					float r = (mt() % 1000) / 1000.f, g = (mt() % 1000) / 1000.f, b = (mt() % 1000) / 1000.f;
					float m = max(max(r, g), b);
					r /= m;
					g /= m;
					b /= m;
					Country* ptr = new Country(i, r * 220, g * 220, b * 220);
					ptr->capital = block;
					countries[i] = ptr;
					countries_data.push_back(ptr);

					block->owner = ptr;
					block->man_level = 10 + mt() % 989;
					block->food = 10 + mt() % 244;
					block->product = 10 + mt() % 244;
					break;
				}
			} while (true);
		}
		for (int i = 0; i < MAX_COUNTRY_COUNT; ++i)
		{
			Country* countryA = countries[i];
			for (int j = 0; j < MAX_COUNTRY_COUNT; ++j)
			{
				Country* countryB = countries[j];
				if (i != j)
				{
					Relation* ptr = new Relation(countryA, countryB);
					relations[i][j] = ptr;
					relations_data.push_back(ptr);
				}
			}
		}
	}
	~Game() {
		mainGame = nullptr;

		for (auto block : board_data) delete block;
		for (auto country : countries_data) delete country;
		for (auto relation : relations_data) delete relation;
	}

	void action_give_new_capital(Country* country) {
		float gage = 0;
		Block* new_capital = nullptr;
		for (const auto& blockA : board_data)
		{
			if (Geo_Livable[blockA->geo] && blockA->owner == country)
			{
				if (float value =
					blockA->build_level + blockA->farm_level + blockA->man_level
					; value > gage)
				{
					gage = value;
					new_capital = blockA;
				}
			}
		}
		if (new_capital == nullptr)
			event_fall_country(country);
		else
			country->capital = new_capital;
	}

	void action_drain_troop(Country* country, Block* location) {
		if (location->garrison == nullptr)
		{
			Troop* ptr = new Troop(country);
			ptr->ground = location;
			++country->stat_troop_size;
			location->garrison = ptr;
		}
	};

	void event_fall_country(Country* countryA) {
		countryA->isExist = false;
		for (int i = 0; i < MAX_COUNTRY_COUNT; ++i)
		{
			if (i != countryA->ID)
			{
				relations[countryA->ID][i]->Clear();
				relations[i][countryA->ID]->Clear();
				if (relations[i][countryA->ID]->isWar)
				{
					countries[i]->ClearPriority();
				}
			}
		}
	}

	void event_take_block_peacely(Block* location, Country* pre_owner, Country* owner) {
		if (pre_owner != nullptr)
		{
			--pre_owner->stat_demesne_size;
			if (location == pre_owner->capital)
			{
				action_give_new_capital(pre_owner);
			}
		}
		if (owner != nullptr)
		{
			++owner->stat_demesne_size;
		}
		for (Country* country : countries_data)
		{
			if (country->isExist)
				country->priority_power[location->X][location->Y] = 0;
		}
	}

	void event_conquer_block(Block* location, Country* pre_owner, Country* owner) {
		if (pre_owner != nullptr)
		{
			--pre_owner->stat_demesne_size;
			if (location == pre_owner->capital)
			{
				if (owner != nullptr)
				{
					if (pre_owner->stat_troop_size == 0 && pre_owner->stat_demesne_size * 10 < owner->stat_demesne_size)
					{
						location->ruin = 1000;
						++owner->stat_demesne_size;

						owner->capital->man_level += location->man_level * 4 / 5;
						owner->capital->food += location->food * 4 / 5;
						owner->capital->product += location->product * 4 / 5;

						location->man_level /= 5;
						location->food /= 5;
						location->product /= 5;

						for (Block* blockA : board_data)
						{
							if (blockA->owner == pre_owner)
							{
								blockA->owner = owner;
							}
							if (blockA->garrison != nullptr && blockA->garrison->owner == pre_owner)
							{
								blockA->garrison->owner = owner;
							}
						}

						event_fall_country(pre_owner);
					}
					else if (pre_owner->stat_troop_size == 0 && pre_owner->stat_war_opponent == 1)
					{
						int peace_time = 360;

						location->ruin = 1000;
						++owner->stat_demesne_size;

						owner->capital->man_level += location->man_level * 4 / 5;
						owner->capital->food += location->food * 4 / 5;
						owner->capital->product += location->product * 4 / 5;

						location->man_level /= 5;
						location->food /= 5;
						location->product /= 5;

						for (Block* blockA : board_data)
						{
							if (blockA->owner == pre_owner)
							{
								if (owner->capital_power[blockA->X][blockA->Y] >
									pre_owner->capital_power[blockA->X][blockA->Y] ||
									blockA->info_loaf != pre_owner->capital->info_loaf)
								{
									blockA->owner = owner;
									event_take_block_peacely(blockA, pre_owner, owner);
									++peace_time;
								}
							}
							else if (blockA->owner == owner)
							{
								if (owner->capital_power[blockA->X][blockA->Y] <
									pre_owner->capital_power[blockA->X][blockA->Y] && blockA->ruin > 0)
								{
									blockA->owner = pre_owner;
									event_take_block_peacely(blockA, owner, pre_owner);
									--peace_time;
								}
							}
						}

						if (peace_time > 1500) peace_time = 1500;

						Relation* relationAB = relations[owner->ID][pre_owner->ID];
						Relation* relationBA = relations[pre_owner->ID][owner->ID];

						relationAB->isWar = false;
						relationBA->isWar = false;
						relationAB->isAccessable = false;
						relationBA->isAccessable = false;
						relationAB->peace = peace_time / 30;
						relationBA->peace = peace_time / 30;

						action_give_new_capital(pre_owner);
						return;
					}
					else
					{
						action_give_new_capital(pre_owner);
					}
				}
				else
				{
					action_give_new_capital(pre_owner);
				}
			}
			location->ruin = 1000;
			if (owner != nullptr)
			{
				++owner->stat_demesne_size;

				location->man_level /= 3;
				location->food /= 3;
				location->product /= 3;
			}
		}
		if (owner != nullptr)
		{
			++owner->stat_demesne_size;
		}
		for (Country* country : countries_data)
		{
			if (country->isExist)
				country->priority_power[location->X][location->Y] = 0;
		}
	}

	void info_spread(Block* blockA, Block* blockB) {
		if (blockA->owner != nullptr && blockA->owner == blockB->owner)
		{
			if (blockA->info_loaf < blockB->info_loaf)
			{
				blockB->info_loaf = blockA->info_loaf;
			}
		}

		for (Country* country : countries_data)
		{
			if (!country->isExist) continue;
			if (Geo_Accessable[blockB->geo])
			{
				if (country->access[blockA->X][blockA->Y] < 65535)
				{
					int value = 0;
					if (blockB->owner == nullptr)
					{
						value = country->access[blockA->X][blockA->Y];
					}
					else if (blockB->owner != country)
					{
						if (blockA->owner == blockB->owner)
						{
							value = country->access[blockA->X][blockA->Y];
						}
						else if (blockB->owner != country)
						{
							if (relations[country->ID][blockB->owner->ID]->isAccessable)
							{
								value = country->access[blockA->X][blockA->Y];
							}
							else
							{
								value = country->access[blockA->X][blockA->Y] + 1;
							}
						}
						else
						{
							value = country->access[blockA->X][blockA->Y];
						}
					}
					else
					{
						value = -1;
					}

					if (value < country->access[blockB->X][blockB->Y] && value >= 0)
						country->access[blockB->X][blockB->Y] = value;
				}
				//Set Distance
				if (country->distance_from_border[blockA->X][blockA->Y] + 1 <
					country->distance_from_border[blockB->X][blockB->Y])
				{
					country->distance_from_border[blockB->X][blockB->Y] = country->distance_from_border[blockA->X][blockA->Y] + 1;
				}
				if (float effect = country->capital_power[blockA->X][blockA->Y] * max(0.f, min(0.999f, Geo_Mho[blockA->geo] + INFO_SPREAD_FACTOR_BUILD * blockA->build_level));
					effect > country->capital_power[blockB->X][blockB->Y])
				{
					country->capital_power[blockB->X][blockB->Y] = effect;
				}

				//Info War
				float& powerA = country->priority_power[blockA->X][blockA->Y];
				int& rankA = country->priority_rank[blockA->X][blockA->Y];

				float& powerB = country->priority_power[blockB->X][blockB->Y];
				int& rankB = country->priority_rank[blockB->X][blockB->Y];

				if ((rankA > rankB || (rankA == rankB && powerA > powerB) || country->priority_time[blockB->X][blockB->Y] == 0) &&
					!(blockA->owner != nullptr && blockA->owner != country && (!(relations[country->ID][blockA->owner->ID]->isAccessable))) &&
					Geo_Accessable[blockB->geo]
					)
				{
					if (float effect = powerA * max(0.999f, Geo_Mho[blockA->geo] + INFO_SPREAD_FACTOR_BUILD * blockA->build_level) - 0.01f; effect > 0)
					{
						rankB = rankA;
						powerB = effect;
						country->priority_time[blockB->X][blockB->Y] = country->priority_time[blockA->X][blockA->Y];
					}
				}
			}
		}

		//Info Spread or Moving
		if (blockA->info_farm_rank > blockB->info_farm_rank)
		{
			if (float effect = blockA->info_farm_power * max(0.f, min(0.999f, Geo_Mho[blockA->geo] + INFO_SPREAD_FACTOR_BUILD * blockA->build_level)) - INFO_FARM_POWER_DECREASE_PER_STEP; effect > 0)
			{
				blockB->info_farm_rank = blockA->info_farm_rank;
				blockB->info_farm_power = effect;
			}
		}
		if (int quantity = 1;
			(blockA->info_farm_rank <= blockB->info_farm_rank &&
				blockA->man_level > quantity&&
				blockB->man_level + 1 < blockB->build_level * 4) || blockA->man_level > 1000)
		{
			blockA->manpower *= (blockA->man_level - 1.f) / blockA->man_level;
			blockA->man_level -= quantity;
			blockB->man_level += quantity;
		}

		if (blockA->info_build_rank > blockB->info_build_rank)
		{
			if (float effect = blockA->info_build_power * max(0.f, min(0.999f, Geo_Mho[blockA->geo] + INFO_SPREAD_FACTOR_BUILD * blockA->build_level)) - INFO_BUILD_POWER_DECREASE_PER_STEP; effect > 0)
			{
				blockB->info_build_rank = blockA->info_build_rank;
				blockB->info_build_power = effect;
			}
		}
		if (int quantity = 1;
			(blockA->info_build_rank <= blockB->info_build_rank &&
				blockA->man_level > quantity&&
				blockB->man_level + 1 < blockB->build_level * 4) || blockA->man_level > 1000)
		{
			blockA->manpower *= (blockA->man_level - 1.f) / blockA->man_level;
			blockA->man_level -= quantity;
			blockB->man_level += quantity;
		}

		if (blockA->info_man_rank > blockB->info_man_rank)
		{
			if (float effect = blockA->info_man_power * max(0.f, min(0.999f, Geo_Mho[blockA->geo] + INFO_SPREAD_FACTOR_BUILD * blockA->build_level)) - INFO_MAN_POWER_DECREASE_PER_STEP; effect > 0)
			{
				blockB->info_man_rank = blockA->info_man_rank;
				blockB->info_man_power = effect;
			}
		}
		if (blockA->info_man_rank <= blockB->info_man_rank || blockA->food > 255 || blockA->product > 255)
		{
			if (float quantity = 0.004f * blockB->build_level;
				(blockA->farm_level > 1 &&
					blockB->food + quantity < 255) || blockA->food > 255)
			{
				blockA->food -= quantity;
				blockB->food += quantity;
			}
			if (float quantity = 0.1f + 0.008f * blockB->build_level;
				(blockA->build_level > 1 &&
					blockB->product + quantity < 255 && blockA->product) || blockA->product > 255)
			{
				blockA->product -= quantity;
				blockB->product += quantity;
			}
		}
	};
	void info() {
		for (const auto& blockA : board_data)
		{
			blockA->info_loaf = blockA->ID;
			for (Country* country : countries_data)
			{
				if (!country->isExist) continue;
				++country->distance_from_border[blockA->X][blockA->Y];
				if (blockA->owner == country)
				{
					for (const auto& blockB : blockA->neighbor)
					{
						if (blockB->owner != blockA->owner && blockB->owner != nullptr &&
							Geo_Accessable[blockB->geo] && relations[blockA->owner->ID][blockB->owner->ID]->peace < 12)
						{
							country->distance_from_border[blockA->X][blockA->Y] = 0;
							break;
						}
					}
				}

				country->capital_power[blockA->X][blockA->Y] /= 2.f;
				country->capital_power[blockA->X][blockA->Y] -= 0.5f;
				if (blockA == country->capital)
				{
					country->capital_power[blockA->X][blockA->Y] = 100 +
						9 * ((1 + blockA->build_level) +
						(1 + blockA->farm_level) +
							(1 + blockA->man_level / 64));
				}
				else if (country->capital_power[blockA->X][blockA->Y] < 0)
				{
					country->capital_power[blockA->X][blockA->Y] = 0;
				}
				//Set Block Value
				float power = 0.1f;
				int rank = 0;
				bool force = false;

				if (blockA->owner == country)
				{
					country->access[blockA->X][blockA->Y] = 0;
					if (country->stat_war_opponent == 0)
					{
						if (blockA->owner->capital == blockA)
						{
							rank = 2;
							power = 1.1f;
						}
						else
						{
							rank = 1;
							power = 1.1f;
						}
					}
				}
				else
				{
					country->access[blockA->X][blockA->Y] = 65535;
					if (blockA->owner != nullptr)
					{
						if (relations[country->ID][blockA->owner->ID]->isWar)
						{
							if (blockA->owner->capital == blockA)
							{
								rank = 4;
								power = 1.1f;
							}
							else if (blockA->garrison != nullptr && blockA->garrison->owner == blockA->owner)
							{
								rank = 5;
								power = 1.1f;
							}
							else
							{
								rank = 3;
								power = 1.1f;
							}
						}
						/*else
						{
							rank = 0;
							power = 0.1f * board_w * board_h;
						}*/
					}
					/*else
					{
						rank = 0;
						power = 0.1f * board_w * board_h;
					}*/
				}

				float& prio_power = country->priority_power[blockA->X][blockA->Y];
				int& prio_rank = country->priority_rank[blockA->X][blockA->Y];
				if (force || prio_rank < rank || (prio_rank == rank && prio_power < power) || prio_power < 0 ||
					country->priority_time[blockA->X][blockA->Y] == 0)
				{
					prio_rank = rank;
					prio_power = power;
					country->priority_time[blockA->X][blockA->Y] = 5;
				}
				else if (country->priority_time[blockA->X][blockA->Y] > 0)
				{
					--country->priority_time[blockA->X][blockA->Y];
				}
			}

			bool isCapital = false;
			if (blockA->owner != nullptr && blockA->owner->capital == blockA) isCapital = true;

			float power = blockA->farm_level * blockA->farm_level;
			int rank = blockA->farm_level + (isCapital) ? 3 : 0;
			//Set Info Food
			if (blockA->info_farm_rank < rank ||
				(blockA->info_farm_rank == rank && blockA->info_farm_power < power) ||
				blockA->info_farm_power < 0)
			{
				blockA->info_farm_rank = rank;
				blockA->info_farm_power = power;
			}
			else
			{
				blockA->info_farm_power -= INFO_FARM_POWER_DECREASE_PER_STEP;
			}

			power = blockA->build_level * blockA->build_level;
			rank = blockA->build_level + (isCapital) ? 3 : 0;
			//Set Info Eco
			if (blockA->info_build_rank < rank ||
				(blockA->info_build_rank == rank && blockA->info_build_power < power) ||
				blockA->info_build_power < 0)
			{
				blockA->info_build_rank = rank;
				blockA->info_build_power = power;
			}
			else
			{
				blockA->info_build_power -= INFO_BUILD_POWER_DECREASE_PER_STEP;
			}

			power = blockA->man_level / 4;
			rank = blockA->man_level + (isCapital) ? 3 : 0;
			//Set Info Man
			if (blockA->info_man_rank < rank ||
				(blockA->info_man_rank == rank && blockA->info_man_power < power) ||
				blockA->info_man_power < 0)
			{
				blockA->info_man_rank = rank;
				blockA->info_man_power = power;
			}
			else
			{
				blockA->info_man_power -= INFO_MAN_POWER_DECREASE_PER_STEP;
			}
		}

		for (Block* blockA : board4_data)
			for (Block* blockB : blockA->neighbor)
				info_spread(blockA, blockB);
	};

	void action_declareWar(Country* countryA, Country* countryB, Relation* relationAB, Relation* relationBA)
	{
		static int i = 0;
		cout << "War Declared! - " << ++i << endl;
		relationAB->isWar = true;
		relationBA->isWar = true;
		relationAB->isAccessable = true;
		relationBA->isAccessable = true;
		relationAB->peace = 12;
		relationBA->peace = 12;
		relationAB->WarDuration = 0;
		relationBA->WarDuration = 0;

		++countryA->stat_war_opponent;
		++countryB->stat_war_opponent;
	}

	void diplomacy()
	{
		//Diplomacy Stat Clear
		for (Country* country : countries_data)
		{
			if (!country->isExist) continue;
			country->stat_war_opponent = 0;
			country->stat_war_opponent_troop_size = 0;
		}

		//Diplomacy Stat Set
		for (int i = 0; i < MAX_COUNTRY_COUNT; ++i)
		{
			Country* countryA = countries[i];
			if (!countryA->isExist) continue;
			for (int j = 0; j < MAX_COUNTRY_COUNT; ++j)
			{
				Country* countryB = countries[j];
				if (!countryB->isExist) continue;
				if (i != j)
				{
					Relation* relationAB = relations[i][j];
					Relation* relationBA = relations[j][i];
					if (relationAB->isWar || relationBA->isWar)
					{
						++countryA->stat_war_opponent;
						++relationAB->WarDuration;
						countryA->stat_war_opponent_troop_size += countryB->stat_demesne_size;
					}
				}
			}
		}

		for (int i = 0; i < MAX_COUNTRY_COUNT; ++i)
		{
			Country* countryA = countries[i];
			if (!countryA->isExist) continue;
			float war_gage = -9999;
			Country* war_target = nullptr;
			for (int j = 0; j < MAX_COUNTRY_COUNT; ++j)
			{
				Country* countryB = countries[j];
				if (!countryB->isExist) continue;
				if (i != j)
				{
					Relation* relationAB = relations[i][j];
					Relation* relationBA = relations[j][i];

					if (relationAB->isWar)
					{
						if (relationAB->WarDuration > 90 && countryA->stat_troop_size > countryB->stat_troop_size
							&& relationAB->peace == 0)
						{
							relationAB->isWar = false;
							relationBA->isWar = false;
							relationAB->isAccessable = false;
							relationBA->isAccessable = false;

							--countryA->stat_war_opponent;
							--countryB->stat_war_opponent;

							relationAB->peace = 12;
							relationBA->peace = 12;
						}
						else if (((countryA->stat_troop_size < 2 && countryB->stat_troop_size < 2)
							|| countryA->access[countryB->capital->X][countryB->capital->Y] >= 1)
							&& relationAB->peace == 0)
						{
							relationAB->isWar = false;
							relationBA->isWar = false;
							relationAB->isAccessable = false;
							relationBA->isAccessable = false;

							--countryA->stat_war_opponent;
							--countryB->stat_war_opponent;

							relationAB->peace = 12;
							relationBA->peace = 12;
						}
					}
					else if (
						countryB->stat_war_opponent == 0)
					{
						if (relationAB->peace == 0 &&
							(relationAB->isNeighbor || countryA->access[countryB->capital->X][countryB->capital->Y] <= 1) &&
							countryA->stat_troop_size - countryA->stat_war_opponent_troop_size
						>
							(countryB->stat_troop_size) &&
							countryA->stat_sum_ruin <= 0
							)
						{
							float value = (relationAB->isNeighbor ? 1 : 0) - 10 /
								(countryB->stat_troop_size + countryB->trained_troop / 10) * 0.1f
								- countryA->distance_from_border[countryB->capital->X][countryB->capital->Y] * 20;
							if (value > war_gage || war_target == nullptr) {
								war_gage = value;
								war_target = countryB;
							}
						}
					}
				}
			}

			if (war_target != nullptr)
			{
				Relation* relationAB = relations[i][war_target->ID];
				Relation* relationBA = relations[war_target->ID][i];
				action_declareWar(countryA, war_target, relationAB, relationBA);
			}
		}
	};

	void step() {
		++day;
		if (day > 30) {
			day = 0;
			++month;
			if (month > 11) { month = 0; ++year; }
			FARM_OUTPUT_FACTOR = max(5 - fabsf(10 - month), 0.f) + 1;
		}

		//Stat Clear
		for (Country* country : countries_data)
		{
			country->stat_demesne_size = 0;
			country->stat_troop_size = 0;
			country->stat_sum_ruin = 0;
		}
		for (Relation* relation : relations_data)
		{
			relation->isNeighbor = false;
			if (day == 1)
			{
				if (relation->peace > 0) relation->peace -= 1;
			}
		}
		//Stat Set
		for (const auto& blockA : board_data)
		{
			if (Geo_Livable[blockA->geo])
			{
				if (blockA->owner != nullptr)
				{
					++blockA->owner->stat_demesne_size;
				}
				if (blockA->garrison != nullptr)
				{
					++blockA->garrison->owner->stat_troop_size;
				}
			}
			if (blockA->owner != nullptr)
			{
				blockA->owner->stat_sum_ruin += blockA->ruin;

				for (const auto& blockB : blockA->neighbor)
				{
					if (blockB->owner != nullptr && blockA->owner != blockB->owner)
					{
						relations[blockA->owner->ID][blockB->owner->ID]->isNeighbor = true;
					}
				}
			}
		}
		for (Country* country : countries_data)
		{
			if (!country->isExist) {
				for (Block* blockA : board_data)
				{
					if (blockA->owner == nullptr && Geo_Livable[blockA->geo] && blockA->garrison == nullptr && month == country->ID % 12 && mt() % (board_w * board_h) == 0)
					{
						country->capital = blockA;
						blockA->owner = country;
						if (blockA->man_level < 1) blockA->man_level = 1;
						event_take_block_peacely(blockA, nullptr, country);
						country->isExist = true;
						break;
					}
				}
			}
			if (day == country->ID % 31) country->trained_troop += country->stat_demesne_size / 36.f + 1;
			if (int max = country->stat_demesne_size / 9 + 10; country->trained_troop > max) country->trained_troop = max;
		}

		for (const auto& blockA : board_data)
		{
			if (Geo_Livable[blockA->geo])
			{
				//Ruin Reduce
				if (blockA->ruin > 0 && blockA->owner != nullptr) {
					if (blockA->owner->stat_war_opponent == 0)
						blockA->ruin -= 9;
				}

				if (blockA->owner != nullptr)
				{
					if (blockA->man_level == 0)
					{
						Country* pre_owner = blockA->owner;
						blockA->owner = nullptr;
						event_conquer_block(blockA, pre_owner, nullptr);
					}
					else //Drain
					{
						++blockA->owner->stat_demesne_size;
						if (blockA->garrison == nullptr &&
							blockA->owner->stat_troop_size < blockA->owner->stat_demesne_size / 4 + 1 &&
							blockA->man_level > 60 &&
							blockA->owner->trained_troop >= 10 &&
							blockA->owner->capital_power[blockA->X][blockA->Y] > 200 &&
							mt() % 256 == 0)
						{
							blockA->man_level -= 20;
							blockA->owner->trained_troop -= 10;
							action_drain_troop(blockA->owner, blockA);
						}
					}
				}

				//Food Gain
				for (int gage = blockA->farm_level; gage >= 0 && blockA->ruin <= 0; --gage)
				{
					if (float cost_manpower = (gage > 0) ? (gage - 1) * (gage - 1) * FARM_CONSUME_MANPOWER_PER_STEP : 0,
						cost_product = (gage > 0) ? (gage - 1) * (gage - 1) * FARM_CONSUME_PRODUCT_PER_STEP : 0;
						blockA->manpower >= cost_manpower && blockA->product >= cost_product)
					{
						blockA->food += gage * gage * FOOD_GAIN_PER_STEP * FARM_OUTPUT_FACTOR;
						blockA->manpower -= cost_manpower;
						blockA->product -= cost_product;
						break;
					}
				}
				if (blockA->food < 0)
				{
					blockA->food += blockA->farm_level;
					if (blockA->food < 0) blockA->food = 0;
					blockA->farm_level -= 1;
					if (blockA->farm_level < Geo_Food_Level[blockA->geo]) blockA->farm_level = Geo_Food_Level[blockA->geo];
				}

				//Manpower Gain
				for (int gage = blockA->man_level; gage >= 0 && blockA->ruin <= 0 && blockA->owner != nullptr; --gage)
				{
					if (float cost_food = gage * MAN_CONSUME_FOOD_PER_STEP;
						blockA->food >= cost_food)
					{
						blockA->manpower += gage * MANPOWER_GAIN_PER_STEP;
						blockA->food -= cost_food;
						break;
					}
				}
				if (blockA->manpower > blockA->man_level) blockA->manpower = blockA->man_level;

				//Product Gain
				for (int gage = blockA->product; gage >= 0 && blockA->ruin <= 0 && blockA->owner != nullptr; --gage)
				{
					if (float cost_manpower = gage * BUILD_CONSUME_MANPOWER_PER_STEP;
						blockA->manpower >= cost_manpower)
					{
						blockA->product += (9 - (16 - gage) * (16 - gage) / 30.f) * 1.0f;
						blockA->manpower -= cost_manpower;
						break;
					}
				}
				if (blockA->product < 0)
				{
					blockA->product += blockA->build_level + 0.1f;
					if (blockA->product < 0) blockA->product = 0;
					blockA->build_level -= 1;
					if (blockA->build_level < 1) blockA->build_level = 1;
				}

				//Upgrade level
				if (float cost = blockA->man_level * MAN_UPGRADE_COST_FACTOR;
					blockA->food > cost&&
					blockA->man_level < MAN_LEVEL_MAX && blockA->owner != nullptr)
				{
					blockA->food -= cost;
					blockA->man_level += 1;
				}

				if (float cost = 50 + blockA->farm_level * blockA->farm_level * 4;
					blockA->manpower > cost&&
					blockA->farm_level < FARM_LEVEL_MAX && blockA->owner != nullptr)
				{
					blockA->manpower -= cost;
					blockA->farm_level += 1;
				}

				if (float cost_product = 1 + blockA->build_level * blockA->build_level * blockA->build_level / 3,
					cost_manpower = 60 + 53 * blockA->build_level;
					blockA->manpower > cost_manpower&&
					blockA->product > cost_product&&
					blockA->build_level < BUILD_LEVEL_MAX && blockA->owner != nullptr)
				{
					blockA->manpower -= cost_manpower;
					blockA->product -= cost_product;
					blockA->build_level += 1;
				}
			}
		}

		for (Country* country : countries_data)
		{
			if (country->stat_demesne_size > 0)
				country->isExist = true;
			if (country->isExist && country->capital->man_level > 40 && country->stat_war_opponent == 0 &&
				country->capital->food > 60)
			{
				Block* nearest = nullptr;
				float gage = 0;
				for (const auto& blockA : board_data)
				{
					if (blockA->owner == nullptr && Geo_Livable[blockA->geo]
						&& country->capital_power[blockA->X][blockA->Y] > 100 &&
						country->access[blockA->X][blockA->Y] == 0)
					{
						float value = country->capital_power[blockA->X][blockA->Y] / 150 + 2 - country->distance_from_border[blockA->X][blockA->Y];
						if (value > gage)
						{
							gage = value;
							nearest = blockA;
						}
					}
				}
				if (nearest != nullptr) //Colonize
				{
					country->capital->man_level -= 20;
					country->capital->food -= 60;
					nearest->man_level += 50;
					nearest->food += 60;
					nearest->owner = country;
					event_take_block_peacely(nearest, nullptr, country);
				}
			}
		}

		info();

		//Move Troop
		for (const auto& blockA : board_data)
		{
			int i = 0;
			int unit_forward = -1;
			float priority_powerA = 0;
			int priority_rankA = 0;
			int distance_from_border = 0;
			bool onUnaccessable = false;
			if (blockA->garrison != nullptr)
			{
				if (day == 0) --blockA->garrison->size;
				if (blockA->garrison->size > 0)
				{
					const Country* ownerA = blockA->garrison->owner;
					priority_powerA = ownerA->priority_power[blockA->X][blockA->Y];
					priority_rankA = ownerA->priority_rank[blockA->X][blockA->Y];
					distance_from_border = ownerA->distance_from_border[blockA->X][blockA->Y];
					if (blockA->garrison->energy < 30) {
						++blockA->garrison->energy;
						if (blockA->ruin <= 0)
							++blockA->garrison->energy;
					}
					else if (blockA->garrison->energy < 0) blockA->garrison->energy = 0;

					if (blockA->garrison->owner != blockA->owner && blockA->owner != nullptr)
					{
						if (relations[blockA->garrison->owner->ID][blockA->owner->ID]->isWar)
						{
							relations[blockA->garrison->owner->ID][blockA->owner->ID]->WarDuration = 0;
							relations[blockA->owner->ID][blockA->garrison->owner->ID]->WarDuration = 0;

							Country* pre_owner = blockA->owner;
							blockA->owner = blockA->garrison->owner;
							--blockA->garrison->size;
							blockA->garrison->energy = 0;

							event_conquer_block(blockA, pre_owner, blockA->owner);
						}
						else if (!relations[blockA->garrison->owner->ID][blockA->owner->ID]->isAccessable)
						{
							onUnaccessable = true;
						}
					}
				}
				else
				{
					delete blockA->garrison;
					blockA->garrison = nullptr;
				}
			}

			for (const auto& blockB : blockA->neighbor)
			{
				/*if (blockA->owner != nullptr)
				{
					if (Geo_Livable[blockB->geo])
					{
						if (blockB->owner == nullptr)
						{
							//Colonize
							if (int over = blockA->man_level - blockA->build_level * 64;
								over > 0 && mt() % 10000 < over)
							{
								Country* pre_owner = blockB->owner;
								blockB->owner = blockA->owner;
								event_take_block_peacely(blockB, pre_owner, blockA->owner);

								blockB->man_level += 1;
								blockA->man_level -= 1;
							}
						}
					}
				}*/
				//if (Geo_Livable[blockA->geo] && Geo_Livable[blockB->geo])

				if (blockA->garrison != nullptr)
				{
					if (onUnaccessable)
					{
						bool isAble = Geo_Accessable[blockB->geo] && blockB->garrison == nullptr;
						if (isAble)
						{
							float value = distance_from_border - blockA->garrison->owner->distance_from_border[blockB->X][blockB->Y];
							if (value > priority_rankA)
							{
								priority_rankA = value;
								unit_forward = i;
							}
						}
					}
					else
					{
						bool isAble = Geo_Accessable[blockB->geo];

						//Foregin Demsne
						if (isAble && blockB->owner != nullptr && blockA->garrison->owner != blockB->owner)
						{
							Relation* relationAB = relations[blockA->garrison->owner->ID][blockB->owner->ID];
							if (!relationAB->isAccessable)
							{
								isAble = false;
							}
						}

						if (isAble)
						{
							float priority_powerB = blockA->garrison->owner->priority_power[blockB->X][blockB->Y];
							int priority_rankB = blockA->garrison->owner->priority_rank[blockB->X][blockB->Y];
							if ((priority_rankB == priority_rankA && priority_powerB > priority_powerA) ||
								(priority_rankB > priority_rankA)
								)
							{
								unit_forward = i;
								priority_rankA = priority_rankB;
								priority_powerA = priority_powerB;
							}
						}
					}
				}
				++i;
			}

			if (blockA->garrison != nullptr && unit_forward != -1 && blockA->garrison->energy > 3 / max(0.001f, min(0.999f, Geo_Mho[blockA->geo] + INFO_SPREAD_FACTOR_BUILD * blockA->build_level)))
			{
				if (blockA->neighbor[unit_forward]->garrison == nullptr)
				{
					Troop* ptr = blockA->garrison;
					blockA->garrison = nullptr;
					ptr->ground = nullptr;
					ptr->energy = 0;
					Block* blockB = blockA->
						neighbor[unit_forward];
					blockB->garrison = ptr;
					ptr->ground = blockB;
					ptr = 0;
				}
				else if (Block* blockB = blockA->neighbor[unit_forward]; blockA->garrison->owner != blockB->garrison->owner)
				{
					float damageA = blockB->garrison->size / 10 + 1;
					float damageB = blockA->garrison->size / 10 + 1;

					blockA->garrison->size -= damageA;
					blockB->garrison->size -= damageB;
				}
			}
		}

		diplomacy();
	};
	const Mat& draw() {
		final_img = Scalar(80, 45, 40);
		addWeighted(final_img, 0, geo_img, 1, 0, final_img);

		con_img = Scalar(0, 0, 0, 0);
		UI_img = Scalar(0, 0, 0, 0);

		float max = 0.0001f;

		switch (showmode)
		{
		case 1:
			if (select_country != nullptr)
			{
				for (const auto& blockA : board_data)
					if (const auto gage = blockA->farm_level; gage > max) max = gage;
			}
			else max = FARM_LEVEL_MIN;

			break;
		case 2:
			if (select_country != nullptr)
			{
				for (const auto& blockA : board_data)
					if (const auto gage = blockA->build_level; gage > max) max = gage;
			}
			else max = BUILD_LEVEL_MAX;
			break;
		case 3:
			if (select_country != nullptr)
			{
				for (const auto& blockA : board_data)
					if (const auto gage = blockA->man_level; gage > max) max = gage;
			}
			else max = MAN_LEVEL_MAX;
			break;
		case 4:
			if (select_country != nullptr)
			{
				for (const auto& blockA : board_data)
					if (const auto gage = blockA->food; gage > max) max = gage;
			}
			else max = 255;
			break;
		case 5:
			if (select_country != nullptr)
			{
				for (const auto& blockA : board_data)
					if (const auto gage = blockA->product; gage > max) max = gage;
			}
			else max = 255;
			break;
		case 6:
			if (select_country != nullptr)
			{
				for (const auto& blockA : board_data)
				{
					if (const auto gage = select_country->priority_rank[blockA->X][blockA->Y];
						gage > max) max = gage;
				}
			}
			break;
		case 7:
			if (select_country != nullptr)
			{
				for (const auto& blockA : board_data)
				{
					if (const auto gage = select_country->distance_from_border[blockA->X][blockA->Y];
						gage > max) max = gage;
				}
			}
			break;
		case 8:
			if (select_country != nullptr)
			{
				for (const auto& blockA : board_data)
				{
					if (const auto gage = select_country->capital_power[blockA->X][blockA->Y];
						gage > max) max = gage;
				}
			}
			break;
		}
		for (const auto& blockA : board_data)
		{
			if (Geo_Livable[blockA->geo])
			{
				float r = 0;
				float g = 0;
				float b = 0;
				float a = 255;
				if (showmode == 0)
				{
					if (Country* temp = blockA->owner; temp != nullptr)
					{
						r = temp->r;
						g = temp->g;
						b = temp->b;
						a = 250;
					}
					else
					{
						a = 0;
					}
				}
				else if (showmode == 1)
				{
					Blend(Hex(0x07fc44), Hex(0x9b964c),
						IntoRange(blockA->farm_level, FARM_LEVEL_MIN, max),
						r, g, b);
				}
				else if (showmode == 2)
				{
					Blend(Hex(0xfcfcb5), Hex(0x050505),
						IntoRange(blockA->build_level, BUILD_LEVEL_MIN, max),
						r, g, b);
				}
				else if (showmode == 3)
				{
					if (blockA->man_level > MAN_LEVEL_MIN)
					{
						Blend(Hex(0x544bfc), Hex(0xf7f7f7),
							IntoRange(blockA->man_level, MAN_LEVEL_MIN, max),
							r, g, b);
					}
					else a = 0;
				}
				else if (showmode == 4)
				{
					Blend(Hex(0x028c00), Hex(0xa5a5a5),
						IntoRange(blockA->food, 0, max),
						r, g, b);
				}
				else if (showmode == 5)
				{
					Blend(Hex(0xef8009), Hex(0xa5a5a5),
						IntoRange(blockA->product, 0, max),
						r, g, b);
				}
				else if (showmode == 6)
				{
					if (select_country != nullptr)
					{
						Blend(Hex(0xffffff), Hex(0x000000),
							IntoRange(select_country->priority_rank[blockA->X][blockA->Y], 0, max),
							r, g, b);
					}
				}
				else if (showmode == 7)
				{
					if (select_country != nullptr)
					{
						//Blend(Hex(0x000000), Hex(0xffffff), IntoRange(select_country->distance_from_border[blockA->X][blockA->Y], 0, max), r, g, b);

						if (select_country->access[blockA->X][blockA->Y] == 0)
						{
							g = 255;
						}
						else if (select_country->access[blockA->X][blockA->Y] == 1)
						{
							r = 255;
							g = 255;
						}
						else if (select_country->access[blockA->X][blockA->Y] > 1)
						{
							r = 255;
						}
					}
				}
				else if (showmode == 8)
				{
					if (select_country != nullptr)
					{
						Blend(Hex(0xffffff), Hex(0x000000), IntoRange(select_country->capital_power[blockA->X][blockA->Y], 0, max), r, g, b);
						if (select_country->capital_power[blockA->X][blockA->Y] > 100)
							r = 255;
					}
				}
				else if (showmode == 9)
				{
					if (blockA->owner != nullptr)
					{
						r = (blockA->info_loaf * 2 / 10) % 256;
						g = (blockA->info_loaf * 3 / 10) % 256;
						b = (blockA->info_loaf * 5 / 10) % 256;
					}
				}

				if (a) Mat(con_img, blockA->rect) = Scalar(b, g, r, a);
			}
			if (blockA->garrison != nullptr)
			{
				if (showmode == 0 || showmode == 6)
				{
					if (blockA->garrison->owner->capital != blockA)
					{
						circle(UI_img, Point(block_size * (blockA->X + 0.5f), block_size * (blockA->Y + 0.5f)), block_size * 0.35f, Scalar(blockA->garrison->owner->b, blockA->garrison->owner->g, blockA->garrison->owner->r, 255), FILLED);
						circle(UI_img, Point(block_size * (blockA->X + 0.5f), block_size * (blockA->Y + 0.5f)), block_size * 0.35f, Scalar(blockA->garrison->owner->b / 2, blockA->garrison->owner->g / 2, blockA->garrison->owner->r / 2, 255), 2, LINE_AA);
					}
				}
			}
		}
		for (const auto& blockA : board_data)
		{
			int i = 0;
			for (const auto& blockB : blockA->neighbor)
			{
				int r = 0;
				int g = 0;
				int b = 0;
				int a = 0;
				if (showmode == 0 || showmode == 6 || showmode == 7 || showmode == 8 || showmode == 9)
				{
					if (blockA->owner != blockB->owner)
					{
						if (Country* countryA = blockA->owner; countryA != nullptr)
						{
							if (Country* countryB = blockB->owner; countryB != nullptr)
							{
								Relation* relationAB = relations[countryA->ID][countryB->ID];
								if (relationAB->isWar)
								{
									r = 255;
									g = 0;
									b = 0;
									a = 255;
								}
								else if (relationAB->peace)
								{
									r = 0;
									g = 255;
									b = 255;
									a = 255;
								}
								else if (countryA->stat_war_opponent > 0)
								{
									r = 255;
									g = 255;
									b = 0;
									a = 255;
								}
								else if (countryA->stat_sum_ruin > 0)
								{
									r = 0;
									g = 0;
									b = 0;
									a = 255;
								}
								else
								{
									r = 127;
									g = 127;
									b = 127;
									a = 255;
								}
							}
							else
							{
								r = countryA->r / 2;
								g = countryA->g / 2;
								b = countryA->b / 2;
								a = 255;
							}
						}
					}
				}
				if (a) Mat(con_img, blockA->big_border[i]) = Scalar(b, g, r, a);
				++i;
			}
		}

		string text;
		switch (showmode)
		{
		case 0:
			text = "Politic";
			for (const Country* country : countries_data)
			{
				if (!country->isExist) continue;
				if (country->stat_demesne_size > 0)
				{
					if (Block* block = country->capital; block != nullptr)
					{
						fillConvexPoly(con_img, std::vector<Point>{
							Point((block->X + 0.5f + 0.2f * cosf(CV_PI * 1 / 10))* block_size, (block->Y + 0.5f + 0.2f * sinf(CV_PI * 1 / 10))* block_size),
								Point((block->X + 0.5f + 0.4f * cosf(CV_PI * 3 / 10))* block_size, (block->Y + 0.5f + 0.4f * sinf(CV_PI * 3 / 10))* block_size),
								Point((block->X + 0.5f + 0.2f * cosf(CV_PI * 5 / 10))* block_size, (block->Y + 0.5f + 0.2f * sinf(CV_PI * 5 / 10))* block_size),
								Point((block->X + 0.5f + 0.4f * cosf(CV_PI * 7 / 10))* block_size, (block->Y + 0.5f + 0.4f * sinf(CV_PI * 7 / 10))* block_size),
								Point((block->X + 0.5f + 0.2f * cosf(CV_PI * 9 / 10))* block_size, (block->Y + 0.5f + 0.2f * sinf(CV_PI * 9 / 10))* block_size),
								Point((block->X + 0.5f + 0.4f * cosf(CV_PI * 11 / 10))* block_size, (block->Y + 0.5f + 0.4f * sinf(CV_PI * 11 / 10))* block_size),
								Point((block->X + 0.5f + 0.2f * cosf(CV_PI * 13 / 10))* block_size, (block->Y + 0.5f + 0.2f * sinf(CV_PI * 13 / 10))* block_size),
								Point((block->X + 0.5f + 0.4f * cosf(CV_PI * 15 / 10))* block_size, (block->Y + 0.5f + 0.4f * sinf(CV_PI * 15 / 10))* block_size),
								Point((block->X + 0.5f + 0.2f * cosf(CV_PI * 17 / 10))* block_size, (block->Y + 0.5f + 0.2f * sinf(CV_PI * 17 / 10))* block_size),
								Point((block->X + 0.5f + 0.4f * cosf(CV_PI * 19 / 10))* block_size, (block->Y + 0.5f + 0.4f * sinf(CV_PI * 19 / 10))* block_size)
						}, Scalar(0, 0, 0, 255));
						fillConvexPoly(con_img, std::vector<Point>{
							Point((block->X + 0.5f + 0.14f * cosf(CV_PI * 1 / 10))* block_size, (block->Y + 0.5f + 0.14f * sinf(CV_PI * 1 / 10))* block_size),
								Point((block->X + 0.5f + 0.28f * cosf(CV_PI * 3 / 10))* block_size, (block->Y + 0.5f + 0.28f * sinf(CV_PI * 3 / 10))* block_size),
								Point((block->X + 0.5f + 0.14f * cosf(CV_PI * 5 / 10))* block_size, (block->Y + 0.5f + 0.14f * sinf(CV_PI * 5 / 10))* block_size),
								Point((block->X + 0.5f + 0.28f * cosf(CV_PI * 7 / 10))* block_size, (block->Y + 0.5f + 0.28f * sinf(CV_PI * 7 / 10))* block_size),
								Point((block->X + 0.5f + 0.14f * cosf(CV_PI * 9 / 10))* block_size, (block->Y + 0.5f + 0.14f * sinf(CV_PI * 9 / 10))* block_size),
								Point((block->X + 0.5f + 0.28f * cosf(CV_PI * 11 / 10))* block_size, (block->Y + 0.5f + 0.28f * sinf(CV_PI * 11 / 10))* block_size),
								Point((block->X + 0.5f + 0.14f * cosf(CV_PI * 13 / 10))* block_size, (block->Y + 0.5f + 0.14f * sinf(CV_PI * 13 / 10))* block_size),
								Point((block->X + 0.5f + 0.28f * cosf(CV_PI * 15 / 10))* block_size, (block->Y + 0.5f + 0.28f * sinf(CV_PI * 15 / 10))* block_size),
								Point((block->X + 0.5f + 0.14f * cosf(CV_PI * 17 / 10))* block_size, (block->Y + 0.5f + 0.14f * sinf(CV_PI * 17 / 10))* block_size),
								Point((block->X + 0.5f + 0.28f * cosf(CV_PI * 19 / 10))* block_size, (block->Y + 0.5f + 0.28f * sinf(CV_PI * 19 / 10))* block_size)
						}, block->garrison != nullptr ? Scalar(170, 170, 170, 255) : Scalar(0, 255, 255, 255));
					}
				}
			}
			break;
		case 1:
			text = "Food_Level";
			break;
		case 2:
			text = "Build";
			break;
		case 3:
			text = "Man";
			break;
		case 4:
			text = "Food";
			break;
		case 5:
			text = "Product";
			break;
		case 6:
			text = "Priority Rank";
			break;
		case 7:
			text = "Distance From Border";
			break;
		case 8:
			text = "Distance From Capital";
			break;
		case 9:
			text = "Loaf";
			break;
		}
		putText(UI_img, text, Point(882, 332), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 0, 255), 2, 8, false);
		putText(UI_img, text, Point(880, 330), FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255, 255), 2, 8, false);

		char buf[256];
		sprintf_s(buf, "%s %2d/AD %4d", Month_Name[month], day, year);

		putText(UI_img, buf, Point(882, 392), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 0, 255), 2, 8, false);
		putText(UI_img, buf, Point(880, 390), FONT_HERSHEY_COMPLEX, 1, Scalar(255, 255, 255, 255), 2, 8, false);

		Concurrency::parallel_for(0, final_img.cols, [&](int x) {
			for (int y = 0; y < final_img.rows; ++y)
			{
				auto& target = final_img.at<Vec3b>(y, x);

				//Blend ConMap
				{
					auto a = sizeof Vec4f;
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

		return final_img;
	};

	function<void(int, int, int, int)> mouse = [&](int events, int mouse_x, int mouse_y, int flags) {
		switch (events)
		{
		case EVENT_LBUTTONDOWN:
		{
			int x = mouse_x / block_size;
			int y = mouse_y / block_size;

			select_country = board[x][y]->owner;
		}break;
		case EVENT_RBUTTONDOWN:
		{
			int x = mouse_x / block_size;
			int y = mouse_y / block_size;

			if (select_country != nullptr)
			{
				float i = select_country->priority_power[x][y];
				int j = select_country->priority_rank[x][y];
				cout << "Rank[" << j << "]" << i << endl;
			}
		}break;
		}
	};
	void keyboard(int key)
	{
		switch (key)
		{
		case 'q':
			(*GameRun) = false;
			break;
		case '`':
			showmode = 0;
			break;
		case '1':
			showmode = 1;
			break;
		case '2':
			showmode = 2;
			break;
		case '3':
			showmode = 3;
			break;
		case '4':
			showmode = 4;
			break;
		case '5':
			showmode = 5;
			break;
		case '6':
			showmode = 6;
			break;
		case '7':
			showmode = 7;
			break;
		case '8':
			showmode = 8;
			break;
		case '9':
			showmode = 9;
			break;
		}
	}
};

void main() {
	int GameRun = 1;
	Game game = Game(&GameRun);

	namedWindow("mainWindow");
	setMouseCallback("mainWindow", mouse_callback, &mainGame->mouse);

	while (GameRun)
	{
		mainGame->step();
		imshow("mainWindow", mainGame->draw());

		mainGame->keyboard(waitKey(1));
	}
}