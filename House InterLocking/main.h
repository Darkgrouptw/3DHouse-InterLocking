#include <QDebug>
#include <QDir>
#include <QFile>
#include <QVector>
#include <QIODevice>
#include <QTextStream>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

typedef OpenMesh::PolyMesh_ArrayKernelT<> MyMesh;

struct InterLockingInfo
{
	int PartNumber;															// �ĴX��
	int InterLockType;														// 0 �� 1�A 1 �O�W�A0 �O�Y
	QString PartName;														// �O�a�O�A�����٬O�k��
	QVector<MyMesh::Point> InterLockingFace;								// �s������			0 1 2 3 4 5 6
};

QVector<InterLockingInfo *>	InfoVector;										// �s���_�ӭ�
QVector<MyMesh *>			Model;											// �Ҧ��� model
QString						FilePathLocation;								// �{�bŪ�� txt ����m
void ReadFile(char *);
void AddInterLocking();														// �d�g���B�J
void SaveAllModel();