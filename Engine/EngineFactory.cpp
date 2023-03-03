#include "EngineFactory.h"

#if defined(_WIN32)
#include "platform/windows/EngineWind.h"
#endif

ZEngineFactory::ZEngineFactory()
{
}

ZEngine* ZEngineFactory::CreateEngine()
{
#if defined(_WIN32)
	return new ZEngineWind();
#elif defined(__linux__)

#endif
	
}
