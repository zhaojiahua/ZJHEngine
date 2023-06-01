#pragma once
#include "core/FMesh.h"
class BoxMesh :    public FMesh
{
	typedef FMesh Super;
public:
	//初始化
	virtual void Init()override;
	//绘制
	virtual void Draw(float deltaTime)override;
	//构建Mesh资产
	void BuildMesh(const FMeshRenderingData* inrenderingdata)override;
	//传入顶点数据创建一个Mesh资产
	static BoxMesh* CreateMesh();
};

