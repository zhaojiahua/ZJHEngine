#include "EngineWind.h"
#include "WinMessageProcess.h"
#include "../../Debugs/EngineDebug.h"
#include "../../config/EngineConfig.h"

#if defined(_WIN32)
ZEngineWind::ZEngineWind()
	:mMultiSampleLevel(0),
	bMultiSample(false),
	mHwnd(NULL),
	mBackBufferFormat(DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM),
	mDepthStencilFormat(DXGI_FORMAT_D24_UNORM_S8_UINT),
	currentFenceIndex(0),currentSwapBufferIndex(0)
{
	for (int i = 0; i < ZEngineRenderConfig::GetRenderConfig()->mSwapChainCount; i++) {
		mSwapBuffers.push_back(Microsoft::WRL::ComPtr<ID3D12Resource>());
	}
}
int ZEngineWind::PreInit(ZWinMainCmdParameters inparas)
{
	//日志系统初始化
	const char LogPath[] = "log";
	init_log_system(LogPath);
	ZLog_sucess("Log init.");
	//处理命令


	ZLog_sucess("Engine preInitialization complete.");
	return 0;
}

int ZEngineWind::Init(ZWinMainCmdParameters inparas)
{
	//初始化Windows
	if (InitWindow(inparas)) {

	}
	//初始化Direction3D
	if (InitDirect3D()) {

	}
	ZLog_sucess("Engine Initialization complete.");
	return 0;
}

int ZEngineWind::PostInit()
{
	WaitGPUCommandQueueComplete();
	//初始化缓冲区
	for (auto item : mSwapBuffers) {
		item.Reset();
	}
	mDepthStencilBuffer.Reset();//初始化深度缓冲区
	//交换链缓冲区自适应设置
	swapChain->ResizeBuffers(
		ZEngineRenderConfig::GetRenderConfig()->mSwapChainCount,
		ZEngineRenderConfig::GetRenderConfig()->mScreenWidth,
		ZEngineRenderConfig::GetRenderConfig()->mScreenHeight,
		mBackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	//绑定资源
	//渲染目标Buffer
	mRTVDescSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);//获取RTV渲染缓冲的大小
	CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart());//获取堆资源位置
	for (UINT i = 0; i < ZEngineRenderConfig::GetRenderConfig()->mSwapChainCount; i++) {
		swapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapBuffers[i]));
		d3dDevice->CreateRenderTargetView(mSwapBuffers[i].Get(), nullptr, heapHandle);
		heapHandle.Offset(1, mRTVDescSize);
	}
	
	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Width = ZEngineRenderConfig::GetRenderConfig()->mScreenWidth;
	resourceDesc.Height = ZEngineRenderConfig::GetRenderConfig()->mScreenHeight;
	resourceDesc.Alignment = 0;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.SampleDesc.Count = bMultiSample ? 4 : 1;
	resourceDesc.SampleDesc.Count = bMultiSample ? (mMultiSampleLevel - 1) : 0;
	resourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	D3D12_CLEAR_VALUE clearValue;
	clearValue.DepthStencil.Depth = 1.0;
	clearValue.DepthStencil.Stencil = 0;
	clearValue.Format = mDepthStencilFormat;
	CD3DX12_HEAP_PROPERTIES tempProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	d3dDevice->CreateCommittedResource(&tempProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf()));
	//深度模版Buffer
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = mDepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	d3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DSVHeap->GetCPUDescriptorHandleForHeapStart());

	CD3DX12_RESOURCE_BARRIER tempBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	commandList->ResourceBarrier(1, &tempBarrier);
	commandList->Close();
	//提交命令
	ID3D12CommandList* cmdList[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

	//视图窗口设置
	viewportInfo.TopLeftX = 0; viewportInfo.TopLeftY = 0;
	viewportInfo.Width = ZEngineRenderConfig::GetRenderConfig()->mScreenWidth;
	viewportInfo.Height = ZEngineRenderConfig::GetRenderConfig()->mScreenHeight;
	viewportInfo.MinDepth = 0.0f;
	viewportInfo.MaxDepth = 1.0f;

	viewportRect.left = 0; viewportRect.top = 0;
	viewportRect.right = ZEngineRenderConfig::GetRenderConfig()->mScreenWidth;
	viewportRect.bottom = ZEngineRenderConfig::GetRenderConfig()->mScreenHeight;

	WaitGPUCommandQueueComplete();

	ZLog_sucess("Engine postInitialization complete.");
	return 0;
}

void ZEngineWind::Tick(float deltaTime)
{
	//重置相关内存
	ANALYSIS_HRESULT(commandAllocator->Reset());
	//重置命令列表
	commandList->Reset(commandAllocator.Get(), NULL);
	//转换资源指向状态
	CD3DX12_RESOURCE_BARRIER tempBarrier = CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentSwapBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &tempBarrier);

	commandList->RSSetViewports(1, &viewportInfo);
	commandList->RSSetScissorRects(1, &viewportRect);
	//清除画布颜色
	D3D12_CPU_DESCRIPTOR_HANDLE cuswapBufferView = GetCurrentSwapBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE cudepthStencilBufferView = GetCurrentDepthStencilView();
	commandList->ClearRenderTargetView(cuswapBufferView, DirectX::Colors::Beige, 1, &viewportRect);
	//清除深度模版缓存
	//commandList->ClearDepthStencilView(cudepthStencilBufferView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 1, &viewportRect);
	//输出合并阶段
	commandList->OMSetRenderTargets(1, &cuswapBufferView, true, &cudepthStencilBufferView);
	//设置渲染状态
	CD3DX12_RESOURCE_BARRIER tempBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentSwapBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &tempBarrier2);
	//关闭命令队列
	ANALYSIS_HRESULT(commandList->Close());
	//提交命令
	ID3D12CommandList* cmdList[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
	//交换缓冲区
	ANALYSIS_HRESULT(swapChain->Present(0, 0));
	currentSwapBufferIndex = !(bool)currentSwapBufferIndex;
	//等待GPU处理
	WaitGPUCommandQueueComplete();
}

int ZEngineWind::PreExit()
{
	ZLog_sucess("Engine preExit complete.");
	return 0;
}

int ZEngineWind::Exit()
{
	ZLog_sucess("Engine Exit complete.");
	return 0;
}

int ZEngineWind::PostExit()
{
	ZEngineRenderConfig::Destroy();
	ZLog_sucess("Engine postExit complete.");
	return 0;
}
ID3D12Resource* ZEngineWind::GetCurrentSwapBuffer() const
{
	return mSwapBuffers[currentSwapBufferIndex].Get();
}
D3D12_CPU_DESCRIPTOR_HANDLE ZEngineWind::GetCurrentSwapBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(RTVHeap->GetCPUDescriptorHandleForHeapStart(), currentSwapBufferIndex, mRTVDescSize);
}
D3D12_CPU_DESCRIPTOR_HANDLE ZEngineWind::GetCurrentDepthStencilView() const
{
	return DSVHeap->GetCPUDescriptorHandleForHeapStart();
}
bool ZEngineWind::InitWindow(ZWinMainCmdParameters inparas)
{
	//窗口类定义
	WNDCLASSEX windClass;
	windClass.cbSize = sizeof(WNDCLASSEX);//窗口类的实例占用多大内存
	windClass.cbClsExtra = 0;//类是否需要额外空间
	windClass.cbWndExtra = 0;//实例是否需要额外内存
	windClass.hbrBackground = nullptr;//窗口背景颜色,如果没有设置就是GDI擦除
	windClass.hCursor = LoadCursor(NULL, IDC_ARROW);//鼠标箭头风格
	windClass.hIcon = nullptr;//应用程序在磁盘显示的图标
	windClass.hIconSm = NULL;//窗口左上角的小图标
	windClass.hInstance = inparas.mPreInstance;//窗口实例
	windClass.lpszClassName = L"ZJHEngine";//窗口名字
	windClass.lpszMenuName = nullptr;
	windClass.style = CS_VREDRAW | CS_HREDRAW;//绘制窗口风格,垂直和水平绘制
	windClass.lpfnWndProc = (WNDPROC)EngineWndProc;//消息处理回调函数

	//注册窗口
	ATOM registerAtom = RegisterClassEx(&windClass);
	if (!registerAtom) {
		ZLog_error("Register window failed.");
		MessageBox(NULL, L"Register failed", L"Error", MB_OK);
	}
	//窗口分辨率
	RECT rect = { 0,0,ZEngineRenderConfig::GetRenderConfig()->mScreenWidth,ZEngineRenderConfig::GetRenderConfig()->mScreenHeight };//���ڷֱ���
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, NULL);

	int wndwidth = rect.right - rect.left;
	int wndheight = rect.bottom - rect.top;

	//创建窗口实例
	mHwnd = CreateWindowEx(
		NULL,//窗口额外的风格
		L"ZJHEngine",//窗口名称
		L"ZJHEngineWnd",//窗口标题栏
		WS_OVERLAPPEDWINDOW,//窗口风格
		100,100,//窗口左上角坐标
		wndwidth, wndheight,//窗口宽高
		NULL,//副窗口句柄
		nullptr,//菜单句柄
		inparas.mPreInstance,//窗口实例
		NULL
	);
	if (!mHwnd) {
		ZLog_error("Create window failed.");
		MessageBox(0, L"Create window failed", 0, 0);
		return false;
	}

	ShowWindow(mHwnd, SW_SHOW);//显示窗口
	UpdateWindow(mHwnd);//窗口刷新
	ZLog_sucess("Windows Initialization complete.");
	return true;
}

bool ZEngineWind::InitDirect3D()
{
	//Debug
	Microsoft::WRL::ComPtr<ID3D12Debug> d3dDebug;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3dDebug)))) {
		d3dDebug->EnableDebugLayer();
	}

	ANALYSIS_HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));	//创建引擎工厂

	HRESULT d3dDeviceResult = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice));//创建引擎驱动
	if (FAILED(d3dDeviceResult)) {
		//如果硬件驱动不成功就创建软件驱动
		Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
		ANALYSIS_HRESULT(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		ANALYSIS_HRESULT(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice)));
	}
	ANALYSIS_HRESULT(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3dFence)));//����Fence

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////��ʼ���������
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;//ֱ�ӱ�GPU����
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;//�������ʱ
	//�����������
	ANALYSIS_HRESULT(d3dDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&commandQueue)));
	//����������
	ANALYSIS_HRESULT(d3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(commandAllocator.GetAddressOf())));
	//���������б�
	HRESULT createResult = d3dDevice->CreateCommandList(
		0,//Ĭ�ϵ���GPU
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,//ֱ�ӵ���
		commandAllocator.Get(),//�������������
		NULL	//����״̬
		, IID_PPV_ARGS(commandList.GetAddressOf()));
	if (FAILED(createResult)) {
		ZLog_error(" commandList error = % i", (int)createResult);
	}
	commandList->Close();//�رյ�ǰ�б�

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////����������
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevel;
	qualityLevel.SampleCount = 4;
	qualityLevel.Flags = D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS::D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	qualityLevel.NumQualityLevels = 0;
	ANALYSIS_HRESULT(d3dDevice->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&qualityLevel,
		sizeof(qualityLevel)));
	mMultiSampleLevel = qualityLevel.NumQualityLevels;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////交换链
	swapChain.Reset();
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = ZEngineRenderConfig::GetRenderConfig()->mScreenWidth;
	swapChainDesc.BufferDesc.Height = ZEngineRenderConfig::GetRenderConfig()->mScreenHeight;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = ZEngineRenderConfig::GetRenderConfig()->mFreshRate;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferCount = ZEngineRenderConfig::GetRenderConfig()->mSwapChainCount;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//ʹ�ñ������Դ��Ϊ�����ȾĿ��
	swapChainDesc.OutputWindow = mHwnd;//ָ��������
	swapChainDesc.Windowed = true;//�Դ��ڷ�ʽ����
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;//�����������
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.BufferDesc.Format = mBackBufferFormat;
	//采样设置
	swapChainDesc.SampleDesc.Count = bMultiSample ? 4 : 1;
	swapChainDesc.SampleDesc.Quality = bMultiSample ? (mMultiSampleLevel - 1) : 0;
	createResult = dxgiFactory->CreateSwapChain(commandQueue.Get(), &swapChainDesc, swapChain.GetAddressOf());
	if (FAILED(createResult)) {
		ZLog_error(" CreateSwapChain error = % i", (int)createResult);
	}

	//堆资源描述符
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;//渲染目标描述符
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;//深度缓冲描述符
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//渲染到目标视图
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	rtvHeapDesc.NumDescriptors = ZEngineRenderConfig::GetRenderConfig()->mSwapChainCount;//前后两个缓冲区
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;//渲染到深度视图
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	dsvHeapDesc.NumDescriptors = 1;//一个深度缓冲区就够用了
	ANALYSIS_HRESULT(d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(RTVHeap.GetAddressOf())));
	ANALYSIS_HRESULT(d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(DSVHeap.GetAddressOf())));

	return false;
}
void ZEngineWind::WaitGPUCommandQueueComplete()
{
	currentFenceIndex++;
	ANALYSIS_HRESULT(commandQueue->Signal(d3dFence.Get(), currentFenceIndex));
	if (d3dFence->GetCompletedValue() < currentFenceIndex) {
		HANDLE eventEX = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);

		ANALYSIS_HRESULT(d3dFence->SetEventOnCompletion(currentFenceIndex, eventEX));

		WaitForSingleObject(eventEX, INFINITE);
		CloseHandle(eventEX);
	}
}

#endif