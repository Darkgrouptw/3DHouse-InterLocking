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
	Concave	= 1,				// 凹
	Convex	= 0					// 禿
};*/
struct ModelInfo
{
	int PartNumber;																		// Part Number
	QString PartName;																	// Part 的名稱 (Ex: roof, base)
};
struct SplitModelInfo
{
	int OrgModelIndex;																	// 對應原本 Model 的 index		(roof1, roof2, roof3, base1, base2, base3, base4)
	int StartModelIndex;																// 對應到 Split Model 的 Index	(roof(0) ,base(3))
	int SplitCount;																		// 有幾個						(roof(3) ,base(4))

	int PartNumber;																		// Part Number
	QString PartName;																	// Part 的名稱
};

typedef OpenMesh::PolyMesh_ArrayKernelT<> MyMesh;
using namespace std;

class InterLockClass
{
public:
	InterLockClass(char *);
	~InterLockClass();

	void								SplitInSmallSize();								// 將東西切成小塊的
	void								SaveAllModel();									// 儲存 Model
private:
	float								GetNextValue(float, float, float);				// 會回傳給你下一次要做的值

	QVector<MyMesh *>					ModelsArray;									// 存 Model 的 Array
	QVector<ModelInfo *>				InfoArray;										// 存 Info 的 Array

	QVector<MyMesh *>					SplitModelsArray;								// 分開的 Model 的 Array
	QVector<SplitModelInfo *>			SplitInfoArray;									// 分開的 Info 的 Array 

	QString								FilePathLocation;								// 要記錄 info 的目錄位置，這樣拿其他 Model 的時候要加上這個位置

	const QString						outputFileEnd = ".obj";							// 輸出種類
	const int							SplitSize = 4;									// 每一個 Unit 的大小
};

