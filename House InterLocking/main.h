#include <QDebug>
#include <QVector>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Mesh/Status.hh>

typedef OpenMesh::PolyMesh_ArrayKernelT<> MyMesh;

void ReadModel();
void AddInterLocking();	
float Parabola_Function(float);							// 產生卡榫，拋物線的弧形

const float InterLocking_Size = 3.f;					// 卡榫的正方形寬度
const float InterLocking_Depth = 1.8f;					// 卡榫的深度
const float InterLocking_Depth_Scale = 0.5f;			// 卡榫還要再多加 Depth幾倍，讓卡榫為尖的
const float InterLocking_Gap = 0.6f;					// 卡榫中間的間隔
const float InterLocking_Height = 0.32f;				// 卡榫凸起來的高度
//const float TypeConvexGap = 0.0
//const float InterLocking_Height_Scale = 1.2f;			// 卡榫動的寬度，呈上這個比例
const float InterLocking_Height_Gap = 0.01f;			// For 迴圈，跑那個弧形