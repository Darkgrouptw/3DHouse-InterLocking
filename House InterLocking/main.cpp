#include <iostream>

#include "InterLockClass.h"

using namespace std;

int main(int argc, char *argv[])
{
	if (argc == 2)
	{
		InterLockClass lock(argv[1]);
	}
	else
	{
		cout << "�Ѽƿ��~!! " << endl;
		cout << "�w�̷ӡG <xxx.exe> <info.txt>" << endl;
	}
	system("PAUSE");
	return 0;
}
