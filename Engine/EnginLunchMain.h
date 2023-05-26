#pragma once
#include "../simple_library/public/simple_library.h"
#include "Debugs/Logs/SimpleLog.h"

#include <vector>

#include <wrl.h>	//Windows提供的库(包括智能智能,事件,创建Component组件等)
#include <dxgi1_6.h>	//
//#include <d3d12.h>	//d3d12API
#include "rendering/DirectX12/d3dx12.h"	//微软提供的工具包,这里面包含了<d3d12.h>
#include <D3Dcompiler.h>	//着色代码
#include <DirectXMath.h>		//Direct提供的数学工具库
#include <DirectXPackedVector.h>	//向量相关的工具库
#include <DirectXCollision.h>	//碰撞相关的算法
#include <DirectXColors.h>	//颜色相关的名称表

using namespace std;
