#pragma once
#include "../../rendering/core/Rendering.h"
#include "MeshDataType.h"

using namespace Microsoft::WRL;

class FMesh : public IRenderingInterface
{
public:
	//初始化
	virtual void Init()override;
	//绘制
	virtual void Draw(float deltaTime)override;
	//构建Mesh资产
	virtual void BuildMesh(const FMeshRenderingData* inrenderingdata);
	//传入顶点数据创建一个Mesh资产
	static FMesh* CreateMesh(FMeshRenderingData* inrenderingdata);
protected:
	//模型基础数据
	ComPtr<ID3DBlob> mCPUVertexBufferPtr;	//CPU的顶点缓冲区
	ComPtr<ID3DBlob> mCPUIndexBufferPtr;	//CPU的索引缓冲区
	ComPtr<ID3D12Resource> mGPUVertexBufferPtr;//GPU的顶点缓冲区
	ComPtr<ID3D12Resource> mGPUVertexBufferPtr;//GPU的索引缓冲区
	ComPtr<ID3D12Resource> mGPUVertexBufferTempPtr;//CPU向GPU传送顶点数据时候的中间缓冲区
	ComPtr<ID3D12Resource> mGPUVertexBufferTempPtr;//CPU向GPU传送索引数据时候的中间缓冲区

private:
	
};

