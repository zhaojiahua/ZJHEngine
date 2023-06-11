#include "FMesh.h"
using namespace DirectX;

FMesh::FMesh() :vertexSizeInBytes(0), vertexStrideInBytes(0), indexSizeInBytes(0), indexFormat(DXGI_FORMAT_R16_UINT), indexSize(0)
{}

void FMesh::Init()
{
}

void FMesh::Draw(float deltaTime)
{
	D3D12_VERTEX_BUFFER_VIEW VBV = GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW IBV = GetIndexBufferView();
	//����Ⱦ��ˮ���ϵ������,����������װ�����׶δ��붥������
	GetdcommandList()->IASetIndexBuffer(&IBV);
	GetdcommandList()->IASetVertexBuffers(
			0,//��ʼ�����0-15
			1,//��ͼ����
			&VBV);
	
	//���û���ͼԪΪ������
	GetdcommandList()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//���ƶ�������
	GetdcommandList()->DrawIndexedInstanced(indexSize,1,0,0,0);
}

FMesh* FMesh::CreateMesh(FMeshRenderingData* inrenderingdata)
{
	FMesh* newMesh = new FMesh();
	newMesh->BuildMesh(inrenderingdata);
	return newMesh;
}

D3D12_VERTEX_BUFFER_VIEW FMesh::GetVertexBufferView()
{
	D3D12_VERTEX_BUFFER_VIEW VBV;
	VBV.BufferLocation == mGPUVertexBufferPtr->GetGPUVirtualAddress();
	VBV.SizeInBytes = vertexSizeInBytes;
	VBV.StrideInBytes = vertexStrideInBytes;
	return VBV;
}

D3D12_INDEX_BUFFER_VIEW FMesh::GetIndexBufferView()
{
	D3D12_INDEX_BUFFER_VIEW IBV;
	IBV.BufferLocation = mGPUIndexBufferPtr->GetGPUVirtualAddress();
	IBV.SizeInBytes = indexSizeInBytes;
	IBV.Format = indexFormat;
	return IBV;
}

void FMesh::BuildMesh(const FMeshRenderingData* inrenderingdata)
{
	vertexStrideInBytes = sizeof(ZVertex);
	//��ȡģ�����ݴ�С
	vertexSizeInBytes = inrenderingdata->vertexData.size() * vertexStrideInBytes;
	indexSize = inrenderingdata->indexData.size();
	indexSizeInBytes = indexSize * sizeof(uint16_t);

	//��ģ�����ݿ�����ָ���ڴ滺����(CPU������)
	ANALYSIS_HRESULT(D3DCreateBlob(vertexSizeInBytes, &mCPUVertexBufferPtr));//������Ӧ��С��CPU���㻺����
	memcpy(mCPUVertexBufferPtr->GetBufferPointer(), inrenderingdata->vertexData.data(), vertexSizeInBytes);
	ANALYSIS_HRESULT(D3DCreateBlob(indexSizeInBytes, &mCPUIndexBufferPtr));//������Ӧ��С��CPU��������������
	memcpy(mCPUIndexBufferPtr->GetBufferPointer(), inrenderingdata->indexData.data(), indexSizeInBytes);
	//����GPU������
	mGPUVertexBufferPtr = ConstructDefaultBuffer(mGPUVertexBufferTempPtr, inrenderingdata->vertexData.data(), vertexSizeInBytes);
	mGPUIndexBufferPtr = ConstructDefaultBuffer(mGPUIndexBufferTempPtr, inrenderingdata->indexData.data(), indexSizeInBytes);

}
