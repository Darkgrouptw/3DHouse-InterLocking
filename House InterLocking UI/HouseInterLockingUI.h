#pragma once
#include <iostream>

#include <QtWidgets/QMainWindow>
#include "ui_HouseInterLockingUI.h"

#include <QFile>
#include <QDir>
#include <QIODevice>
#include <QTextStream>
#include <QFileDialog>

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

using namespace std;

typedef OpenMesh::PolyMesh_ArrayKernelT<> MyMesh;

class HouseInterLockingUI : public QMainWindow
{
	Q_OBJECT

public:
	HouseInterLockingUI(QWidget *parent = Q_NULLPTR);

private:
	Ui::HouseInterLockingUIClass ui;

	void SetSliderFunction(bool, bool, bool, bool, bool, bool);

	int selectIndex = -1;
	bool IsChanging = false;

private slots:
	void ComboBoxChangeEvent(int);

	void SliderChangeEvent(int);
};
