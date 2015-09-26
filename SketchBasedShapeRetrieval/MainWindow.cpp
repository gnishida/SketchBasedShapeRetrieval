#include "MainWindow.h"
#include <QFileDialog>
#include <QDate>
#include <time.h>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	// メニューハンドラ
	connect(ui.actionOpenOFF, SIGNAL(triggered()), this, SLOT(onOpenOFF()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionGALIFTest, SIGNAL(triggered()), this, SLOT(onGALIFTest()));
	connect(ui.actionGaborFilterTest, SIGNAL(triggered()), this, SLOT(onGaborFilterTest()));
	connect(ui.actionCollectOFFFiles, SIGNAL(triggered()), this, SLOT(onCollectOFFFiles()));

	glWidget = new GLWidget3D(this);
	setCentralWidget(glWidget);
}


void MainWindow::onOpenOFF() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open OFF file..."), "", tr("OFF Files (*.off)"));
	if (filename.isEmpty()) return;

	QString title = "Sketch Based Shape Retrieval - " + filename;
	this->setWindowTitle(title);
	glWidget->loadOFF(filename.toUtf8().data());
	glWidget->updateGL();
}

void MainWindow::onGALIFTest() {
	glWidget->galifTest();
}

void MainWindow::onGaborFilterTest() {
	glWidget->gaborFilterTest();
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
