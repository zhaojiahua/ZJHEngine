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
	//绑定渲染流水线上的输入槽,可以在输入装配器阶段传入顶点数据
	GetdcommandList()->IASetIndexBuffer(&IBV);
	GetdcommandList()->IASetVertexBuffers(
			0,//起始输入槽0-15
			1,//视图数量
			&VBV);
	
	//设置绘制图元为三角形
	GetdcommandList()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//绘制顶点数据
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
	//获取模型数据大小
	vertexSizeInBytes = inrenderingdata->vertexData.size() * vertexStrideInBytes;
	indexSize = inrenderingdata->indexData.size();
	indexSizeInBytes = indexSize * sizeof(uint16_t);

	//把模型数据拷贝到指定内存缓冲区(CPU缓冲区)
	ANALYSIS_HRESULT(D3DCreateBlob(vertexSizeInBytes, &mCPUVertexBufferPtr));//创建相应大小的CPU顶点缓冲区
	memcpy(mCPUVertexBufferPtr->GetBufferPointer(), inrenderingdata->vertexData.data(), vertexSizeInBytes);
	ANALYSIS_HRESULT(D3DCreateBlob(indexSizeInBytes, &mCPUIndexBufferPtr));//创建相应大小的CPU顶点索引缓冲区
	memcpy(mCPUIndexBufferPtr->GetBufferPointer(), inrenderingdata->indexData.data(), indexSizeInBytes);
	//创建GPU缓冲区
	mGPUVertexBufferPtr = ConstructDefaultBuffer(mGPUVertexBufferTempPtr, inrenderingdata->vertexData.data(), vertexSizeInBytes);
	mGPUIndexBufferPtr = ConstructDefaultBuffer(mGPUIndexBufferTempPtr, inrenderingdata->indexData.data(), indexSizeInBytes);

}
