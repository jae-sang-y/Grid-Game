#include "stdafx.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	mainApp = new Renderer();

	if (!mainApp->InitializeWindow(hInstance, nShowCmd, FALSE))
	{
		MessageBox(0, L"Window Initialization - Failed",
			L"Error", MB_OK);
		return 1;
	}

	if (!mainApp->InitD3D())
	{
		MessageBox(0, L"Failed to initialize direct3d 12",
			L"Error", MB_OK);
		mainApp->Cleanup();
		return 1;
	}

	mainApp->mainloop();

	mainApp->WaitForPreviousFrame();

	CloseHandle(mainApp->fenceEvent);

	mainApp->Cleanup();

	return 0;
}
