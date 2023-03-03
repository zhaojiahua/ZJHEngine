#pragma once
#include "core/Engine.h"

class ZEngineFactory
{
public:
	ZEngineFactory();
	static ZEngine* CreateEngine();
};

