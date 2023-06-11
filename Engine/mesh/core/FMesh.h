#pragma once
#include "../../rendering/core/Rendering.h"
#include "MeshDataType.h"

using namespace Microsoft::WRL;

class FMesh : public IRenderingInterface
{
public:
	//构造函数
	FMesh();
	//初始化
	virtual void Init()override;
	//绘制
	virtual void Draw(float deltaTime)override;
	//构建Mesh资产
	virtual void BuildMesh(const FMeshRenderingData* inrenderingdata);
	//传入顶点数据创建一个Mesh资产
	static FMesh* CreateMesh(FMeshRenderingData* inrenderingdata);
	//构建顶点缓冲区视图和索引缓冲区视图
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();
protected:
	//模型基础数据
	ComPtr<ID3DBlob> mCPUVertexBufferPtr;	//CPU的顶点缓冲区
	ComPtr<ID3DBlob> mCPUIndexBufferPtr;	//CPU的索引缓冲区
	ComPtr<ID3D12Resource> mGPUVertexBufferPtr;//GPU的顶点缓冲区
	ComPtr<ID3D12Resource> mGPUIndexBufferPtr;//GPU的索引缓冲区
	ComPtr<ID3D12Resource> mGPUVertexBufferTempPtr;//CPU向GPU传送顶点数据时候的中间缓冲区
	ComPtr<ID3D12Resource> mGPUIndexBufferTempPtr;//CPU向GPU传送索引数据时候的中间缓冲区

	UINT vertexSizeInBytes;//顶点数据区大小
	UINT vertexStrideInBytes;//顶点数据单位大小
	UINT indexSizeInBytes;//索引区大小
	DXGI_FORMAT indexFormat;//存储索引的格式
	UINT indexSize;//索引数量
private:
	
};

