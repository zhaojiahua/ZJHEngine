#include "BoxMesh.h"
#include "core/MeshDataType.h"

void BoxMesh::Init()
{
	Super::Init();
}

void BoxMesh::Draw(float deltaTime)
{
	Super::Draw(deltaTime);
}

void BoxMesh::BuildMesh(const FMeshRenderingData* inrenderingdata)
{
	Super::BuildMesh(inrenderingdata);
}

BoxMesh* BoxMesh::CreateMesh()
{
	FMeshRenderingData meshrenderingdata;
	//顶点数据
	meshrenderingdata.vertexData.push_back(ZVertex(XMFLOAT3(0.f, 0.f, 0.f), XMFLOAT4(Colors::White)));
	meshrenderingdata.vertexData.push_back(ZVertex(XMFLOAT3(1.f, 0.f, 0.f), XMFLOAT4(Colors::Aqua)));
	meshrenderingdata.vertexData.push_back(ZVertex(XMFLOAT3(0.f, 1.f, 0.f), XMFLOAT4(Colors::Blue)));
	meshrenderingdata.vertexData.push_back(ZVertex(XMFLOAT3(1.f, 1.f, 0.f), XMFLOAT4(Colors::Chocolate)));
	meshrenderingdata.vertexData.push_back(ZVertex(XMFLOAT3(0.f, 1.f, 1.f), XMFLOAT4(Colors::BlanchedAlmond)));
	meshrenderingdata.vertexData.push_back(ZVertex(XMFLOAT3(1.f, 1.f, 1.f), XMFLOAT4(Colors::Cornsilk)));
	meshrenderingdata.vertexData.push_back(ZVertex(XMFLOAT3(0.f, 0.f, 1.f), XMFLOAT4(Colors::Coral)));
	meshrenderingdata.vertexData.push_back(ZVertex(XMFLOAT3(1.f, 0.f, 1.f), XMFLOAT4(Colors::DarkOrchid)));
	//索引数据
	//前
	meshrenderingdata.indexData.push_back(0); meshrenderingdata.indexData.push_back(1); meshrenderingdata.indexData.push_back(2);
	meshrenderingdata.indexData.push_back(1); meshrenderingdata.indexData.push_back(3); meshrenderingdata.indexData.push_back(2);
	//后
	meshrenderingdata.indexData.push_back(7); meshrenderingdata.indexData.push_back(6); meshrenderingdata.indexData.push_back(5);
	meshrenderingdata.indexData.push_back(6); meshrenderingdata.indexData.push_back(4); meshrenderingdata.indexData.push_back(7);
	//左
	meshrenderingdata.indexData.push_back(4); meshrenderingdata.indexData.push_back(6); meshrenderingdata.indexData.push_back(0);
	meshrenderingdata.indexData.push_back(4); meshrenderingdata.indexData.push_back(0); meshrenderingdata.indexData.push_back(2);
	//右
	meshrenderingdata.indexData.push_back(1); meshrenderingdata.indexData.push_back(7); meshrenderingdata.indexData.push_back(3);
	meshrenderingdata.indexData.push_back(3); meshrenderingdata.indexData.push_back(7); meshrenderingdata.indexData.push_back(5);
	//上
	meshrenderingdata.indexData.push_back(2); meshrenderingdata.indexData.push_back(3); meshrenderingdata.indexData.push_back(4);
	meshrenderingdata.indexData.push_back(4); meshrenderingdata.indexData.push_back(3); meshrenderingdata.indexData.push_back(5);
	//下
	meshrenderingdata.indexData.push_back(0); meshrenderingdata.indexData.push_back(6); meshrenderingdata.indexData.push_back(7);
	meshrenderingdata.indexData.push_back(0); meshrenderingdata.indexData.push_back(7); meshrenderingdata.indexData.push_back(1);



	BoxMesh* boxMesh = new BoxMesh;
	boxMesh->BuildMesh(&meshrenderingdata);

	return boxMesh;
}
