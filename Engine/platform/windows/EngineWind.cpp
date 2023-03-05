#include "EngineWind.h"
#include "WinMessageProcess.h"
#include "../../Debugs/EngineDebug.h"
#include "../../config/EngineConfig.h"

#if defined(_WIN32)
int ZEngineWind::PreInit(ZWinMainCmdParameters inparas)
{
	//日志系统处理
	const char LogPath[] = "log";
	init_log_system(LogPath);
	ZLog_sucess("Log init.");
	//消息处理


	if (InitWindow(inparas)) {

	}
	ZLog_sucess("Engine preInitialization complete.");
	return 0;
}

int ZEngineWind::Init()
{
	ZLog_sucess("Engine Initialization complete.");
	return 0;
}

int ZEngineWind::PostInit()
{
	ZLog_sucess("Engine postInitialization complete.");
	return 0;
}

void ZEngineWind::Tick()
{
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
bool ZEngineWind::InitWindow(ZWinMainCmdParameters inparas)
{
	WNDCLASSEX windClass;
	windClass.cbSize = sizeof(WNDCLASSEX);//改对象占用内存大小
	windClass.cbClsExtra = 0;//是否需要额外空间
	windClass.cbWndExtra = 0;//是否需要额外内存 
	windClass.hbrBackground = nullptr;//默认GDI擦除背景
	windClass.hCursor = LoadCursor(NULL, IDC_ARROW);//设置一个箭头光标
	windClass.hIcon = nullptr;//应用程序放在磁盘上显示的图标
	windClass.hIconSm = NULL;//应用程序左上角显示的图标
	windClass.hInstance = inparas.mPreInstance;//窗口实例
	windClass.lpszClassName = L"ZJHEngine";//窗口名字
	windClass.lpszMenuName = nullptr;
	windClass.style = CS_VREDRAW | CS_HREDRAW;//垂直和水平重绘制
	windClass.lpfnWndProc = (WNDPROC)EngineWndProc;//处理消息的回调函数

	//注册窗口
	ATOM registerAtom = RegisterClassEx(&windClass);
	if (!registerAtom) {
		ZLog_error("Register window failed.");
		MessageBox(NULL, L"Register failed", L"Error", MB_OK);
	}
	//设置窗口
	RECT rect = { 0,0,ZEngineRenderConfig::GetRenderConfig()->mScreenWidth,ZEngineRenderConfig::GetRenderConfig()->mScreenHeight };//窗口分辨率
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, NULL);

	int wndwidth = rect.right - rect.left;
	int wndheight = rect.bottom - rect.top;

	//创建窗口实例
	mHwnd = CreateWindowEx(
		NULL,//窗口额外风格
		L"ZJHEngine",//窗口名
		L"ZJHEngineWnd",//显示在窗口标题栏的名字
		WS_OVERLAPPEDWINDOW,//窗口风格
		100,100,//窗口坐标
		wndwidth, wndheight,//窗口宽高
		NULL,//父窗口句柄
		nullptr,//菜单句柄
		inparas.mPreInstance,//窗口实例
		NULL
	);
	if (!mHwnd) {
		ZLog_error("Create window failed.");
		MessageBox(0, L"Create window failed", 0, 0);
		return false;
	}

	ShowWindow(mHwnd, SW_SHOW);
	UpdateWindow(mHwnd);
	ZLog_sucess("Windows Initialization complete.");
	return true;
}
bool ZEngineWind::InitDirect3D()
{

	ANALYSIS_HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));	//创建工厂

	HRESULT d3dDeviceResult = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice));//创建驱动
	if (FAILED(d3dDeviceResult)) {
		//如果创建硬件适配器失败就创建软件适配器
		Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
		ANALYSIS_HRESULT(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		ANALYSIS_HRESULT(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice)));
	}
	ANALYSIS_HRESULT(d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&d3dFence)));//创建Fence

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////初始化命令对象
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;//直接被GPU调用
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE;//允许命令超时
	//创建命令队列
	ANALYSIS_HRESULT(d3dDevice->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&commandQueue)));
	//创建分配器
	ANALYSIS_HRESULT(d3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(commandAllocator.GetAddressOf())));
	//创建命令列表
	ANALYSIS_HRESULT(d3dDevice->CreateCommandList(
		0,//默认单个GPU
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,//直接调用
		commandAllocator.Get(),//关联命令分配器
		NULL	//管线状态
		, IID_PPV_ARGS(commandList.GetAddressOf())));
	commandList->Close();//关闭当前列表

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////交换链
	swapChain.Reset();
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = ZEngineRenderConfig::GetRenderConfig()->mScreenWidth;
	swapChainDesc.BufferDesc.Height = ZEngineRenderConfig::GetRenderConfig()->mScreenHeight;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = ZEngineRenderConfig::GetRenderConfig()->mFreshRate;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferCount = ZEngineRenderConfig::GetRenderConfig()->mSwapChainCount;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//使用表面或资源作为输出渲染目标
	swapChainDesc.OutputWindow = mHwnd;//指定主窗口
	swapChainDesc.Windowed = true;//以窗口方式运行
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;//交换缓冲后丢弃
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	dxgiFactory->CreateSwapChain(commandQueue.Get(), &swapChainDesc, swapChain.GetAddressOf());

	return false;
}
#endif