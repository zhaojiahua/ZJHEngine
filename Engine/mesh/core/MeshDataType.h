#pragma once
#include "../../EnginLunchMain.h"
using namespace DirectX;
struct ZVertex
{
	ZVertex(XMFLOAT3 inPos, XMFLOAT4 inColor);
	XMFLOAT3 position;//点位置数据
	XMFLOAT4 color;//颜色数据
};
struct FMeshRenderingData
{
	vector<ZVertex> vertexData;//顶点数据
	vector<uint16_t> indexData;//索引数据
};

