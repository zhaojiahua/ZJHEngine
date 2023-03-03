#pragma once

#if defined(_WIN32)
#include "../EnginLunchMain.h"
class ZWinMainCmdParameters
{
public:
	ZWinMainCmdParameters(HINSTANCE thisInstance, HINSTANCE preInstance, LPSTR cmdLine, int showCmd);
	HINSTANCE mThisInstance;
	HINSTANCE mPreInstance;
	LPSTR mCmdLine;
	int mShowCmd;
};

#elif 0

#endif // 1
