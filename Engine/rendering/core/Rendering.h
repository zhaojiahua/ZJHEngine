#pragma once
#include "../../core/Engine.h"

//渲染接口,所有需要渲染的物体都继承自此
class IRenderingInterface
{
	friend class ZEngineWind;
public:
	IRenderingInterface();
	virtual ~IRenderingInterface();

	virtual void Init();
	virtual void Draw(float deltaTime);

	bool operator==(const IRenderingInterface& otherrender);

private:
	static vector<IRenderingInterface*> renderingInterfaces;	//存储所有的renderingInterface
	simple_c_guid mGuid;//每一个renderingInterface都带有一个Guid
};

