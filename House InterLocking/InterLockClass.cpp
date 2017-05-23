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
		InterLockingInfo *info = new InterLockingInfo();
		
		// 讀檔案名稱
		ss >> tempStr;
		OpenMesh::IO::read_mesh(*tempMesh, (FilePathLocation + tempStr + ".obj").toStdString().data());
		tempStr.replace("model_part", "");
		info->PartNumber = tempStr.toInt();
		cout << "Part 數 => " << info->PartNumber << endl;

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
	}
	file.close();
	cout << "========== 讀取完成 ==========" << endl;
	#pragma endregion
}


InterLockClass::~InterLockClass()
{
}
