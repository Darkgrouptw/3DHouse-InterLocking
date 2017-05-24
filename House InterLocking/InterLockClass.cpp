#include "InterLockClass.h"

InterLockClass::InterLockClass(char *inputFile)
{
	#pragma region 讀檔頭
	cout << "========== 讀取檔案 ==========" << endl;
	QString tempStr = QString(inputFile);
	QFile file(tempStr);
	if (!file.open(QIODevice::ReadOnly))
	{
		cout << "無法打開檔案 !!" << endl;
		return;
	}
	QTextStream ss(&file);
	int tempInt, totalNumber;
	float tempFloat, tempX, tempY, tempZ;
	int tempType;

	QString InputFileStr(inputFile);
	QStringList FilePathList = InputFileStr.split("/");
	FilePathLocation = "";
	for (int i = 0; i < FilePathList.length() - 1; i++)
		FilePathLocation += FilePathList[i] + "/";

	ss >> totalNumber;
	cout << "總共個數 => " << totalNumber << endl;
	#pragma endregion
	#pragma region 讀裡面資料
	for (int i = 0; i < totalNumber; i++)
	{
		MyMesh *tempMesh = new MyMesh;
		ModelInfo *info = new ModelInfo();
		
		// 讀檔案名稱
		ss >> tempStr;
		OpenMesh::IO::read_mesh(*tempMesh, (FilePathLocation + tempStr + ".obj").toStdString().data());
		tempStr.replace("model_part", "");
		info->PartNumber = tempStr.toInt();
		cout << "Part 數 => " << info->PartNumber << endl;
		cout << "點數 => " << tempMesh->n_vertices() << endl;

		tempMesh->request_vertex_status();
		tempMesh->request_edge_status();
		tempMesh->request_face_status();
		ModelsArray.push_back(tempMesh);

		ss >> info->PartName;
		ss >> tempInt;
		
		// 裡面為卡忖的資訊，暫時先不會用到
		for (int j = 0; j < tempInt / 2; j++)
			for (int k = 0; k < 6; k++)
			{
				ss >> tempX >> tempY >> tempZ >> tempType;
				//qDebug() << tempX << tempY << tempZ << tempType;
				/*switch (k)
				{
				case 0:
				case 3:
					info->InterLockType.push_back(tempType);
					info->InterLockingFace.push_back(MyMesh::Point(tempX, tempY, tempZ));
					break;
				default:
					info->InterLockingFace.push_back(MyMesh::Point(tempX, tempY, tempZ));
					break;
				}*/
			}

		//qDebug() << "InterLockingFace length => " << info->InterLockingFace.length();
		InfoArray.push_back(info);
	}
	file.close();
	cout << "========== 讀取完成 ==========" << endl;
	#pragma endregion
}
InterLockClass::~InterLockClass()
{
	// 清空記憶體
	for (int i = 0; i < ModelsArray.length(); i++)
		delete ModelsArray[i];
	for (int i = 0; i < InfoArray.length(); i++)
		delete InfoArray[i];
}

void InterLockClass::SplitInSmallSize()
{
	cout << "========== 開始切成小物件 ==========" << endl;
	int SplitCount = 0;
	for (int i = 0; i < InfoArray.length(); i++)
	{
		QVector<MyMesh::Point> PointArray;
		if (InfoArray[i]->PartName.contains("Windows"))
		{
			cout << "還沒做" << endl;
		}
		else if (InfoArray[i]->PartName == "base"/* || InfoArray[i]->PartName.contains("Wall")*/)
		{
			#pragma region 	排除錯誤狀況
			if (ModelsArray[i]->n_vertices() != 24)
			{
				cout << "Base 或 Wall 有錯誤" << endl;
				return;
			}
			#pragma endregion
			#pragma region 抓出最外圍的八個點
			MyMesh::VertexIter v_it = ModelsArray[i]->vertices_begin();
			QVector<MyMesh::Point> PointArray;

			int offsetArray[] = { 0, 1, 2,3,23,22,21,20 };
			for (int j = 0; j < 8; j++)
			{
				MyMesh::Point tempP = ModelsArray[i]->point(v_it + offsetArray[j]);
				PointArray.push_back(tempP);
			}
			#pragma endregion
			#pragma region 開始要加物件
			QVector<MyMesh::VertexHandle>	vhandle;						// Vertex Handle
			
			// 0 的部分，算要往哪裡跑
			float dx = (PointArray[1][0] - PointArray[0][0] > 0) ? 1 : -1;
			float dy = (PointArray[3][1] - PointArray[0][1] > 0) ? 1 : -1;
			float dz = (PointArray[4][2] - PointArray[0][2] > 0) ? 1 : -1;

			// 從這個開始
			float CurrentX;
			float CurrentY;
			float CurrentZ;

			// 下一個的 X, Y, Z
			float NextX;
			float NextY;
			float NextZ;

			// 開始條件
			float StartX = PointArray[0][0];
			float StartY = PointArray[0][1];
			float StartZ = PointArray[0][2];
			
			// 結束條件
			float EndX = PointArray[1][0];
			float EndY = PointArray[3][1];
			float EndZ = PointArray[4][2];
			
			//  X 軸有幾個、Y 軸有幾個、Z 軸有幾個
			int XCount = (abs(PointArray[1][0] - PointArray[0][0]) - 0.1f) / SplitSize + 2;					// 相減的部分 + 頭尾
			int YCount = (abs(PointArray[3][1] - PointArray[0][1]) - 0.1f) / SplitSize + 2;					// 相減的部分 + 頭尾
			int ZCount = (abs(PointArray[4][2] - PointArray[0][2]) - 0.1f) / SplitSize + 2;					// 相減的部分 + 頭尾

			// Bool 是否有拉到最後面
			bool IsEndX = false, IsEndY = false, IsEndZ = false;

			cout << "dx => " << dx << endl;
			cout << "dy => " << dy << endl;
			cout << "dz => " << dz << endl;

			cout << "X Count => " << XCount << endl;
			cout << "Y Count => " << YCount << endl;
			cout << "Z Count => " << ZCount << endl;
			
			// 新增 info
			SplitModelInfo* splitInfo = new SplitModelInfo();
			splitInfo->OrgModelIndex = i;
			splitInfo->StartModelIndex = SplitCount;
			splitInfo->PartNumber = InfoArray[i]->PartNumber;
			splitInfo->PartName = InfoArray[i]->PartName;

			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);
				CurrentY = StartY;
				while (CurrentY * dy < EndY * dy)
				{
					NextY = GetNextValue(CurrentY, dy, EndY);
					CurrentX = StartX;
					while (CurrentX * dx < EndX * dx)
					{
						NextX = GetNextValue(CurrentX, dx, EndX);
						#pragma region 初始化
						MyMesh *tempMesh = new MyMesh();
						SplitCount++;
						#pragma endregion
						#pragma region 將點加上去
						#pragma region 上
						vhandle.clear();
						if (dx == dz)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, NextZ)));
						}
						tempMesh->add_face(vhandle.toStdVector());
#pragma endregion
						#pragma region 下
						vhandle.clear();
						if (dx == dz)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, CurrentZ)));
						}
						tempMesh->add_face(vhandle.toStdVector());
						#pragma endregion
						#pragma region 左
						vhandle.clear();
						if (dy == dz)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, NextZ)));
						}
						tempMesh->add_face(vhandle.toStdVector());
						#pragma endregion
						#pragma region 右
						vhandle.clear();
						if (dy == dz)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, CurrentZ)));
						}
						tempMesh->add_face(vhandle.toStdVector());
						#pragma endregion
						#pragma region 前
						vhandle.clear();
						if (dx == dy)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
						}
						tempMesh->add_face(vhandle.toStdVector());
#pragma endregion
						#pragma region 後
						vhandle.clear();
						if (dx == dy)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
						}
						tempMesh->add_face(vhandle.toStdVector());
						#pragma endregion
						SplitModelsArray.push_back(tempMesh);

						tempMesh->update_normals();
						#pragma endregion
						CurrentX = NextX;
					}
					CurrentY = NextY;
				}
				CurrentZ = NextZ;
			}
			splitInfo->SplitCount = SplitCount;
			SplitInfoArray.push_back(splitInfo);
			#pragma endregion
		}
	}

	for (int i = 0; i < SplitInfoArray.length(); i++)
	{
		cout << "Name\t\t=> " << SplitInfoArray[i]->PartName.toStdString() << endl;
		cout << "Start Index\t=> " << SplitInfoArray[i]->StartModelIndex << endl;
		cout << "Part Number\t=> " << SplitInfoArray[i]->PartNumber << endl;
		cout << "Part Size\t=> " << SplitInfoArray[i]->SplitCount << endl;
	}
	cout << "========== 切小物件完成 ==========" << endl;
}

void InterLockClass::SaveAllModel()
{
	cout << "========== 開始儲存 ==========" << endl;
	if (!QDir(FilePathLocation + "/Output/").exists())
		QDir().mkdir(FilePathLocation + "/Output/");

	for (int i = 0; i < SplitInfoArray.length(); i++)
	{
		// 新增 Part 的資料夾
		QString infoPartName = SplitInfoArray[i]->PartName;
		if (!QDir(FilePathLocation + "/Output/" + infoPartName).exists())
			QDir().mkdir(FilePathLocation + "/Output/" + infoPartName);

		for (int j = 0; j < SplitInfoArray[i]->SplitCount; j++)
		{
			// 把 obj 存近來
			if (!OpenMesh::IO::write_mesh(*(ModelsArray[i]), QString(FilePathLocation + "Output/" + infoPartName + "/model_part" + QString::number(InfoArray[i]->PartNumber, 10) + "_" + QString::number(j +1) + outputFileEnd).toStdString().data()))
				cout << "儲存失敗 => model_part" << InfoArray[i]->PartNumber << "_" << (j + 1) << outputFileEnd.toStdString() << endl;
			else
				cout << "儲存成功 => model_part" << InfoArray[i]->PartNumber << "_" << (j + 1) << outputFileEnd.toStdString() << endl;
		}
	}
	cout << "========== 儲存完成 ==========" << endl;
}

float InterLockClass::GetNextValue(float currentValue, float d, float max)
{
	float nextValue = currentValue + SplitSize * d;					// 下一個值

	if (nextValue * d >= max * d && currentValue * d < max * d)
		nextValue = max;
	return nextValue;
}
