#pragma once

#include <DirectXMath.h>
using namespace DirectX;


constexpr int virtual_W = 20 * 80;
constexpr int virtual_H = 20 * 60;

class View {
public:
	const XMFLOAT4 default_color = XMFLOAT4(1, 1, 1, 1);
	XMFLOAT4 color = default_color;
	virtual void DrawImage(const std::wstring path, const XMFLOAT2 pos, const XMFLOAT2 size) {}
	virtual void DrawRect(const XMFLOAT2 pos, const XMFLOAT2 size) {}
	virtual void DrawStr(const XMFLOAT2 pos, const std::string text) {}
	virtual void ClearDepth() {}
	virtual XMFLOAT2 GetMousePos()  = 0;

	double DT = 0.0;
	double T = 0.0;
};
extern View* V;

void main_start();
void main_step();
void main_draw();
void main_end();
void main_msg(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);