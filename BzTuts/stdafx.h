#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <string>
#include <DirectXMath.h>

#include "Renderer.h"


static Renderer* mainApp = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return mainApp->WndCallback(hWnd, msg, wParam, lParam);
}

