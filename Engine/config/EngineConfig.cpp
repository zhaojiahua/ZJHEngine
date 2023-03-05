#include "EngineConfig.h"

ZEngineRenderConfig* mRenderConfig = nullptr;

ZEngineRenderConfig::ZEngineRenderConfig()
	:mScreenWidth(1080),mScreenHeight(720),mFreshRate(60),mSwapChainCount(2)
{}

ZEngineRenderConfig* ZEngineRenderConfig::GetRenderConfig()
{
	if (!mRenderConfig) {
		mRenderConfig = new ZEngineRenderConfig();
	}
	return mRenderConfig;
}

void ZEngineRenderConfig::Destroy()
{
	if (mRenderConfig) {
		delete(mRenderConfig);
		mRenderConfig = nullptr;
	}
}
