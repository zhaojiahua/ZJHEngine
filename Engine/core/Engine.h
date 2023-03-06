#pragma once

#if defined(_WIN32)
#include "WinMainCmdParameters.h"
#endif

class ZEngine
{
public:
	//引擎预初始化
	virtual int PreInit(
#if defined(_WIN32)
		ZWinMainCmdParameters
#endif
	) = 0;
	//初始化
	virtual int Init(
#if defined(_WIN32)
		ZWinMainCmdParameters inparas
#endif
	) = 0;
	//初始化完成后的收尾工作
	virtual int PostInit() = 0;

	//渲染循环主体函数
	virtual void Tick() = 0;

	//引擎预退出
	virtual int PreExit() = 0;
	//退出
	virtual int Exit() = 0;
	//退出后的收尾
	virtual int PostExit() = 0;
};