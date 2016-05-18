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
	int PartNumber;															// 第幾塊
	int InterLockType;														// 0 跟 1， 1 是凹，0 是凸
	QString PartName;														// 是地板，左牆還是右牆
	QVector<MyMesh::Point> InterLockingFace;								// 連接的面			0 1 2 3 4 5 6
};

QVector<InterLockingInfo *>	InfoVector;										// 存接起來面
QVector<MyMesh *>			Model;											// 所有的 model
QString						FilePathLocation;								// 現在讀到 txt 的位置
void ReadFile(char *);
void AddInterLocking();														// 卡榫的步驟
void SaveAllModel();