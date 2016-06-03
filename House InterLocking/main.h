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

const float InterLocking_Size = 1.4f;					// �d�g������μe��
const float InterLocking_Depth = 0.8f;					// �d�g���`��
const float InterLocking_Depth_Scale = 0.5f;			// �d�g�٭n�A�h�[ Depth�X���A���d�g���y��
const float InterLocking_Gap = 0.25f;					// �d�g���������j
const float InterLocking_Height = 0.1f;					// �d�g�Y�_�Ӫ�����
const float BeforeGapRate = 0.1f;						// �b�d�g���e�����Z�A�e�`�`�ת��X�����X(0 ~ 1 ����)

const float InterLocking_Size_Z_Scale = 0.8;			// ��e���ɭԡA�|���H�o�Ӥ�A�������p���ܫp����
const float InterLocking_Size_X_Scale = 1;				// ������ɭ�""""""""""""""""""""""""""""""""""
const float InterLocking_Height_Gap = 0.01f;			// For �j��A�]���ө���
const float InterLocking_Gap_Offset = 0.1f;				// �d�g�̫᪺�e��
