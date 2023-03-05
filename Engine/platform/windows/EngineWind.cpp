#include "EngineWind.h"
#include "WinMessageProcess.h"
#include "../../Debugs/EngineDebug.h"
#include "../../config/EngineConfig.h"

#if defined(_WIN32)
int ZEngineWind::PreInit(ZWinMainCmdParameters inparas)
{
	//��־ϵͳ����
	const char LogPath[] = "log";
	init_log_system(LogPath);
	ZLog_sucess("Log init.");
	//��Ϣ����


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
	windClass.cbSize = sizeof(WNDCLASSEX);//�Ķ���ռ���ڴ��С
	windClass.cbClsExtra = 0;//�Ƿ���Ҫ����ռ�
	windClass.cbWndExtra = 0;//�Ƿ���Ҫ�����ڴ� 
	windClass.hbrBackground = nullptr;//Ĭ��GDI��������
	windClass.hCursor = LoadCursor(NULL, IDC_ARROW);//����һ����ͷ���
	windClass.hIcon = nullptr;//Ӧ�ó�����ڴ�������ʾ��ͼ��
	windClass.hIconSm = NULL;//Ӧ�ó������Ͻ���ʾ��ͼ��
	windClass.hInstance = inparas.mPreInstance;//����ʵ��
	windClass.lpszClassName = L"ZJHEngine";//��������
	windClass.lpszMenuName = nullptr;
	windClass.style = CS_VREDRAW | CS_HREDRAW;//��ֱ��ˮƽ�ػ���
	windClass.lpfnWndProc = (WNDPROC)EngineWndProc;//������Ϣ�Ļص�����

	//ע�ᴰ��
	ATOM registerAtom = RegisterClassEx(&windClass);
	if (!registerAtom) {
		ZLog_error("Register window failed.");
		MessageBox(NULL, L"Register failed", L"Error", MB_OK);
	}
	//���ô���
	RECT rect = { 0,0,ZEngineRenderConfig::GetRenderConfig()->mScreenWidth,ZEngineRenderConfig::GetRenderConfig()->mScreenHeight };//���ڷֱ���
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, NULL);

	int wndwidth = rect.right - rect.left;
	int wndheight = rect.bottom - rect.top;

	//��������ʵ��
	mHwnd = CreateWindowEx(
		NULL,//���ڶ�����
		L"ZJHEngine",//������
		L"ZJHEngineWnd",//��ʾ�ڴ��ڱ�����������
		WS_OVERLAPPEDWINDOW,//���ڷ��
		100,100,//��������
		wndwidth, wndheight,//���ڿ��
		NULL,//�����ھ��
		nullptr,//�˵����
		inparas.mPreInstance,//����ʵ��
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

	ANALYSIS_HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)));	//��������

	HRESULT d3dDeviceResult = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice));//��������
	if (FAILED(d3dDeviceResult)) {
		//�������Ӳ��������ʧ�ܾʹ������������
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
	ANALYSIS_HRESULT(d3dDevice->CreateCommandList(
		0,//Ĭ�ϵ���GPU
		D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,//ֱ�ӵ���
		commandAllocator.Get(),//�������������
		NULL	//����״̬
		, IID_PPV_ARGS(commandList.GetAddressOf())));
	commandList->Close();//�رյ�ǰ�б�

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////������
	swapChain.Reset();
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = ZEngineRenderConfig::GetRenderConfig()->mScreenWidth;
	swapChainDesc.BufferDesc.Height = ZEngineRenderConfig::GetRenderConfig()->mScreenHeight;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = ZEngineRenderConfig::GetRenderConfig()->mFreshRate;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferCount = ZEngineRenderConfig::GetRenderConfig()->mSwapChainCount;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;//ʹ�ñ������Դ��Ϊ�����ȾĿ��
	swapChainDesc.OutputWindow = mHwnd;//ָ��������
	swapChainDesc.Windowed = true;//�Դ��ڷ�ʽ����
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;//�����������
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	dxgiFactory->CreateSwapChain(commandQueue.Get(), &swapChainDesc, swapChain.GetAddressOf());

	return false;
}
#endif