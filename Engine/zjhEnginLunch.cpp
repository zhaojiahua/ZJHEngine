#include "EnginLunchMain.h"
#include "EngineFactory.h"
#include "Debugs/Logs/SimpleLog.h"

int Init(ZEngine* engine, HINSTANCE thisInstance, HINSTANCE preInstance, LPSTR cmdLine, int showCmd);
void Tick(ZEngine* engine);
int Exit(ZEngine* engine);

ZEngine* engine = nullptr;

int WINAPI WinMain(HINSTANCE thisInstance, HINSTANCE preInstance, LPSTR cmdLine, int showCmd)
{
	int result = 0;
	engine = ZEngineFactory::CreateEngine();
	if (engine) {
		//初始化
		result = Init(engine, thisInstance, preInstance, cmdLine, showCmd);

		//渲染出图
		MSG msg = { 0 };
		while (msg.message != WM_QUIT) {
			if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);//翻译消息
				DispatchMessage(&msg);//派发消息
			}
			else {
				Tick(engine);
			}
		}
	}

	//退出
	result = Exit(engine);

	return result;//如果是0就表示程序成功退出
}

int Init(ZEngine* engine, HINSTANCE thisInstance, HINSTANCE preInstance, LPSTR cmdLine, int showCmd) {

	ZWinMainCmdParameters winmainParas(thisInstance, preInstance, cmdLine, showCmd);

	int notsuccess = engine->PreInit(winmainParas);
	if (notsuccess) {
		ZLog_error("[%i] ZEngine preInitialize error", notsuccess);
		return notsuccess;
	}
	notsuccess = engine->Init(winmainParas);
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

	float deltaTime = 0.1f;
	engine->Tick(deltaTime);
	Sleep(20);
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