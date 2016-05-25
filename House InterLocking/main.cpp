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
	QVector<MyMesh::Point>				PointStack;					// 點的 stack
	QVector<int>						AddInterLockingType;
	QVector<QVector<MyMesh::Point>>		OtherPoint;					// 除了水平以外的面
	QVector<int>						OtherType;					// 卡榫類別
	QVector<QVector<MyMesh::Point>>		AddPointStack;
	QVector<MyMesh::VertexHandle>		AddHandleStack;
	MyMesh::Point *TempVertex;
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
					for (int j = 0; j < InfoVector[i]->InterLockingFace.length(); j += 3)
						if (InfoVector[i]->InterLockingFace[j][0] == PointStack[0][0] && InfoVector[i]->InterLockingFace[j][1] == PointStack[0][1] && InfoVector[i]->InterLockingFace[j][2] == PointStack[0][2] &&
							InfoVector[i]->InterLockingFace[j + 1][0] == PointStack[1][0] && InfoVector[i]->InterLockingFace[j + 1][1] == PointStack[1][1] && InfoVector[i]->InterLockingFace[j + 1][2] == PointStack[1][2] &&
							InfoVector[i]->InterLockingFace[j + 2][0] == PointStack[2][0] && InfoVector[i]->InterLockingFace[j + 2][1] == PointStack[2][1] && InfoVector[i]->InterLockingFace[j + 2][2] == PointStack[2][2])
						{
							AddPointStack.push_back(PointStack);
							AddInterLockingType.push_back(InfoVector[i]->InterLockType[j / 3]);
							Model[PartNumber]->delete_face(f_it, false);
							break;
						}
				//else if (PointStack[0][2] == PointStack[1][2] && PointStack[1][2] == PointStack[2][2])		// Y 的水平
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
			if (InfoVector[i]->InterLockingFace.length() / 3 == 8)
			{
				#pragma region 確定最大的四個點，之後把她刪除
				TempVertex = new MyMesh::Point[4];
				TempVertex[0][0] = InfoVector[i]->InterLockingFace[0][0];
				TempVertex[0][1] = InfoVector[i]->InterLockingFace[0][1];
				TempVertex[0][2] = InfoVector[i]->InterLockingFace[0][2];

				TempVertex[1][0] = InfoVector[i]->InterLockingFace[6][0];
				TempVertex[1][1] = InfoVector[i]->InterLockingFace[6][1];
				TempVertex[1][2] = InfoVector[i]->InterLockingFace[6][2];

				TempVertex[2][0] = InfoVector[i]->InterLockingFace[7][0];
				TempVertex[2][1] = InfoVector[i]->InterLockingFace[7][1];
				TempVertex[2][2] = InfoVector[i]->InterLockingFace[7][2];

				TempVertex[3][0] = InfoVector[i]->InterLockingFace[17][0];
				TempVertex[3][1] = InfoVector[i]->InterLockingFace[17][1];
				TempVertex[3][2] = InfoVector[i]->InterLockingFace[17][2];

				for (MyMesh::FaceIter f_it = Model[PartNumber]->faces_begin(); f_it != Model[PartNumber]->faces_end(); ++f_it)
				{
					state = 0;
					for (MyMesh::FaceVertexIter fv_it = Model[PartNumber]->fv_iter(f_it); fv_it.is_valid(); ++fv_it)
					{
						MyMesh::Point p = Model[PartNumber]->point(fv_it);
						if ((p[0] == TempVertex[0][0] && p[1] == TempVertex[0][1] && p[2] == TempVertex[0][2]) ||
							(p[0] == TempVertex[1][0] && p[1] == TempVertex[1][1] && p[2] == TempVertex[1][2]) ||
							(p[0] == TempVertex[2][0] && p[1] == TempVertex[2][1] && p[2] == TempVertex[2][2]) ||
							(p[0] == TempVertex[3][0] && p[1] == TempVertex[3][1] && p[2] == TempVertex[3][2]))
							state++;
					}
					if (state == 3)
						Model[PartNumber]->delete_face(f_it, false);
				}
				Model[PartNumber]->garbage_collection();
				#pragma endregion 
				#pragma region 	產生其中第一塊
				Center[0] = (InfoVector[i]->InterLockingFace[0][0] + InfoVector[i]->InterLockingFace[1][0] + InfoVector[i]->InterLockingFace[2][0] + InfoVector[i]->InterLockingFace[5][0]) / 4;
				Center[1] = (InfoVector[i]->InterLockingFace[0][1] + InfoVector[i]->InterLockingFace[1][1] + InfoVector[i]->InterLockingFace[2][1] + InfoVector[i]->InterLockingFace[5][1]) / 4;
				Center[2] = (InfoVector[i]->InterLockingFace[0][2] + InfoVector[i]->InterLockingFace[1][2] + InfoVector[i]->InterLockingFace[2][2] + InfoVector[i]->InterLockingFace[5][2]) / 4;

				TempVertex[0][0] = (Center[0] + InfoVector[i]->InterLockingFace[0][0]) / 2 - (InfoVector[i]->InterLockType[0] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[0][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[0][1] = Center[1];
				TempVertex[0][2] = (Center[2] + InfoVector[i]->InterLockingFace[0][2]) / 2 - (InfoVector[i]->InterLockType[0] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[0][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[1][0] = (Center[0] + InfoVector[i]->InterLockingFace[1][0]) / 2 - (InfoVector[i]->InterLockType[0] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[1][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[1][1] = Center[1];
				TempVertex[1][2] = (Center[2] + InfoVector[i]->InterLockingFace[1][2]) / 2 - (InfoVector[i]->InterLockType[0] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[1][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[2][0] = (Center[0] + InfoVector[i]->InterLockingFace[2][0]) / 2 - (InfoVector[i]->InterLockType[0] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[2][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[2][1] = Center[1];
				TempVertex[2][2] = (Center[2] + InfoVector[i]->InterLockingFace[2][2]) / 2 - (InfoVector[i]->InterLockType[0] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[2][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[3][0] = (Center[0] + InfoVector[i]->InterLockingFace[5][0]) / 2 - (InfoVector[i]->InterLockType[0] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[5][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[3][1] = Center[1];
				TempVertex[3][2] = (Center[2] + InfoVector[i]->InterLockingFace[5][2]) / 2 - (InfoVector[i]->InterLockType[0] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[5][2] - Center[2], 1.0f) : 0) * ConvexGap;

				#pragma region 產生有洞的面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[5]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[5]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion 
				#pragma region 產生向下的卡榫
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion
				#pragma region 產生最後的面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (InfoVector[i]->InterLockType[0] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion
				#pragma endregion
				#pragma region 	產生其中第二塊
				Center[0] = (InfoVector[i]->InterLockingFace[6][0] + InfoVector[i]->InterLockingFace[7][0] + InfoVector[i]->InterLockingFace[8][0] + InfoVector[i]->InterLockingFace[11][0]) / 4;
				Center[1] = (InfoVector[i]->InterLockingFace[6][1] + InfoVector[i]->InterLockingFace[7][1] + InfoVector[i]->InterLockingFace[8][1] + InfoVector[i]->InterLockingFace[11][1]) / 4;
				Center[2] = (InfoVector[i]->InterLockingFace[6][2] + InfoVector[i]->InterLockingFace[7][2] + InfoVector[i]->InterLockingFace[8][2] + InfoVector[i]->InterLockingFace[11][2]) / 4;

				TempVertex[0][0] = (Center[0] + InfoVector[i]->InterLockingFace[6][0]) / 2 - (InfoVector[i]->InterLockType[1] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[6][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[0][1] = Center[1];
				TempVertex[0][2] = (Center[2] + InfoVector[i]->InterLockingFace[6][2]) / 2 - (InfoVector[i]->InterLockType[1] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[6][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[1][0] = (Center[0] + InfoVector[i]->InterLockingFace[7][0]) / 2 - (InfoVector[i]->InterLockType[1] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[7][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[1][1] = Center[1];
				TempVertex[1][2] = (Center[2] + InfoVector[i]->InterLockingFace[7][2]) / 2 - (InfoVector[i]->InterLockType[1] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[7][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[2][0] = (Center[0] + InfoVector[i]->InterLockingFace[8][0]) / 2 - (InfoVector[i]->InterLockType[1] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[8][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[2][1] = Center[1];
				TempVertex[2][2] = (Center[2] + InfoVector[i]->InterLockingFace[8][2]) / 2 - (InfoVector[i]->InterLockType[1] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[8][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[3][0] = (Center[0] + InfoVector[i]->InterLockingFace[11][0]) / 2 - (InfoVector[i]->InterLockType[1] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[11][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[3][1] = Center[1];
				TempVertex[3][2] = (Center[2] + InfoVector[i]->InterLockingFace[11][2]) / 2 - (InfoVector[i]->InterLockType[1] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[11][2] - Center[2], 1.0f) : 0) * ConvexGap;

				#pragma region 產生有洞的面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[6]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[7]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[6]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[8]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[7]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[8]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[6]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[6]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[11]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[8]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[11]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[8]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion 
				#pragma region 產生向下的卡榫
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion
				#pragma region 產生最後的面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (InfoVector[i]->InterLockType[1] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion
				#pragma endregion
				#pragma region 	產生其中第三塊
				Center[0] = (InfoVector[i]->InterLockingFace[12][0] + InfoVector[i]->InterLockingFace[13][0] + InfoVector[i]->InterLockingFace[14][0] + InfoVector[i]->InterLockingFace[17][0]) / 4;
				Center[1] = (InfoVector[i]->InterLockingFace[12][1] + InfoVector[i]->InterLockingFace[13][1] + InfoVector[i]->InterLockingFace[14][1] + InfoVector[i]->InterLockingFace[17][1]) / 4;
				Center[2] = (InfoVector[i]->InterLockingFace[12][2] + InfoVector[i]->InterLockingFace[13][2] + InfoVector[i]->InterLockingFace[14][2] + InfoVector[i]->InterLockingFace[17][2]) / 4;

				TempVertex[0][0] = (Center[0] + InfoVector[i]->InterLockingFace[12][0]) / 2 - (InfoVector[i]->InterLockType[2] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[12][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[0][1] = Center[1];
				TempVertex[0][2] = (Center[2] + InfoVector[i]->InterLockingFace[12][2]) / 2 - (InfoVector[i]->InterLockType[2] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[12][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[1][0] = (Center[0] + InfoVector[i]->InterLockingFace[13][0]) / 2 - (InfoVector[i]->InterLockType[2] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[13][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[1][1] = Center[1];
				TempVertex[1][2] = (Center[2] + InfoVector[i]->InterLockingFace[13][2]) / 2 - (InfoVector[i]->InterLockType[2] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[13][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[2][0] = (Center[0] + InfoVector[i]->InterLockingFace[14][0]) / 2 - (InfoVector[i]->InterLockType[2] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[14][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[2][1] = Center[1];
				TempVertex[2][2] = (Center[2] + InfoVector[i]->InterLockingFace[14][2]) / 2 - (InfoVector[i]->InterLockType[2] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[14][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[3][0] = (Center[0] + InfoVector[i]->InterLockingFace[17][0]) / 2 - (InfoVector[i]->InterLockType[2] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[17][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[3][1] = Center[1];
				TempVertex[3][2] = (Center[2] + InfoVector[i]->InterLockingFace[17][2]) / 2 - (InfoVector[i]->InterLockType[2] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[17][2] - Center[2], 1.0f) : 0) * ConvexGap;

				#pragma region 產生有洞的面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[12]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[13]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[12]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[14]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[13]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[14]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[12]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[12]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[17]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[14]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[17]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[14]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion 
				#pragma region 產生向下的卡榫
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion
				#pragma region 產生最後的面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (InfoVector[i]->InterLockType[2] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion
				#pragma endregion
				#pragma region 	產生其中第四塊
				Center[0] = (InfoVector[i]->InterLockingFace[18][0] + InfoVector[i]->InterLockingFace[19][0] + InfoVector[i]->InterLockingFace[20][0] + InfoVector[i]->InterLockingFace[23][0]) / 4;
				Center[1] = (InfoVector[i]->InterLockingFace[18][1] + InfoVector[i]->InterLockingFace[19][1] + InfoVector[i]->InterLockingFace[20][1] + InfoVector[i]->InterLockingFace[23][1]) / 4;
				Center[2] = (InfoVector[i]->InterLockingFace[18][2] + InfoVector[i]->InterLockingFace[19][2] + InfoVector[i]->InterLockingFace[20][2] + InfoVector[i]->InterLockingFace[23][2]) / 4;

				TempVertex[0][0] = (Center[0] + InfoVector[i]->InterLockingFace[18][0]) / 2 - (InfoVector[i]->InterLockType[3] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[18][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[0][1] = Center[1];
				TempVertex[0][2] = (Center[2] + InfoVector[i]->InterLockingFace[18][2]) / 2 - (InfoVector[i]->InterLockType[3] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[18][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[1][0] = (Center[0] + InfoVector[i]->InterLockingFace[19][0]) / 2 - (InfoVector[i]->InterLockType[3] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[19][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[1][1] = Center[1];
				TempVertex[1][2] = (Center[2] + InfoVector[i]->InterLockingFace[19][2]) / 2 - (InfoVector[i]->InterLockType[3] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[19][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[2][0] = (Center[0] + InfoVector[i]->InterLockingFace[20][0]) / 2 - (InfoVector[i]->InterLockType[3] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[20][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[2][1] = Center[1];
				TempVertex[2][2] = (Center[2] + InfoVector[i]->InterLockingFace[20][2]) / 2 - (InfoVector[i]->InterLockType[3] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[20][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[3][0] = (Center[0] + InfoVector[i]->InterLockingFace[23][0]) / 2 - (InfoVector[i]->InterLockType[3] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[23][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[3][1] = Center[1];
				TempVertex[3][2] = (Center[2] + InfoVector[i]->InterLockingFace[23][2]) / 2 - (InfoVector[i]->InterLockType[3] == LockType::Convex ? qBound(-1.0f, InfoVector[i]->InterLockingFace[23][2] - Center[2], 1.0f) : 0) * ConvexGap;

				#pragma region 產生有洞的面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[18]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[19]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[18]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[20]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[19]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[20]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[18]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[18]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[23]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[20]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[23]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[20]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion 
				#pragma region 產生向下的卡榫
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion
				#pragma region 產生最後的面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (InfoVector[i]->InterLockType[3] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion
				#pragma endregion
				#pragma region 沒有卡榫的兩個面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[19]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[4]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[5]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[19]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[5]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[18]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[20]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[12]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[13]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[23]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[12]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(InfoVector[i]->InterLockingFace[20]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma endregion
			}
		}
		else if (InfoVector[i]->PartName.contains("Triangle"))
		{
			OtherPoint.clear();
			OtherType.clear();

			const float TriagnleScaleZ = 30.0f / 32;
			for (MyMesh::FaceIter f_it = Model[PartNumber]->faces_begin(); f_it != Model[PartNumber]->faces_end(); ++f_it)
			{
				PointStack.clear();
				for (MyMesh::FaceVertexIter fv_it = Model[PartNumber]->fv_iter(f_it); fv_it.is_valid(); ++fv_it)
					PointStack.push_back(Model[PartNumber]->point(fv_it));

				// Check Y 是不是平的
				if (PointStack[0][1] == PointStack[1][1] && PointStack[1][1] == PointStack[2][1])
				{
					for (int j = 0; j < InfoVector[i]->InterLockingFace.length(); j += 3)
						if (InfoVector[i]->InterLockingFace[j][0] == PointStack[0][0] && InfoVector[i]->InterLockingFace[j][1] == PointStack[0][1] && InfoVector[i]->InterLockingFace[j][2] == PointStack[0][2] &&
							InfoVector[i]->InterLockingFace[j + 1][0] == PointStack[1][0] && InfoVector[i]->InterLockingFace[j + 1][1] == PointStack[1][1] && InfoVector[i]->InterLockingFace[j + 1][2] == PointStack[1][2] &&
							InfoVector[i]->InterLockingFace[j + 2][0] == PointStack[2][0] && InfoVector[i]->InterLockingFace[j + 2][1] == PointStack[2][1] && InfoVector[i]->InterLockingFace[j + 2][2] == PointStack[2][2] &&
							InfoVector[i]->InterLockType[j / 3] == LockType::Convex)
						{
							AddPointStack.push_back(PointStack);
							AddInterLockingType.push_back(InfoVector[i]->InterLockType[j / 3]);
							Model[PartNumber]->delete_face(f_it, false);
							break;
						}
				}
				else			// 不是水平的
					for (int j = 0; j < InfoVector[i]->InterLockingFace.length(); j += 3)
						if (InfoVector[i]->InterLockingFace[j][0] == PointStack[0][0] && InfoVector[i]->InterLockingFace[j][1] == PointStack[0][1] && InfoVector[i]->InterLockingFace[j][2] == PointStack[0][2] &&
							InfoVector[i]->InterLockingFace[j + 1][0] == PointStack[1][0] && InfoVector[i]->InterLockingFace[j + 1][1] == PointStack[1][1] && InfoVector[i]->InterLockingFace[j + 1][2] == PointStack[1][2] &&
							InfoVector[i]->InterLockingFace[j + 2][0] == PointStack[2][0] && InfoVector[i]->InterLockingFace[j + 2][1] == PointStack[2][1] && InfoVector[i]->InterLockingFace[j + 2][2] == PointStack[2][2])
						{
							OtherPoint.push_back(PointStack);
							OtherType.push_back(InfoVector[i]->InterLockType[j / 3]);
							Model[PartNumber]->delete_face(f_it, false);
						}


			}
			Model[PartNumber]->garbage_collection();

			// 開始加卡榫
			for (int j = 0; j < AddPointStack.length(); j += 2)
			{
				Center[0] = (AddPointStack[j][0][0] + AddPointStack[j][1][0] + AddPointStack[j][2][0] + AddPointStack[j + 1][2][0]) / 4;
				Center[1] = AddPointStack[j][0][1];
				Center[2] = (AddPointStack[j][0][2] + AddPointStack[j][1][2] + AddPointStack[j][2][2] + AddPointStack[j + 1][2][2]) / 4 + 2;

				TempVertex = new MyMesh::Point[4];
				TempVertex[0][0] = (Center[0] + AddPointStack[j][0][0]) / 2 - (AddInterLockingType[j] == LockType::Convex ? qBound(-1.0f, AddPointStack[j][0][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[0][1] = Center[1];
				TempVertex[0][2] = (Center[2] + AddPointStack[j][0][2]) / 2 * TriagnleScaleZ - (AddInterLockingType[j] == LockType::Convex ? qBound(-1.0f, AddPointStack[j][0][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[1][0] = (Center[0] + AddPointStack[j][1][0]) / 2  - (AddInterLockingType[j] == LockType::Convex ? qBound(-1.0f, AddPointStack[j][1][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[1][1] = Center[1];
				TempVertex[1][2] = (Center[2] + AddPointStack[j][1][2]) / 2 * TriagnleScaleZ - (AddInterLockingType[j] == LockType::Convex ? qBound(-1.0f, AddPointStack[j][1][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[2][0] = (Center[0] + AddPointStack[j][2][0]) / 2 - (AddInterLockingType[j] == LockType::Convex ? qBound(-1.0f, AddPointStack[j][2][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[2][1] = Center[1];
				TempVertex[2][2] = (Center[2] + AddPointStack[j][2][2]) / 2 * TriagnleScaleZ - (AddInterLockingType[j] == LockType::Convex ? qBound(-1.0f, AddPointStack[j][2][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[3][0] = (Center[0] + AddPointStack[j + 1][2][0]) / 2 - (AddInterLockingType[j] == LockType::Convex ? qBound(-1.0f, AddPointStack[j + 1][2][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[3][1] = Center[1];
				TempVertex[3][2] = (Center[2] + AddPointStack[j + 1][2][2]) / 2 * TriagnleScaleZ - (AddInterLockingType[j] == LockType::Convex ? qBound(-1.0f, AddPointStack[j + 1][2][2] - Center[2], 1.0f) : 0) * ConvexGap;

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

			// 開始處理斜的面
			for (int j = 0; j < OtherPoint.length(); j += 2)
			{
				Center[0] = (OtherPoint[j][0][0] + OtherPoint[j][1][0] + OtherPoint[j][2][0] + OtherPoint[j + 1][2][0]) / 4;
				Center[1] = (OtherPoint[j][0][1] + OtherPoint[j][1][1] + OtherPoint[j][2][1] + OtherPoint[j + 1][2][1]) / 4;
				Center[2] = (OtherPoint[j][0][2] + OtherPoint[j][1][2] + OtherPoint[j][2][2] + OtherPoint[j + 1][2][2]) / 4;

				TempVertex = new MyMesh::Point[4];
				TempVertex[0][0] = (Center[0] + OtherPoint[j][0][0]) / 2 -(OtherType[j] == LockType::Convex ? qBound(-1.0f, OtherPoint[j][0][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[0][1] = (Center[1] + OtherPoint[j][0][1]) / 2;
				TempVertex[0][2] = (Center[2] + OtherPoint[j][0][2]) / 2 *TriagnleScaleZ - (OtherType[j] == LockType::Convex ? qBound(-1.0f, OtherPoint[j][0][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[1][0] = (Center[0] + OtherPoint[j][1][0]) / 2 -(OtherType[j] == LockType::Convex ? qBound(-1.0f, OtherPoint[j][1][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[1][1] = (Center[1] + OtherPoint[j][1][1]) / 2;
				TempVertex[1][2] = (Center[2] + OtherPoint[j][1][2]) / 2 *TriagnleScaleZ - (OtherType[j] == LockType::Convex ? qBound(-1.0f, OtherPoint[j][1][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[2][0] = (Center[0] + OtherPoint[j][2][0]) / 2 -(OtherType[j] == LockType::Convex ? qBound(-1.0f, OtherPoint[j][2][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[2][1] = (Center[1] + OtherPoint[j][2][1]) / 2;
				TempVertex[2][2] = (Center[2] + OtherPoint[j][2][2]) / 2 *TriagnleScaleZ - (OtherType[j] == LockType::Convex ? qBound(-1.0f, OtherPoint[j][2][2] - Center[2], 1.0f) : 0) * ConvexGap;

				TempVertex[3][0] = (Center[0] + OtherPoint[j + 1][2][0]) / 2 -(OtherType[j] == LockType::Convex ? qBound(-1.0f, OtherPoint[j + 1][2][0] - Center[0], 1.0f) : 0) * ConvexGap;
				TempVertex[3][1] = (Center[1] + OtherPoint[j + 1][2][1]) / 2;
				TempVertex[3][2] = (Center[2] + OtherPoint[j + 1][2][2]) / 2 *TriagnleScaleZ - (OtherType[j] == LockType::Convex ? qBound(-1.0f, OtherPoint[j + 1][2][2] - Center[2], 1.0f) : 0) * ConvexGap;
			
				#pragma region 產生有洞的面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(OtherPoint[j][0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(OtherPoint[j][0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(OtherPoint[j][1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(OtherPoint[j][1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(OtherPoint[j][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(OtherPoint[j][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(OtherPoint[j][0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(OtherPoint[j][0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(OtherPoint[j + 1][2]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(OtherPoint[j][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(OtherPoint[j][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(OtherPoint[j + 1][2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
#pragma endregion
				#pragma region 產生向下的卡榫
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[1]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[2]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[0]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(TempVertex[3]));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
#pragma endregion
				#pragma	region 產生最後的面
				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[1][0], TempVertex[1][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[1][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());

				AddHandleStack.clear();
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[0][0], TempVertex[0][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[0][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[2][0], TempVertex[2][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[2][2])));
				AddHandleStack.push_back(Model[PartNumber]->add_vertex(MyMesh::Point(TempVertex[3][0], TempVertex[3][1] - InterLocking_Height - (OtherType[j] == LockType::Concave ? ConcaveHeightGap : 0), TempVertex[3][2])));
				Model[PartNumber]->add_face(AddHandleStack.toStdVector());
				#pragma	endregion
			}
		}
		else if (InfoVector[i]->PartName.contains("roof"))
		//delete TempVertex;
		qDebug() << "Model_part" << PartNumber << " Need To Add " << AddPointStack.length() + OtherPoint.length() << " triangles !!";
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
		//system("PAUSE");
	}
	else
		qDebug() << "Please add file path !!";
	return 0;
}