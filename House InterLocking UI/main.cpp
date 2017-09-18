#include "HouseInterLockingUI.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	HouseInterLockingUI w;
	w.show();
	return a.exec();
}
