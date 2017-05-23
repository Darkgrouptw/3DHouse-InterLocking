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
	Concave	= 1,				// �W
	Convex	= 0					// �r
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
	QVector<MyMesh *>					ModelsArray;									// �s Model �� Array
	QVector<InterLockingInfo *>			InfoArray;										// �s Info �� Array

	QString								FilePathLocation;								// �n�O�� info ���ؿ���m�A�o�ˮ���L Model ���ɭԭn�[�W�o�Ӧ�m
};

