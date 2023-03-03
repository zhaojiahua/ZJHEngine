#include "WinMainCmdParameters.h"

ZWinMainCmdParameters::ZWinMainCmdParameters(HINSTANCE thisInstance, HINSTANCE preInstance, LPSTR cmdLine, int showCmd) :
	mThisInstance(thisInstance), mPreInstance(preInstance), mCmdLine(cmdLine), mShowCmd(showCmd)
{
}
