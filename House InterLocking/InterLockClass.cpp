#include "InterLockClass.h"

InterLockClass::InterLockClass(char *inputFile)
{
	#pragma region Ū���Y
	QString tempStr = QString(inputFile);
	QFile file(tempStr);
	if (!file.open(QIODevice::ReadOnly))
	{
		cout << "�L�k���}�ɮ� !!" << endl;
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
	cout << "�`�@�Ӽ� => " << totalNumber << endl;
	#pragma endregion
	#pragma region Ū�̭����
	for (int i = 0; i < totalNumber; i++)
	{
		MyMesh *mesh = new MyMesh;

		//Models
	}
	file.close();
	#pragma endregion
}


InterLockClass::~InterLockClass()
{
}
