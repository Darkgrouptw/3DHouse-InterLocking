#include "InterLockClass.h"

InterLockClass::InterLockClass(char *inputFile)
{
	#pragma region 讀檔頭
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

		cout << "讀取檔案 => " << (FilePathLocation + tempStr + ".obj").toStdString() << endl;

		tempStr.replace("model_part", "");
		break;
	}
	file.close();
	#pragma endregion
}


InterLockClass::~InterLockClass()
{
}
