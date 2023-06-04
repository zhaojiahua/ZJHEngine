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
	//��ȡģ�����ݴ�С
	UINT vertexSize = inrenderingdata->vertexData.size();
	UINT indexSize = inrenderingdata->indexData.size();

	//��ģ�����ݿ�����ָ���ڴ滺����(CPU������)
	ANALYSIS_HRESULT(D3DCreateBlob(vertexSize, &mCPUVertexBufferPtr));//������Ӧ��С��CPU���㻺����
	memcpy(mCPUVertexBufferPtr->GetBufferPointer(), inrenderingdata->vertexData.data(), vertexSize);
	ANALYSIS_HRESULT(D3DCreateBlob(indexSize, &mCPUIndexBufferPtr));//������Ӧ��С��CPU��������������
	memcpy(mCPUIndexBufferPtr->GetBufferPointer(), inrenderingdata->indexData.data(), indexSize);
	//����GPU������
	mGPUVertexBufferPtr = ConstructDefaultBuffer(mGPUVertexBufferTempPtr, inrenderingdata->vertexData.data(), vertexSize);
	mGPUIndexBufferPtr = ConstructDefaultBuffer(mGPUIndexBufferTempPtr, inrenderingdata->indexData.data(), indexSize);

}
