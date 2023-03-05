#include "EnginLunchMain.h"
#include "EngineFactory.h"
#include "Debugs/Logs/SimpleLog.h"

int Init(ZEngine* engine, HINSTANCE thisInstance, HINSTANCE preInstance, LPSTR cmdLine, int showCmd);
void Tick(ZEngine* engine);
int Exit(ZEngine* engine);

int WINAPI WinMain(HINSTANCE thisInstance, HINSTANCE preInstance, LPSTR cmdLine, int showCmd)
{
	ZEngine* engine = ZEngineFactory::CreateEngine();
	//初始化
	int result = Init(engine, thisInstance, preInstance, cmdLine, showCmd);

	//渲染
	while (true) {
		Tick(engine);
	}

	//退出
	result = Exit(engine);

	return result;//如果是0就表示程序成功退出
}

int Init(ZEngine* engine, HINSTANCE thisInstance, HINSTANCE preInstance, LPSTR cmdLine, int showCmd) {
	int notsuccess = engine->PreInit(ZWinMainCmdParameters(thisInstance, preInstance, cmdLine, showCmd));
	if (notsuccess) {
		ZLog_error("[%i] ZEngine preInitialize error", notsuccess);
		return notsuccess;
	}
	notsuccess = engine->Init();
	if (notsuccess) {
		ZLog_error("[%i] ZEngine initialize error", notsuccess);
		return notsuccess;
	}
	notsuccess = engine->PostInit();
	if (notsuccess) {
		ZLog_error("[%i] ZEngine postInitialize error", notsuccess);
		return notsuccess;
	}
	return notsuccess;
}
void Tick(ZEngine* engine) {
	engine->Tick();
}
int Exit(ZEngine* engine) {
	int notsuccess = engine->PreExit();
	if (notsuccess) {
		ZLog_error("[%i] ZEngine preExit error", notsuccess);
		return notsuccess;
	}
	notsuccess = engine->Exit();
	if (notsuccess) {
		ZLog_error("[%i] ZEngine exit error", notsuccess);
		return notsuccess;
	}
	notsuccess = engine->PostExit();
	if (notsuccess) {
		ZLog_error("[%i] ZEngine postExit error", notsuccess);
		return notsuccess;
	}
	ZLog("ZEngine Exit");
	return notsuccess;
}