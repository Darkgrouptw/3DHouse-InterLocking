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
	int lastSplitCount = 0;
	for (int i = 0; i < InfoArray.length(); i++)
	{
		QVector<MyMesh::Point> PointArray;
		QVector<MyMesh::VertexHandle> vhandle;								// Vertex Handle

		if (InfoArray[i]->PartName.contains("Windows"))
		{
			cout << "還沒做" << endl;
		}
		else if (InfoArray[i]->PartName.contains("roof"))
		{
			#pragma region 	排除錯誤狀況
			if (ModelsArray[i]->n_vertices() != 20)
			{
				cout << "Triangle 有錯誤" << endl;
				return;
			}
			#pragma endregion
			#pragma region 加四個點 x 2
			MyMesh::VertexIter v_it = ModelsArray[i]->vertices_begin();

			int offsetArray[] = { 0,1,2,3,16,19 ,18, 17 };
			for (int j = 0; j < 8; j++)
			{
				MyMesh::Point tempP = ModelsArray[i]->point(v_it + offsetArray[j]);
				PointArray.push_back(tempP);
			}
			#pragma endregion
			#pragma region 開始要加物件
			QVector<MyMesh::VertexHandle>	vhandle;						// Vertex Handle
			
			// 新增 info
			SplitModelInfo* splitInfo = new SplitModelInfo();
			splitInfo->OrgModelIndex = i;
			splitInfo->StartModelIndex = SplitCount;
			splitInfo->PartNumber = InfoArray[i]->PartNumber;
			splitInfo->PartName = InfoArray[i]->PartName;

			// 0 的部分，算要往哪裡跑
			float dx = (PointArray[7][0] - PointArray[4][0] > 0) ? 1 : -1;
			float dz = (PointArray[5][2] - PointArray[4][2] > 0) ? 1 : -1;

			// 從這個開始
			float CurrentX;
			float CurrentZ;

			// 下一個的 X, Z
			float NextX;
			float NextZ;

			// 開始條件
			float StartX = PointArray[4][0];
			float StartZ = PointArray[4][2];

			// 結束條件
			float EndX = PointArray[7][0];
			float EndZ = PointArray[5][2];

			// 屋頂的 offset
			float offsetY = PointArray[0][1] - PointArray[4][1];

			CurrentX = StartX;
			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);

				CurrentX = StartX;
				while (CurrentX * dx < EndX * dx)
				{
					NextX = GetNextValue(CurrentX, dx, EndX);

					float Prograss = (CurrentZ - StartZ) / (EndZ - StartZ);
					float NextPrograss = (NextZ - StartZ) / (EndZ - StartZ);

					float CurrentY = (PointArray[5][1] - PointArray[4][1]) * Prograss + PointArray[4][1];
					float NextY = (PointArray[5][1] - PointArray[4][1]) * NextPrograss + PointArray[4][1];
					
					#pragma region 初始化
					MyMesh *tempMesh = new MyMesh;
					SplitCount++;
					#pragma endregion
					#pragma region 前面
					vhandle.clear();
					if (dz < 0) 
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region 後面
					vhandle.clear();

					if (dz < 0)
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						}
						else 
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						}
					}
					else
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
						}
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region 左側
					vhandle.clear();

					if (dz < 0)
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						}
					}
					else
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						}
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region 右側
					vhandle.clear();

					if (dz < 0)
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
						}
					}
					else
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
						}
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region 上
					vhandle.clear();

					if (dz < 0)
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
						}
					}
					else
					{
						if (NextPrograss != 1)
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY + offsetY, NextZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						}
						else
						{
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY + offsetY, CurrentZ)));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, PointArray[1][2])));
							vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY + offsetY, CurrentZ)));
						}
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion
					#pragma region 下
					vhandle.clear();

					if (dz < 0)
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
					}
					else
					{
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, CurrentY, CurrentZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(NextX, NextY, NextZ)));
						vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(CurrentX, NextY, NextZ)));
					}
					tempMesh->add_face(vhandle.toStdVector());
					#pragma endregion


					tempMesh->update_normals();
					SplitModelsArray.push_back(tempMesh);

					CurrentX = NextX;
				}

				CurrentZ = NextZ;
			}
			splitInfo->SplitCount = SplitCount - lastSplitCount;
			lastSplitCount = SplitCount;
			SplitInfoArray.push_back(splitInfo);
			#pragma endregion
		}
		else if (InfoArray[i]->PartName.contains(("Triangle")))
		{
			#pragma region 	排除錯誤狀況
			if (ModelsArray[i]->n_vertices() != 18)
			{
				cout << "Triangle 有錯誤" << endl;
				return;
			}
			#pragma endregion
			#pragma region 加三個點 x 2
			MyMesh::VertexIter v_it = ModelsArray[i]->vertices_begin();

			int offsetArray[] = { 0,1,2,3,5,4 };
			for (int j = 0; j < 6; j++)
			{
				MyMesh::Point tempP = ModelsArray[i]->point(v_it + offsetArray[j]);
				PointArray.push_back(tempP);
			}
			#pragma endregion
			#pragma region 開始要加物件
			QVector<MyMesh::VertexHandle>	vhandle;						// Vertex Handle
			
			// 加上 中心對應到底下的點
			MyMesh::Point tempP = (PointArray[0] + PointArray[2]) / 2;
			PointArray.push_back(tempP);
			tempP = (PointArray[3] + PointArray[5]) / 2;
			PointArray.push_back(tempP);

			// 新增 info
			SplitModelInfo* splitInfo = new SplitModelInfo();
			splitInfo->OrgModelIndex = i;
			splitInfo->StartModelIndex = SplitCount;
			splitInfo->PartNumber = InfoArray[i]->PartNumber;
			splitInfo->PartName = InfoArray[i]->PartName;

			#pragma region 前半段
			// 6 的部分，算要往哪裡跑
			float dz = (PointArray[0][2] - PointArray[6][2] > 0) ? 1 : -1;

			// 從這個開始
			float CurrentZ;

			// 下一個的 X, Z
			float NextZ;

			// 開始條件
			float StartZ = PointArray[6][2];

			// 結束條件
			float EndZ = PointArray[0][2];

			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);

				float Prograss = (CurrentZ - StartZ) / (EndZ - StartZ);
				float NextPrograss = (NextZ - StartZ) / (EndZ - StartZ);

				float CurrentY = (PointArray[0][1] - PointArray[1][1]) * Prograss + PointArray[1][1];
				float NextY = (PointArray[0][1] - PointArray[1][1]) * NextPrograss + PointArray[1][1];

				MyMesh *tempMesh = new MyMesh;
				#pragma region 前面
				if (Prograss < 1)
				{
					vhandle.clear();

					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], NextZ)));
					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], NextY, NextZ)));
					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], PointArray[0][1], NextZ)));
					tempMesh->add_face(vhandle.toStdVector());
				}
				#pragma endregion
				#pragma region 後面
				vhandle.clear();
				
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], PointArray[0][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], CurrentZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region 左側
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], PointArray[0][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], PointArray[0][1], NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region 右側
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], NextY, NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], NextZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region 上
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], NextY, NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], NextY, NextZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region 下
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], PointArray[0][1], NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[0][0], PointArray[0][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], NextZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion


				tempMesh->update_normals();
				SplitModelsArray.push_back(tempMesh);
				SplitCount++;

				CurrentZ = NextZ;
			}
			#pragma endregion
			#pragma region 後半段
			// 6 的部分，算要往哪裡跑
			dz = (PointArray[2][2] - PointArray[6][2] > 0) ? 1 : -1;

			// 從這個開始
			CurrentZ;

			// 下一個的 X, Z
			NextZ;

			// 開始條件
			StartZ = PointArray[6][2];

			// 結束條件
			EndZ = PointArray[2][2];

			CurrentZ = StartZ;
			while (CurrentZ * dz < EndZ * dz)
			{
				NextZ = GetNextValue(CurrentZ, dz, EndZ);

				float Prograss = (CurrentZ - StartZ) / (EndZ - StartZ);
				float NextPrograss = (NextZ - StartZ) / (EndZ - StartZ);

				float CurrentY = (PointArray[2][1] - PointArray[1][1]) * Prograss + PointArray[1][1];
				float NextY = (PointArray[2][1] - PointArray[1][1]) * NextPrograss + PointArray[1][1];

				MyMesh *tempMesh = new MyMesh;
				#pragma region 前面
				if (Prograss < 1)
				{
					vhandle.clear();

					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], PointArray[2][1], NextZ)));
					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], NextY, NextZ)));
					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], NextY, NextZ)));
					vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], NextZ)));
					tempMesh->add_face(vhandle.toStdVector());
				}
				#pragma endregion
				#pragma region 後面
				vhandle.clear();
				
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], PointArray[2][1], CurrentZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region 左側
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], NextY, NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], PointArray[2][1], NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], PointArray[2][1], CurrentZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region 右側
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], NextY, NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], CurrentZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region 上
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], NextY, NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], CurrentY, CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], NextY, NextZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion
				#pragma region 下
				vhandle.clear();

				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], NextZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[3][0], PointArray[3][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], PointArray[2][1], CurrentZ)));
				vhandle.push_back(tempMesh->add_vertex(MyMesh::Point(PointArray[2][0], PointArray[2][1], NextZ)));
				tempMesh->add_face(vhandle.toStdVector());
				#pragma endregion

				tempMesh->update_normals();
				SplitModelsArray.push_back(tempMesh);
				SplitCount++;

				CurrentZ = NextZ;
			}
			#pragma endregion
			splitInfo->SplitCount = SplitCount - lastSplitCount;
			lastSplitCount = SplitCount;

			SplitInfoArray.push_back(splitInfo);
			#pragma endregion
		}
		else if (InfoArray[i]->PartName == "base" || InfoArray[i]->PartName.contains("Wall"))
		{
			#pragma region 	排除錯誤狀況
			if (ModelsArray[i]->n_vertices() != 24)
			{
				cout << "Base 或 Wall 有錯誤" << endl;
				return;
			}
			#pragma endregion
			#pragma region 抓出最外圍的兩個點
			MyMesh::VertexIter v_it = ModelsArray[i]->vertices_begin();

			int *offsetArray = new int[2];
			if (InfoArray[i]->PartName.contains("Wall") && InfoArray[i]->PartName != "backWall")
			{
				offsetArray[0] = 21;
				offsetArray[1] = 0;
			}
			else
			{
				offsetArray[0] = 0;
				offsetArray[1] = 21;
			}

			for (int j = 0; j < 2; j++)
			{
				MyMesh::Point tempP = ModelsArray[i]->point(v_it + offsetArray[j]);
				PointArray.push_back(tempP);
			}
			#pragma endregion
			#pragma region 開始要加物件
			
			// 0 的部分，算要往哪裡跑
			float dx = (PointArray[1][0] - PointArray[0][0] > 0) ? 1 : -1;
			float dy = (PointArray[1][1] - PointArray[0][1] > 0) ? 1 : -1;
			float dz = (PointArray[1][2] - PointArray[0][2] > 0) ? 1 : -1;

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
			float EndY = PointArray[1][1];
			float EndZ = PointArray[1][2];

			// Bool 是否有拉到最後面
			bool IsEndX = false, IsEndY = false, IsEndZ = false;

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
						if (dx * dy * dz < 0)
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
						if (dx * dy * dz < 0)
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
						if (dx * dy * dz < 0)
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
						if (dx * dy * dz < 0)
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
						if (dx * dy * dz < 0)
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
						if (dx * dy * dz < 0)
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

						tempMesh->update_normals();
						SplitModelsArray.push_back(tempMesh);
						#pragma endregion
						CurrentX = NextX;
					}
					CurrentY = NextY;
				}
				CurrentZ = NextZ;
			}
			splitInfo->SplitCount = SplitCount - lastSplitCount;
			lastSplitCount = SplitCount;

			SplitInfoArray.push_back(splitInfo);
			#pragma endregion
			#pragma region 清空
			delete[] offsetArray;
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
			int StartIndex = SplitInfoArray[i]->StartModelIndex;

			// 把 obj 存進來
			if (!OpenMesh::IO::write_mesh(*SplitModelsArray[j + StartIndex], 
				QString(FilePathLocation + "Output/" + infoPartName + "/model_part" + QString::number(SplitInfoArray[i]->PartNumber, 10) + "_" + QString::number(j + 1) + outputFileEnd).toStdString().data()))
				cout << "儲存失敗 => model_part" << SplitInfoArray[i]->PartNumber << "_" << (j + 1) << outputFileEnd.toStdString() << endl;
			else
				cout << "儲存成功 => model_part" << SplitInfoArray[i]->PartNumber << "_" << (j + 1) << outputFileEnd.toStdString() << endl;
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