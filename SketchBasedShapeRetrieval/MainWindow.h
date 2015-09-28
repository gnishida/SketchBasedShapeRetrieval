#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "glew.h"
#include <QtGui/QMainWindow>
#include "ui_MainWindow.h"
#include "GLWidget3D.h"
#include "RenderingControlWidget.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	Ui::MainWindowClass ui;
	GLWidget3D* glWidget;
	RenderingControlWidget* controlWidget;

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	void collectOFFFiles(const QString& dirname);

public slots:
	void onOpenOFF();
	void onGALIFTest();
	void onGaborFilterTest();
	void onParameterOptimization();
	void onCollectOFFFiles();
	void onChangeRendering();
};

#endif // MAINWINDOW_H
