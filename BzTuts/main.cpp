#include "stdafx.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	mainApp = new Renderer();

	try
	{
		mainApp->InitializeWindow(hInstance, nShowCmd, FALSE);

		mainApp->InitD3D();

		mainApp->mainloop();

		mainApp->WaitForPreviousFrame();

		CloseHandle(mainApp->m_fenceEvent);
	}
	catch (DxException dx)
	{
		_com_error err(dx.ErrorCode);
		MessageBoxW(
			mainApp->hwnd,
			(dx.Filename + L"(line : " + std::to_wstring(dx.LineNumber) + L")\n" + dx.FunctionName + L"\n" + err.ErrorMessage()).c_str(),
			L"Exception",
			MB_OK | MB_ICONERROR
		);
		return -1;
	}

	return 0;
}