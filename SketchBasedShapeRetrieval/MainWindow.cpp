#include "MainWindow.h"
#include <QFileDialog>
#include <QDate>
#include <time.h>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	QActionGroup* groupRendering = new QActionGroup(this);
	ui.actionRenderingRegular->setCheckable(true);
	ui.actionRenderingLine->setCheckable(true);
	ui.actionRenderingRegular->setActionGroup(groupRendering);
	ui.actionRenderingLine->setActionGroup(groupRendering);
	ui.actionRenderingRegular->setChecked(true);

	// メニューハンドラ
	connect(ui.actionOpenOFF, SIGNAL(triggered()), this, SLOT(onOpenOFF()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionGALIFTest, SIGNAL(triggered()), this, SLOT(onGALIFTest()));
	connect(ui.actionGaborFilterTest, SIGNAL(triggered()), this, SLOT(onGaborFilterTest()));
	connect(ui.actionParameterOptimization, SIGNAL(triggered()), this, SLOT(onParameterOptimization()));
	connect(ui.actionCollectOFFFiles, SIGNAL(triggered()), this, SLOT(onCollectOFFFiles()));
	connect(ui.actionRenderingRegular, SIGNAL(triggered()), this, SLOT(onChangeRendering()));
	connect(ui.actionRenderingLine, SIGNAL(triggered()), this, SLOT(onChangeRendering()));

	glWidget = new GLWidget3D(this);
	setCentralWidget(glWidget);

	controlWidget = new RenderingControlWidget(this);
	//controlWidget->show();
	addDockWidget(Qt::LeftDockWidgetArea, controlWidget);
}


void MainWindow::onOpenOFF() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open file..."), "", tr("Object Files (*.obj *.off)"));
	if (filename.isEmpty()) return;

	QString title = "Sketch Based Shape Retrieval - " + filename;
	this->setWindowTitle(title);
	glWidget->loadObject(filename.toUtf8().data());
	glWidget->updateGL();
}

void MainWindow::onGALIFTest() {
	glWidget->galifTest();
}

void MainWindow::onGaborFilterTest() {
	glWidget->gaborFilterTest();
}

void MainWindow::onParameterOptimization() {
	glWidget->parameterOptimization();
}

void MainWindow::onCollectOFFFiles() {
	collectOFFFiles("png");
}

void MainWindow::collectOFFFiles(const QString& dirname) {
	QDir dir(dirname);

	QStringList filters;
	filters << "*.png";
	QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::Files|QDir::AllDirs|QDir::NoDotAndDotDot);
	for (int i = 0; i < fileInfoList.size(); ++i) {
		if (fileInfoList[i].isDir()) {
			collectOFFFiles(fileInfoList[i].absoluteFilePath());
		} else {
			QFile file(fileInfoList[i].absoluteFilePath());
			file.copy("sketch/" + fileInfoList[i].fileName());
		}
	}
}

void MainWindow::onChangeRendering() {
	if (ui.actionRenderingRegular->isChecked()) {
		glWidget->renderingMode = GLWidget3D::RENDERING_MODE_REGULAR;
	} else {
		glWidget->renderingMode = GLWidget3D::RENDERING_MODE_LINE;
	}
	glWidget->updateGL();
}