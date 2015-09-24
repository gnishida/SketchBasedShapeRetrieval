#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "glew.h"
#include <QtGui/QMainWindow>
#include "ui_MainWindow.h"
#include "GLWidget3D.h"

class MainWindow : public QMainWindow {
	Q_OBJECT

private:
	Ui::MainWindowClass ui;
	GLWidget3D* glWidget;

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	void collectOFFFiles(const QString& dirname);

public slots:
	void onOpenOFF();
	void onGALIFTest();
	void onGaborFilterTest();
	void onCollectOFFFiles();
};

#endif // MAINWINDOW_H
