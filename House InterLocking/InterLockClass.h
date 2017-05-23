#pragma once
#include <iostream>

#include <QDir>
#include <QFile>
#include <QVector>
#include <QIODevice>
#include <QTextStream>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
/*enum LockType
{
	Concave	= 1,				// 凹
	Convex	= 0					// 禿
};*/
struct InterLockingInfo
{
	int PartNumber;																		// Part Number
	QString PartName;
};

typedef OpenMesh::PolyMesh_ArrayKernelT<> MyMesh;
using namespace std;

class InterLockClass
{
public:
	InterLockClass(char *);
	~InterLockClass();

private:
	QVector<MyMesh *>					ModelsArray;									// 存 Model 的 Array
	QVector<InterLockingInfo *>			InfoArray;										// 存 Info 的 Array

	QString								FilePathLocation;								// 要記錄 info 的目錄位置，這樣拿其他 Model 的時候要加上這個位置
};

