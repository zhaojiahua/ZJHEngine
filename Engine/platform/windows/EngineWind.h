#pragma once

#if defined(_WIN32)
#include "../../core/Engine.h"
class ZEngineWind : public ZEngine
{
	friend class IRenderingInterface;
public:
	ZEngineWind();

	virtual int PreInit(ZWinMainCmdParameters inparas);
	virtual int Init(ZWinMainCmdParameters inparas);
	virtual int PostInit();

	virtual void Tick(float deltaTime);

	virtual int PreExit();
	virtual int Exit();
	virtual int PostExit();

	//获取当前指向的资源内存
	ID3D12Resource* GetCurrentSwapBuffer() const;
	//获取当前的堆资源描述
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentSwapBufferView() const;
	//获取深度模版缓冲堆描述
	D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentDepthStencilView() const;

private:
	//初始化windows窗口
	bool InitWindow(ZWinMainCmdParameters inparas);
	//初始化D3D
	bool InitDirect3D();

protected:
	//等待GPU处理完命令
	void WaitGPUCommandQueueComplete();
	//把命令列表提交到命令队列
	void SubmitCommandList();

protected:
	HWND mHwnd;//�����ھ��
	UINT mMultiSampleLevel;//���ز�������
	bool bMultiSample;
	DXGI_FORMAT mBackBufferFormat;	//后台缓冲的图片格式
	DXGI_FORMAT mDepthStencilFormat;	//深度模版缓冲的格式
	UINT mRTVDescSize;

	UINT64 currentFenceIndex;
	int currentSwapBufferIndex;

	//视图尺寸
	D3D12_VIEWPORT viewportInfo;
	D3D12_RECT viewportRect;

protected:
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;	//DXGI,负责图形基础结构的创建
	Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;	//设备(显卡)驱动
	Microsoft::WRL::ComPtr<ID3D12Fence> d3dFence;	//"围栏",平衡CPU和GPU之间的同步工作

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;//GPU命令队列
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;//命令分配器,用来管理命令列表中命令的结构
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;//CPU命令列表

	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;//交换链

	//资源描述符和堆(每次DrawCall之前都需要将本次绘制的相关资源绑定到渲染流水线上,描述符就是送往GPU向GPU描述资源的轻量结构)
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RTVHeap;//渲染目标视图描述符堆,2个,对应交换链的前后缓冲区
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DSVHeap;//深度模版视图描述符堆1个就够了

	//资源
	vector<Microsoft::WRL::ComPtr<ID3D12Resource>> mSwapBuffers;	//渲染目标视图资源,最终的渲染结果都存储在这里
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;	//深度缓冲视图资源,存放深度值

};
#endif