#pragma once
#include "../../core/Engine.h"

using namespace Microsoft::WRL;

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

	//获取设备的Guid
	simple_c_guid GetGuid() { return mGuid; }
protected:
	//获取引擎的驱动设备
	ComPtr<ID3D12Device> Getd3dDevice();
	//获取引擎的GPU命令列表
	ComPtr<ID3D12GraphicsCommandList> GetdcommandList();
	//从模型数据创建构建GPU渲染资源
	ComPtr<ID3D12Resource> ConstructDefaultBuffer(ComPtr<ID3D12Resource>& upTempBuffer, const void* indata, UINT64 indatasize);
private:
	static vector<IRenderingInterface*> renderingInterfaces;	//存储所有的renderingInterface
	simple_c_guid mGuid;//每一个renderingInterface都带有一个Guid

};

