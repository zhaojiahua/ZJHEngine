#include "EngineWind.h"

#if defined(_WIN32)
int ZEngineWind::PreInit(ZWinMainCmdParameters inparas)
{
	if (InitWindow(inparas)) {

	}
	return 0;
}

int ZEngineWind::Init()
{
	return 0;
}

int ZEngineWind::PostInit()
{
	return 0;
}

void ZEngineWind::Tick()
{
}

int ZEngineWind::PreExit()
{
	return 0;
}

int ZEngineWind::Exit()
{
	return 0;
}

int ZEngineWind::PostExit()
{
	return 0;
}
bool ZEngineWind::InitWindow(ZWinMainCmdParameters inparas)
{
	WNDCLASSEX windClass;

	return false;
}
#endif