#include <QDebug>
#include <QFile>
#include <QVector>
#include <QIODevice>
#include <QTextStream>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

typedef OpenMesh::PolyMesh_ArrayKernelT<> MyMesh;

struct InterLockingInfo
{
	int PartNumber;												// �ĴX��
	int InterLockType;											// 0 �� 1
	QString PartName;											// �O�a�O�A�����٬O�k��
	QVector<MyMesh::Point> InterLockingFace;					// �s������
};

QVector<InterLockingInfo>	InfoVector;							// �s���_�ӭ�
QVector<MyMesh *>			Model;								// �Ҧ��� model

void ReadFile(char *);
void AddInterLocking();