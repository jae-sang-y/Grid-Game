#pragma once

#include <DirectXMath.h>
using namespace DirectX;


constexpr int virtual_W = 1440;
constexpr int virtual_H = 1080;

struct View {
	const XMFLOAT4 default_color = XMFLOAT4( 1, 1, 1, 1 );
	XMFLOAT4 color = default_color;
	virtual void DrawImage(std::wstring path, XMFLOAT2 pos, XMFLOAT2 size) {};
	virtual void DrawRect(XMFLOAT2 pos, XMFLOAT2 size) {};
	virtual void DrawStr(XMFLOAT2 pos, std::string text) {};
	virtual void ClearDepth() {};
	virtual XMFLOAT2 GetMousePos()  = 0;

	float DT = 0.f;
	float T = 0.f;
};
extern View* V;

void main_start();
void main_step();
void main_draw();
void main_end();
void main_msg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);