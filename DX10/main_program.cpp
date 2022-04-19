#pragma warning(push, 0)
#include "DXUT.h"
#include "View.h"
#include <fstream>
#include <sstream>
#include <random>
#include <time.h>
#include <unordered_set>

#pragma warning(pop)


MainProgram* main_pgm = MainProgram::get_instance();

void main_start() {
	SetGeoInfo();
	main_pgm->MappingBlocks();
	main_pgm->LoadMapImage();
	main_pgm->SpreadNation();
};
void main_step() {
	main_pgm->StageUpdateInfo();
	main_pgm->StagePropagandaInfo();
	main_pgm->StageStep();
	main_pgm->StagePostStep();
};
void main_draw() {
	main_pgm->StageDraw();
};
void main_end() {
};
void main_msg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	hWnd;
	lParam;

	if (uMsg == WM_KEYDOWN) {
		main_pgm->keyboard((int)wParam);
	}
	else if (uMsg == WM_LBUTTONDOWN) {
		main_pgm->mousedown();
	}
}