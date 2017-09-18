#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_HouseInterLockingUI.h"

class HouseInterLockingUI : public QMainWindow
{
	Q_OBJECT

public:
	HouseInterLockingUI(QWidget *parent = Q_NULLPTR);

private:
	Ui::HouseInterLockingUIClass ui;
};
