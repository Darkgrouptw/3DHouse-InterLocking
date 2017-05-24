#pragma once
#include <iostream>
#include <cmath>

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
struct ModelInfo
{
	int PartNumber;																		// Part Number
	QString PartName;																	// Part ���W�� (Ex: roof, base)
};
struct SplitModelInfo
{
	int OrgModelIndex;																	// �����쥻 Model �� index		(roof1, roof2, roof3, base1, base2, base3, base4)
	int StartModelIndex;																// ������ Split Model �� Index	(roof(0) ,base(3))
	int SplitCount;																		// ���X��						(roof(3) ,base(4))

	int PartNumber;																		// Part Number
	QString PartName;																	// Part ���W��
};

typedef OpenMesh::PolyMesh_ArrayKernelT<> MyMesh;
using namespace std;

class InterLockClass
{
public:
	InterLockClass(char *);
	~InterLockClass();

	void								SplitInSmallSize();								// �N�F������p����
	void								SaveAllModel();									// �x�s Model
private:
	float								GetNextValue(float, float, float);				// �|�^�ǵ��A�U�@���n������

	QVector<MyMesh *>					ModelsArray;									// �s Model �� Array
	QVector<ModelInfo *>				InfoArray;										// �s Info �� Array

	QVector<MyMesh *>					SplitModelsArray;								// ���}�� Model �� Array
	QVector<SplitModelInfo *>			SplitInfoArray;									// ���}�� Info �� Array 

	QString								FilePathLocation;								// �n�O�� info ���ؿ���m�A�o�ˮ���L Model ���ɭԭn�[�W�o�Ӧ�m

	const QString						outputFileEnd = ".obj";							// ��X����
	const int							SplitSize = 4;									// �C�@�� Unit ���j�p
};

