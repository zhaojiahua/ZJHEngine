#include "Rendering.h"


vector<IRenderingInterface*>IRenderingInterface::renderingInterfaces;
IRenderingInterface::IRenderingInterface()
{
	create_guid(&mGuid);
	renderingInterfaces.push_back(this);
}

IRenderingInterface::~IRenderingInterface()
{
	for (vector<IRenderingInterface*>::const_iterator iter = renderingInterfaces.begin(); iter != renderingInterfaces.end(); ++iter) {
		if (*iter == this) {
			renderingInterfaces.erase(iter);
			break;
		}
	}
}

void IRenderingInterface::Init()
{
}

void IRenderingInterface::Draw(float deltaTime)
{
}

bool IRenderingInterface::operator==(const IRenderingInterface& otherrender)
{
	return guid_equal(&mGuid, &otherrender.mGuid);
}
