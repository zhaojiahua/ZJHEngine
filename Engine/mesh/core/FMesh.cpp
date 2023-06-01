#include "FMesh.h"

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
}
