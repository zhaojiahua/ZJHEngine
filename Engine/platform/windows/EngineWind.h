#pragma once

#if defined(_WIN32)
#include "../../core/Engine.h"
class ZEngineWind : public ZEngine
{
public:
	ZEngineWind();

	virtual int PreInit(ZWinMainCmdParameters inparas);
	virtual int Init();
	virtual int PostInit();

	virtual void Tick();

	virtual int PreExit();
	virtual int Exit();
	virtual int PostExit();

private:
	//初始化windows
	bool InitWindow(ZWinMainCmdParameters inparas);
	//初始化D3D
	bool InitDirect3D();

protected:
	HWND mHwnd;//主窗口句柄
	UINT mMultiSampleLevel;//多重采样层数
	bool bMultiSample;

protected:
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;	//创建DX图形基础结构
	Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;	//创建命令适配器
	Microsoft::WRL::ComPtr<ID3D12Fence> d3dFence;	//"围栏",平衡CPU和GPU之间的计算

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;//GPU命令队列
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;//命令调度器
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;//CPU命令列表

	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;//交换链

};
#endif