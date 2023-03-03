#pragma once

#if defined(_WIN32)
#include "../../core/Engine.h"
class ZEngineWind : public ZEngine
{
public:
	virtual int PreInit(ZWinMainCmdParameters inparas);
	virtual int Init();
	virtual int PostInit();

	virtual void Tick();

	virtual int PreExit();
	virtual int Exit();
	virtual int PostExit();

private:
	bool InitWindow(ZWinMainCmdParameters inparas);
};
#endif