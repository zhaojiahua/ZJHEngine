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
	//��ʼ��windows
	bool InitWindow(ZWinMainCmdParameters inparas);
	//��ʼ��D3D
	bool InitDirect3D();

protected:
	HWND mHwnd;//�����ھ��
	UINT mMultiSampleLevel;//���ز�������
	bool bMultiSample;

protected:
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;	//����DXͼ�λ����ṹ
	Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;	//��������������
	Microsoft::WRL::ComPtr<ID3D12Fence> d3dFence;	//"Χ��",ƽ��CPU��GPU֮��ļ���

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;//GPU�������
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;//���������
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;//CPU�����б�

	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;//������

};
#endif