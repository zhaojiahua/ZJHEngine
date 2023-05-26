#include "WinMessageProcess.h"

LRESULT EngineWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	//其它的交给Windows自己处理
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
