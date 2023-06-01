#include "EngineWind.h"
#include "WinMessageProcess.h"
#include "../../Debugs/EngineDebug.h"
#include "../../config/EngineConfig.h"
#include "../../rendering/core/Rendering.h"
#include "../../mesh/BoxMesh.h"

#if defined(_WIN32)
ZEngineWind::ZEngineWind()
	:mMultiSampleLevel(0),
	bMultiSample(false),
	mHwnd(NULL),
	mBackBufferFormat(DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM),
	mDepthStencilFormat(DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT),
	currentFenceIndex(0),currentSwapBufferIndex(0)
{
	for (int i = 0; i < ZEngineRenderConfig::GetRenderConfig()->mSwapChainCount; ++i) {
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
	//初始化缓冲区(前面创建了两个缓冲区,在这里进行初始化)
	for (auto item : mSwapBuffers) {
		item.Reset();
	}
	mDepthStencilBuffer.Reset();//深度模版缓冲区也初始化
	//交换链缓冲区自适应设置(设置缓冲视图的尺寸,两个缓冲区大小一致)
	swapChain->ResizeBuffers(
		ZEngineRenderConfig::GetRenderConfig()->mSwapChainCount,
		ZEngineRenderConfig::GetRenderConfig()->mScreenWidth,
		ZEngineRenderConfig::GetRenderConfig()->mScreenHeight,
		mBackBufferFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	//绑定资源到流水线上
	//渲染目标Buffer
	mRTVDescSize = d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);//获取RTV描述符堆的大小
	CD3DX12_CPU_DESCRIPTOR_HANDLE heapHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart());//获取资源描述堆句柄,用这个句柄访问这个描述堆
	for (UINT i = 0; i < ZEngineRenderConfig::GetRenderConfig()->mSwapChainCount; ++i) {
		swapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapBuffers[i]));	//缓冲区资源绑定到交换链
		d3dDevice->CreateRenderTargetView(mSwapBuffers[i].Get(), nullptr, heapHandle);	//根据资源描述符句柄创建默认格式的RTV资源
		heapHandle.Offset(1, mRTVDescSize);
	}
	
	D3D12_RESOURCE_DESC resourceDesc;
	resourceDesc.Width = ZEngineRenderConfig::GetRenderConfig()->mScreenWidth;
	resourceDesc.Height = ZEngineRenderConfig::GetRenderConfig()->mScreenHeight;
	resourceDesc.Alignment = 0;//默认对齐方式
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;//
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2D贴图资源
	resourceDesc.SampleDesc.Count = bMultiSample ? 4 : 1;
	resourceDesc.SampleDesc.Count = bMultiSample ? (mMultiSampleLevel - 1) : 0;
	resourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//允许使用深度模版
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//默认资源布局
	D3D12_CLEAR_VALUE clearValue;
	clearValue.DepthStencil.Depth = 1.0;
	clearValue.DepthStencil.Stencil = 0;
	clearValue.Format = mDepthStencilFormat;
	CD3DX12_HEAP_PROPERTIES tempProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);		//创建默认堆,通过这个堆,CPU上传数据到GPU
	d3dDevice->CreateCommittedResource(&tempProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf()));
	
	//深度模版Buffer
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = mDepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	d3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), &dsvDesc, DSVHeap->GetCPUDescriptorHandleForHeapStart());	//根据深度缓冲描述创建深度缓冲视图

	//创建资源的"围栏",负责资源同步,资源状态的转换
	CD3DX12_RESOURCE_BARRIER tempBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mDepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	commandList->ResourceBarrier(1, &tempBarrier);
	commandList->Close();
	//提交命令
	SubmitCommandList();

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

	//构建一个BoxMesh
	BoxMesh* boxmesh = BoxMesh::CreateMesh();

	return 0;
}

void ZEngineWind::Tick(float deltaTime)
{
	//重置录制相关内存
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
	commandList->ClearRenderTargetView(cuswapBufferView, DirectX::Colors::Bisque, 1, &viewportRect);
	//清除深度模版缓存
	//commandList->ClearDepthStencilView(cudepthStencilBufferView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.f, 0, 0, &viewportRect);
	//输出合并阶段
	commandList->OMSetRenderTargets(1, &cuswapBufferView, true, &cudepthStencilBufferView);

	//渲染场景内容到RTV
	for (auto item : IRenderingInterface::renderingInterfaces) {
		item->Draw(deltaTime);
	}

	//设置渲染状态
	CD3DX12_RESOURCE_BARRIER tempBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(GetCurrentSwapBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &tempBarrier2);
	//关闭命令列表
	ANALYSIS_HRESULT(commandList->Close());
	//提交命令
	SubmitCommandList();
	//呈现并交换缓冲区
	ANALYSIS_HRESULT(swapChain->Present(0, 0));	//呈现缓冲区
	currentSwapBufferIndex = !(bool)currentSwapBufferIndex;	//交换一下缓冲区索引
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
		d3dDebug->EnableDebugLayer();//开启Debug调试层,可以吧一些错误信息打印到VS输出窗口
	}

	ANALYSIS_HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));	//创建引擎工厂

	HRESULT d3dDeviceResult = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice));//创建显卡驱动
	if (FAILED(d3dDeviceResult)) {
		//如果硬件驱动不成功就创建软件驱动
		Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
		ANALYSIS_HRESULT(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		ANALYSIS_HRESULT(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice)));
	}
	ANALYSIS_HRESULT(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3dFence)));//创建Fence

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////��ʼ���������
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;//GPU直接执行
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;
	//命令队列
	ANALYSIS_HRESULT(d3dDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&commandQueue)));
	//命令分配器
	ANALYSIS_HRESULT(d3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(commandAllocator.GetAddressOf())));
	//命令列表
	HRESULT createResult = d3dDevice->CreateCommandList(
		0,//默认单个GPU
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,//直接执行命令
		commandAllocator.Get(),//将CommandList关联到Allocator
		NULL	//管线状态,NULL的话它会自己设置一个虚拟管线状态
		, IID_PPV_ARGS(commandList.GetAddressOf()));
	if (FAILED(createResult)) {
		ZLog_error(" commandList error = % i", (int)createResult);
	}
	commandList->Close();//关闭列表

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////判断多重采样的支持情况
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
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//使用表面或资源作为输出渲染目标
	swapChainDesc.OutputWindow = mHwnd;//绑定主窗口
	swapChainDesc.Windowed = true;//非全屏模式(窗口模型运行)
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_FLIP_DISCARD;//交换完后直接丢弃
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.BufferDesc.Format = mBackBufferFormat;
	//采样设置
	swapChainDesc.SampleDesc.Count = bMultiSample ? 4 : 1;
	swapChainDesc.SampleDesc.Quality = bMultiSample ? (mMultiSampleLevel - 1) : 0;
	createResult = dxgiFactory->CreateSwapChain(commandQueue.Get(), &swapChainDesc, swapChain.GetAddressOf());
	if (FAILED(createResult)) {
		ZLog_error(" CreateSwapChain error = % i", (int)createResult);
	}

	//资源描述符堆的创建
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;//渲染目标描述符
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;//深度缓冲描述符
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;//渲染目标视图
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
	ANALYSIS_HRESULT(commandQueue->Signal(d3dFence.Get(), currentFenceIndex));//等待GPU处理完信号
	if (d3dFence->GetCompletedValue() < currentFenceIndex) {
		HANDLE eventEX = CreateEventEx(NULL, NULL, 0, EVENT_ALL_ACCESS);		//创建一个事件内核对象并返回该内核对象句柄(锁)
		ANALYSIS_HRESULT(d3dFence->SetEventOnCompletion(currentFenceIndex, eventEX));	//GPU完成后会通知eventEX HANDLE

		WaitForSingleObject(eventEX, INFINITE);//阻塞CPU,无限等待eventEX
		CloseHandle(eventEX);	//等到了eventEX后关闭事件(锁),开始执行后续任务
	}
}

void ZEngineWind::SubmitCommandList()
{
	ID3D12CommandList* cmdList[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);
}

#endif