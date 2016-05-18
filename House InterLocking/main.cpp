#include "main.h"

void ReadFile(char* inputFile)
{
	QString tempStr = QString(inputFile);
	QFile file(tempStr);
	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug() << "Input File can't open !!";
		return;
	}
	QTextStream ss(&file);
	int tempInt,totalNumber;
	float tempFloat, tempX, tempY, tempZ;
	int tempType;

	QString InputFileStr(inputFile);
	QStringList FilePathList = InputFileStr.split("/");
	FilePathLocation = "";
	for (int i = 0; i < FilePathList.length() - 1; i++)
		FilePathLocation += FilePathList[i] + "/";

	ss >> totalNumber;
	qDebug() << "Total Number => " << totalNumber;
	for (int i = 0; i < totalNumber; i++)
	{
		MyMesh *tempMesh = new MyMesh();
		InterLockingInfo *info = new InterLockingInfo();
		ss >> tempStr;
		OpenMesh::IO::read_mesh(*tempMesh, (FilePathLocation + tempStr + ".obj").toLocal8Bit().data());
		qDebug() << "FilePath => " << (FilePathLocation + tempStr + ".obj");
		tempStr.replace("model_part", "");
		info->PartNumber = tempStr.toInt();
		qDebug() << "Number of part => " << info->PartNumber;
		qDebug() << "Mesh => " << (tempMesh->faces_begin() == tempMesh->faces_end() ? true : false);


		tempMesh->request_vertex_status();
		tempMesh->request_edge_status();
		tempMesh->request_face_status();
		Model.push_back(tempMesh);

		ss >> info->PartName;
		ss >> tempInt;
		for (int j = 0; j < tempInt / 2; j++)
			for (int k = 0; k < 6; k++)
			{
				ss >> tempX >> tempY >> tempZ >> tempType;
				//qDebug() << tempX << tempY << tempZ << tempType;
				switch (k)
				{
				case 0:
				case 3:
					info->InterLockType.push_back(tempType);
					info->InterLockingFace.push_back(MyMesh::Point(tempX, tempY, tempZ));
					break;
				default:
					info->InterLockingFace.push_back(MyMesh::Point(tempX, tempY, tempZ));
					break;
				}
			}

		qDebug() << "InterLockingFace length => " << info->InterLockingFace.length();
		//for (int j = 0; j < info->InterLockingFace.length(); j++)
		//	qDebug() << info->InterLockingFace[j][0] << info->InterLockingFace[j][1] << info->InterLockingFace[j][2];

		InfoVector.push_back(info);
	}
	file.close();
}

void AddInterLocking()
{
	int PartNumber;
	int state;
	bool NotFace;
	QVector<MyMesh::Point>				PointStack;				// 點的 stack
	QVector<int>						AddInterLockingType;
	QVector<QVector<MyMesh::Point>>		AddPointStack;
	QVector<MyMesh::VertexHandle>		AddHandleStack;
	MyMesh::Point *TempVertex;   // = new MyMesh::Point[4];
	MyMesh::Point Center;
	for (int i = 0; i < InfoVector.length(); i++)
	{
		AddPointStack.clear();
		AddInterLockingType.clear();
		int PartNumber = InfoVector[i]->PartNumber;
		if (InfoVector[i]->PartName.contains("Wall"))
		{
			for (MyMesh::FaceIter f_it = Model[PartNumber]->faces_begin(); f_it != Model[PartNumber]->faces_end(); ++f_it)
			{
				PointStack.clear();
				for (MyMesh::FaceVertexIter fv_it = Model[PartNumber]->fv_iter(f_it); fv_it.is_valid(); ++fv_it)
					PointStack.push_back(Model[PartNumber]->point(fv_it));

				// Check Y 是不是平的
				if (PointStack[0][1] == PointStack[1][1] && PointStack[1][1] == PointStack[2][1])
					for (int j = 0; j < InfoVector[i]->InterLockingFace.length(); j+=3)
						if (InfoVector[i]->InterLockingFace[j][0] == PointStack[0][0] && InfoVector[i]->InterLockingFace[j][1] == PointStack[0][1] && InfoVector[i]->InterLockingFace[j][2] == PointStack[0][2] &&
							InfoVector[i]->InterLockingFace[j + 1][0] == PointStack[1][0] && InfoVector[i]->InterLockingFace[j + 1][1] == PointStack[1][1] && InfoVector[i]->InterLockingFace[j + 1][2] == PointStack[1][2] &&
							InfoVector[i]->InterLockingFace[j + 2][0] == PointStack[2][0] && InfoVector[i]->InterLockingFace[j + 2][1] == PointStack[2][1] && InfoVector[i]->InterLockingFace[j + 2][2] == PointStack[2][2])
						{
							AddPointStack.push_back(PointStack);
							AddInterLockingType.push_back(InfoVector[i]->InterLockType[j / 3]);				// 要再除以2 是因為兩個面共用同一個卡榫結構
							Model[PartNumber]->delete_face(f_it, false);
							break;
						}
			}
			Model[PartNumber]->garbage_collection();
			
			// 開始加卡榫
			for (int j = 0; j < AddPointStack.length(); j+=2)
			{
				Center[0] = (AddPointStack[j][0][0] + AddPointStack[j][1][0] + AddPointStack[j][2][0] + AddPointStack[j + 1][2][0]) / 4;
				Center[1] = AddPointStack[j][0][1];
				Center[2] = (AddPointStack[j][0][2] + AddPointStack[j][1][2] + AddPointStack[j][2][2] + AddPointStack[j + 1][2][2]) / 4;
					
				TempVertex = new MyMesh::Point[4];
				TempVertex[0][0] = (Center[0] + AddPointStack[j][0][0]) / 2 - (AddInterLockingType[j] ==  LockType::Convex ? qBound(-1.0f, AddPointStack[j][0][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[0][1] = Center[1];
				TempVertex[0][2] = (Center[2] + AddPointStack[j][0][2]) / 2 - (AddInterLockingType[j] ==  LockType::Convex ? qBound(-1.0f, AddPointStack[j][0][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[1][0] = (Center[0] + AddPointStack[j][1][0]) / 2 - (AddInterLockingType[j] ==  LockType::Convex ? qBound(-1.0f, AddPointStack[j][1][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[1][1] = Center[1];
				TempVertex[1][2] = (Center[2] + AddPointStack[j][1][2]) / 2 - (AddInterLockingType[j] ==  LockType::Convex ? qBound(-1.0f, AddPointStack[j][1][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[2][0] = (Center[0] + AddPointStack[j][2][0]) / 2 - (AddInterLockingType[j] ==  LockType::Convex ? qBound(-1.0f, AddPointStack[j][2][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[2][1] = Center[1];
				TempVertex[2][2] = (Center[2] + AddPointStack[j][2][2]) / 2 - (AddInterLockingType[j] ==  LockType::Convex ? qBound(-1.0f, AddPointStack[j][2][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[3][0] = (Center[0] + AddPointStack[j + 1][2][0]) / 2 - (AddInterLockingType[j] ==  LockType::Convex ? qBound(-1.0f, AddPointStack[j + 1][2][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[3][1] = Center[1];
				TempVertex[3][2] = (Center[2] + AddPointStack[j + 1][2][2]) / 2 - (AddInterLockingType[j] ==  LockType::Convex ? qBound(-1.0f, AddPointStack[j + 1][2][2] - Center[2], 1.0f) : 0) * ConvexGap;
				
				#pragma region 產生有洞的面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j + 1][2]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j + 1][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion 
				#pragma region 產生向下的卡榫
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion
				#pragma region 產生最後的面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (AddInterLockingType[j] ==  LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion
			}
		}
		else if (InfoVector[i]->PartName == "base")
		{
			for (int j = 0; j < InfoVector[i]->InterLockingFace.length() / 4; j++)
				if (InfoVector[i]->InterLockingFace[j][1] == InfoVector[i]->InterLockingFace[j + 1][1] && InfoVector[i]->InterLockingFace[j + 1][1] == InfoVector[i]->InterLockingFace[j + 2][1] &&
					InfoVector[i]->InterLockingFace[j + 2][1] == InfoVector[i]->InterLockingFace[j + 3][1])
				{

				}
		}
		else if (InfoVector[i]->PartName.contains("Triangle"))
		{
			for (MyMesh::FaceIter f_it = Model[PartNumber]->faces_begin(); f_it != Model[PartNumber]->faces_end(); ++f_it)
			{
				PointStack.clear();
				for (MyMesh::FaceVertexIter fv_it = Model[PartNumber]->fv_iter(f_it); fv_it.is_valid(); ++fv_it)
					PointStack.push_back(Model[PartNumber]->point(fv_it));

				// Check Y 是不是平的
				if (PointStack[0][1] == PointStack[1][1] && PointStack[1][1] == PointStack[2][1])
					for (int j = 0; j < InfoVector[i]->InterLockingFace.length(); j += 3)
						if (InfoVector[i]->InterLockingFace[j][0] == PointStack[0][0] && InfoVector[i]->InterLockingFace[j][1] == PointStack[0][1] && InfoVector[i]->InterLockingFace[j][2] == PointStack[0][2] &&
							InfoVector[i]->InterLockingFace[j + 1][0] == PointStack[1][0] && InfoVector[i]->InterLockingFace[j + 1][1] == PointStack[1][1] && InfoVector[i]->InterLockingFace[j + 1][2] == PointStack[1][2] &&
							InfoVector[i]->InterLockingFace[j + 2][0] == PointStack[2][0] && InfoVector[i]->InterLockingFace[j + 2][1] == PointStack[2][1] && InfoVector[i]->InterLockingFace[j + 2][2] == PointStack[2][2] &&
							InfoVector[i]->InterLockType[j / 3] ==  LockType::Convex)
						{
							AddPointStack.push_back(PointStack);
							AddInterLockingType.push_back(InfoVector[i]->InterLockType[j / 3]);				// 要再除以2 是因為兩個面共用同一個卡榫結構
							Model[PartNumber]->delete_face(f_it, false);
							break;
						}
			}
			Model[PartNumber]->garbage_collection();

			// 開始加卡榫
			for (int j = 0; j < AddPointStack.length(); j += 2)
			{
				Center[0] = (AddPointStack[j][0][0] + AddPointStack[j][1][0] + AddPointStack[j][2][0] + AddPointStack[j + 1][2][0]) / 4;
				Center[1] = AddPointStack[j][0][1];
				Center[2] = (AddPointStack[j][0][2] + AddPointStack[j][1][2] + AddPointStack[j][2][2] + AddPointStack[j + 1][2][2]) / 4;

				TempVertex = new MyMesh::Point[4];
				TempVertex[0][0] = (Center[0] + AddPointStack[j][0][0]) / 2 - (AddInterLockingType[j] == 0 ? qBound(-1.0f, AddPointStack[j][0][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[0][1] = Center[1];
				TempVertex[0][2] = (Center[2] + AddPointStack[j][0][2]) / 2 - (AddInterLockingType[j] == 0 ? qBound(-1.0f, AddPointStack[j][0][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[1][0] = (Center[0] + AddPointStack[j][1][0]) / 2 - (AddInterLockingType[j] == 0 ? qBound(-1.0f, AddPointStack[j][1][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[1][1] = Center[1];
				TempVertex[1][2] = (Center[2] + AddPointStack[j][1][2]) / 2 - (AddInterLockingType[j] == 0 ? qBound(-1.0f, AddPointStack[j][1][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[2][0] = (Center[0] + AddPointStack[j][2][0]) / 2 - (AddInterLockingType[j] == 0 ? qBound(-1.0f, AddPointStack[j][2][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[2][1] = Center[1];
				TempVertex[2][2] = (Center[2] + AddPointStack[j][2][2]) / 2 - (AddInterLockingType[j] == 0 ? qBound(-1.0f, AddPointStack[j][2][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[3][0] = (Center[0] + AddPointStack[j + 1][2][0]) / 2 - (AddInterLockingType[j] == 0 ? qBound(-1.0f, AddPointStack[j + 1][2][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[3][1] = Center[1];
				TempVertex[3][2] = (Center[2] + AddPointStack[j + 1][2][2]) / 2 - (AddInterLockingType[j] == 0 ? qBound(-1.0f, AddPointStack[j + 1][2][2] - Center[2], 1.0f) : 0) * ConvexGap;

				#pragma region 產生有洞的面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j + 1][2]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(AddPointStack[j + 1][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion 
				#pragma region 產生向下的卡榫
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion
				#pragma region 產生最後的面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (AddInterLockingType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion
			}
			qDebug() << "Model_part" << PartNumber << " Need To Add " << AddPointStack.length() << " triangles !!";
		}
	}
	// Base 最後做
}

void SaveAllModel()
{
	if (!QDir(FilePathLocation + "/Output/").exists())
		QDir().mkdir(FilePathLocation + "/Output/");
	for (int i = 0; i < Model.length(); i++)
		if (!OpenMesh::IO::write_mesh(*Model[i], QString(FilePathLocation + "Output/Final_model_part" + QString::number(InfoVector[i]->PartNumber, 10) + ".obj").toLocal8Bit().data()))
			qDebug() << ("Fail to save the obj => model_part" + QString::number(InfoVector[i]->PartNumber, 10) + ".obj");
}

int main(int argc, char *argv[])
{
	if (argc == 2)
	{
		ReadFile(argv[1]);
		AddInterLocking();
		SaveAllModel();
	}
	else if (argc == 1)
	{
		ReadFile("../x64/Debug/House/info.txt");
		AddInterLocking();
		SaveAllModel();
		system("PAUSE");
	}
	else
		qDebug() << "Please add file path !!";
	return 0;
}