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
	Concave =	1,			// 凹
	Convex =	0			// 凸
};
typedef OpenMesh::PolyMesh_ArrayKernelT<> MyMesh;

struct InterLockingInfo
{
	int PartNumber;															// 第幾塊
	QVector<int> InterLockType;												
	QString PartName;														// 是地板，左牆還是右牆
	QVector<MyMesh::Point> InterLockingFace;								// 連接的面			0 1 2 3 4 5 6
};

QVector<InterLockingInfo *>	InfoVector;										// 存接起來面
QVector<MyMesh *>			Model;											// 所有的 model
QString						FilePathLocation;								// 現在讀到 txt 的位置

const float ConvexGap = 0.2f;												// 凸的卡榫，要向內縮
const float InterLocking_Height = 1.2f;										// 卡榫高度
const float ConcaveHeightGap = 0.2f;										// Type 1 卡榫高度，要加深
void ReadFile(char *);
void AddInterLocking();														// 卡榫的步驟
void SaveAllModel();