#include "FMesh.h"
using namespace DirectX;

void FMesh::Init()
{
}

void FMesh::Draw(float deltaTime)
{
}

FMesh* FMesh::CreateMesh(FMeshRenderingData* inrenderingdata)
{
	FMesh* newMesh = new FMesh();
	newMesh->BuildMesh(inrenderingdata);
	return newMesh;
}

void FMesh::BuildMesh(const FMeshRenderingData* inrenderingdata)
{
	//获取模型数据大小
	UINT vertexSize = inrenderingdata->vertexData.size();
	UINT indexSize = inrenderingdata->indexData.size();

	//把模型数据拷贝到指定内存缓冲区(CPU缓冲区)
	ANALYSIS_HRESULT(D3DCreateBlob(vertexSize, &mCPUVertexBufferPtr));//创建相应大小的CPU顶点缓冲区
	memcpy(mCPUVertexBufferPtr->GetBufferPointer(), inrenderingdata->vertexData.data(), vertexSize);
	ANALYSIS_HRESULT(D3DCreateBlob(indexSize, &mCPUIndexBufferPtr));//创建相应大小的CPU顶点索引缓冲区
	memcpy(mCPUIndexBufferPtr->GetBufferPointer(), inrenderingdata->indexData.data(), indexSize);
	//创建GPU缓冲区
	mGPUVertexBufferPtr = ConstructDefaultBuffer(mGPUVertexBufferTempPtr, inrenderingdata->vertexData.data(), vertexSize);
	mGPUIndexBufferPtr = ConstructDefaultBuffer(mGPUIndexBufferTempPtr, inrenderingdata->indexData.data(), indexSize);

}
