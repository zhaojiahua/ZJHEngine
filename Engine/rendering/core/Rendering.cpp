#include "Rendering.h"
#include "../../platform/windows/EngineWind.h"


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

ComPtr<ID3D12Device> IRenderingInterface::Getd3dDevice()
{
	if (ZEngineWind* engineWnd= dynamic_cast<ZEngineWind*>(engine)) {
		return engineWnd->d3dDevice;
	}
	return nullptr;
}

ComPtr<ID3D12GraphicsCommandList> IRenderingInterface::GetdcommandList()
{
	if (ZEngineWind* engineWnd = dynamic_cast<ZEngineWind*>(engine)) {
		return engineWnd->commandList;
	}
	return nullptr;
}

ComPtr<ID3D12Resource> IRenderingInterface::ConstructDefaultBuffer(ComPtr<ID3D12Resource>& upTempBuffer, const void* indata, UINT64 indatasize)
{
	ComPtr<ID3D12Resource> tempbuffer;//GPU缓冲区
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(indatasize);//资源描述符
	CD3DX12_HEAP_PROPERTIES tempProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);//默认缓冲区
	ANALYSIS_HRESULT(Getd3dDevice()->CreateCommittedResource(&tempProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS(tempbuffer.GetAddressOf())));
	CD3DX12_HEAP_PROPERTIES updataBufferProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);//上传缓冲区
	ANALYSIS_HRESULT(Getd3dDevice()->CreateCommittedResource(&updataBufferProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(upTempBuffer.GetAddressOf())));
	D3D12_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pData = indata;
	subresourceData.RowPitch = indatasize;
	subresourceData.SlicePitch = subresourceData.RowPitch;
	//标记资源为复制目标
	CD3DX12_RESOURCE_BARRIER copytempBarrier = CD3DX12_RESOURCE_BARRIER::Transition(tempbuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	GetdcommandList()->ResourceBarrier(1, &copytempBarrier);
	UpdateSubresources<1>(GetdcommandList().Get(), tempbuffer.Get(), upTempBuffer.Get(), 0, 0, 1, &subresourceData);//更新资源
	//切换资源状态
	CD3DX12_RESOURCE_BARRIER readtempBarrier = CD3DX12_RESOURCE_BARRIER::Transition(tempbuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ);
	GetdcommandList()->ResourceBarrier(1, &readtempBarrier);

	return tempbuffer;
}
