#include <QDebug>
#include <QVector>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Mesh/Status.hh>

typedef OpenMesh::PolyMesh_ArrayKernelT<> MyMesh;

void ReadModel();
void AddInterLocking();	
float Parabola_Function(float);							// ���ͥd�g�A�ߪ��u������

const float InterLocking_Size = 3.f;					// �d�g������μe��
const float InterLocking_Depth = 1.8f;					// �d�g���`��
const float InterLocking_Depth_Scale = 0.5f;			// �d�g�٭n�A�h�[ Depth�X���A���d�g���y��
const float InterLocking_Gap = 0.6f;					// �d�g���������j
const float InterLocking_Height = 0.32f;				// �d�g�Y�_�Ӫ�����
//const float TypeConvexGap = 0.0
//const float InterLocking_Height_Scale = 1.2f;			// �d�g�ʪ��e�סA�e�W�o�Ӥ��
const float InterLocking_Height_Gap = 0.01f;			// For �j��A�]���ө���