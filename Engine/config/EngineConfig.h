#pragma once

struct ZEngineRenderConfig
{
	ZEngineRenderConfig();
	int mScreenWidth;
	int mScreenHeight;
	int mFreshRate;
	int mSwapChainCount;

	static ZEngineRenderConfig* GetRenderConfig();
	static void Destroy();

private:
	static ZEngineRenderConfig* mRenderConfig;
};
