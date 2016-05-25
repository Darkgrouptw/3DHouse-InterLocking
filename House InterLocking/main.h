#include <QDebug>
#include <QDir>
#include <QFile>
#include <QVector>
#include <QIODevice>
#include <QTextStream>
//#include <QtGui/QVector3D>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

enum LockType
{
	Concave =	1,			// �W
	Convex =	0			// �Y
};
typedef OpenMesh::PolyMesh_ArrayKernelT<> MyMesh;

struct InterLockingInfo
{
	int PartNumber;															// �ĴX��
	QVector<int> InterLockType;												
	QString PartName;														// �O�a�O�A�����٬O�k��
	QVector<MyMesh::Point> InterLockingFace;								// �s������			0 1 2 3 4 5 6
};

QVector<InterLockingInfo *>	InfoVector;										// �s���_�ӭ�
QVector<MyMesh *>			Model;											// �Ҧ��� model
QString						FilePathLocation;								// �{�bŪ�� txt ����m

const float ConvexGap = 0.2f;												// �Y���d�g�A�n�V���Y
const float InterLocking_Height = 1.2f;										// �d�g����
const float ConcaveHeightGap = 0.2f;										// Type 1 �d�g���סA�n�[�`
void ReadFile(char *);
void AddInterLocking();														// �d�g���B�J
void SaveAllModel();