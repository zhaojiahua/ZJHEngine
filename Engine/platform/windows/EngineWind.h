#pragma once

#if defined(_WIN32)
#include "../../core/Engine.h"
class ZEngineWind : public ZEngine
{
public:
	ZEngineWind();

	virtual int PreInit(ZWinMainCmdParameters inparas);
	virtual int Init(ZWinMainCmdParameters inparas);
	virtual int PostInit();

	virtual void Tick();

	virtual int PreExit();
	virtual int Exit();
	virtual int PostExit();

private:
	//��ʼ��windows
	bool InitWindow(ZWinMainCmdParameters inparas);
	//��ʼ��D3D
	bool InitDirect3D();

protected:
	void WaitGPUCommandQueueComplete();

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
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;	//����DXͼ�λ����ṹ
	Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;	//��������������
	Microsoft::WRL::ComPtr<ID3D12Fence> d3dFence;	//"Χ��",ƽ��CPU��GPU֮��ļ���

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;//GPU�������
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;//���������
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;//CPU�����б�

	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;//交换链

	//资源描述符和堆
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> RTVHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DSVHeap;

	//资源
	vector<Microsoft::WRL::ComPtr<ID3D12Resource>> mSwapBuffers;
	Microsoft::WRL::ComPtr<ID3D12Resource> mDepthStencilBuffer;

};
#endif