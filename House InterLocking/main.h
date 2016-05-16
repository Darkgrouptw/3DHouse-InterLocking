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
	int PartNumber;												// 第幾塊
	int InterLockType;											// 0 跟 1
	QString PartName;											// 是地板，左牆還是右牆
	QVector<MyMesh::Point> InterLockingFace;					// 連接的面
};

QVector<InterLockingInfo>	InfoVector;							// 存接起來面
QVector<MyMesh *>			Model;								// 所有的 model

void ReadFile(char *);
void AddInterLocking();