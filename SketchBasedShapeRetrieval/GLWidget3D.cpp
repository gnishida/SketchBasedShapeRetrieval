#include "GLWidget3D.h"
#include "MainWindow.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include "GLUtils.h"
#include "CVUtils.h"
#include "OBJLoader.h"
#include <QDir>
#include "TopNSearch.h"
#include "offstats.h"
#include <QFileInfo>

#define DEBUG	0

GLWidget3D::GLWidget3D(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers)) {
	// 光源位置をセット
	// ShadowMappingは平行光源を使っている。この位置から原点方向を平行光源の方向とする。
	light_dir = glm::normalize(glm::vec3(-4, -5, -8));

	// シャドウマップ用のmodel/view/projection行列を作成
	glm::mat4 light_pMatrix = glm::ortho<float>(-100, 100, -100, 100, 0.1, 200);
	glm::mat4 light_mvMatrix = glm::lookAt(-light_dir * 50.0f, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	light_mvpMatrix = light_pMatrix * light_mvMatrix;
}

/**
 * This event handler is called when the mouse press events occur.
 */
void GLWidget3D::mousePressEvent(QMouseEvent *e) {
	camera.mousePress(e->x(), e->y());
}

/**
 * This event handler is called when the mouse release events occur.
 */
void GLWidget3D::mouseReleaseEvent(QMouseEvent *e) {
}

/**
 * This event handler is called when the mouse move events occur.
 */
void GLWidget3D::mouseMoveEvent(QMouseEvent *e) {
	if (e->buttons() & Qt::LeftButton) { // Rotate
		camera.rotate(e->x(), e->y());
	} else if (e->buttons() & Qt::MidButton) { // Move
		camera.move(e->x(), e->y());
	} else if (e->buttons() & Qt::RightButton) { // Zoom
		camera.zoom(e->x(), e->y());
	}

	updateGL();
}

/**
 * This function is called once before the first call to paintGL() or resizeGL().
 */
void GLWidget3D::initializeGL() {
	renderManager.init("../shaders/vertex.glsl", "", "../shaders/fragment.glsl", false);
	rb.init(renderManager.program, 4, 5, width(), height());

	showWireframe = true;
	showScopeCoordinateSystem = false;

	// set the clear color for the screen
	//qglClearColor(QColor(113, 112, 117));
	qglClearColor(QColor(255, 255, 255));
}

/**
 * This function is called whenever the widget has been resized.
 */
void GLWidget3D::resizeGL(int width, int height) {
	height = height ? height : 1;
	glViewport(0, 0, width, height);
	camera.updatePMatrix(width, height);

	rb.update(width, height);
}

/**
 * This function is called whenever the widget needs to be painted.
 */
void GLWidget3D::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	// Model view projection行列をシェーダに渡す
	glUniformMatrix4fv(glGetUniformLocation(renderManager.program, "mvpMatrix"),  1, GL_FALSE, &camera.mvpMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(renderManager.program, "mvMatrix"),  1, GL_FALSE, &camera.mvMatrix[0][0]);

	// pass the light direction to the shader
	//glUniform1fv(glGetUniformLocation(renderManager.program, "lightDir"), 3, &light_dir[0]);
	glUniform3f(glGetUniformLocation(renderManager.program, "lightDir"), light_dir.x, light_dir.y, light_dir.z);
	
	rb.pass1();
	drawScene(0);
	rb.pass2();
	drawScene(0);
}

/**
 * Draw the scene.
 */
void GLWidget3D::drawScene(int drawMode) {
	if (drawMode == 0) {
		glUniform1i(glGetUniformLocation(renderManager.program, "depthComputation"), 0);
	} else {
		glUniform1i(glGetUniformLocation(renderManager.program, "depthComputation"), 1);
	}
	
	renderManager.renderAll();
}

void GLWidget3D::loadOFF(const std::string& filename) {
	renderManager.removeObjects();

	std::vector<Vertex> vertices;
	psb::Mesh* mesh = psb::ReadOffFile(filename.c_str());
	for (int i = 0; i < mesh->nfaces; i++) {
		psb::Face& f = mesh->faces[i];

		for (int k = 0; k < f.nverts; ++k) {
			vertices.push_back(Vertex(glm::vec3(f.verts[k]->x, f.verts[k]->y, f.verts[k]->z), glm::vec3(f.normal[0], f.normal[1], f.normal[2]), glm::vec4(1, 1, 1, 1)));
		}
	}
	renderManager.addObject("shape", "", vertices);

	delete [] mesh->faces;
	delete [] mesh->verts;
	delete mesh;

	renderManager.centerObjects();	
}

void GLWidget3D::galifTest() {
	QDir dir("d:\\dataset\\psb\\");
	QStringList filters;
	filters << "*.off";
	QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot);

	std::vector<BagOfFeature> features;
	for (int i = 0; i < fileInfoList.size(); ++i) {
		std::cout << "extracting features... " << fileInfoList[i].fileName().toUtf8().constData() << std::endl;
		//extractFeatures(fileInfoList[i].absoluteFilePath().toUtf8().constData(), 18, 36, features);
		extractFeatures(fileInfoList[i].absoluteFilePath().toUtf8().constData(), 6, 12, features);
	}

	// k-meansで、BoF特徴量のクラスタリングを行う
	std::vector<cv::Mat> centroids;
	kmeans(features, centroids);

	std::cout << "==========================" << std::endl;
	std::cout << "#Views: " << features.size() << std::endl;
	std::cout << "==========================" << std::endl;

	// 各3Dモデルの各ビューについて、BoF特徴量をヒストグラムとして計算する
	for (int i = 0; i < features.size(); ++i) {
		features[i].computeHistogram(centroids);
	}

	// test
	{
		QDir dir("d:\\dataset\\sketch\\");
		QStringList filters;
		filters << "*.jpg" << "*.png";
		QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot);
		for (int i = 0; i < fileInfoList.size(); ++i) {
			std::cout << "sketch features... " << fileInfoList[i].fileName().toUtf8().constData() << std::endl;

			cv::Mat sketch = cv::imread(fileInfoList[i].absoluteFilePath().toUtf8().constData(), 0);
			cv::threshold(sketch, sketch, 128, 255, cv::THRESH_BINARY);
			sketch = 255 - sketch;
			sketch.convertTo(sketch, CV_32F, 1.0f, 0.0f);


			// スケッチサイズを画面サイズに合わせる
			cvutils::mat_resize(sketch, cv::Size(width(), height()), true);

			BagOfFeature bof(sketch, fileInfoList[i].absoluteFilePath().toUtf8().constData(), camera, 2.5f, 10.0f);
			bof.computeHistogram(centroids);

			std::vector<int> results;
			bof.findSimilarModels(features, results, 1);

			for (int k = 0; k < results.size(); ++k) {
				loadOFF(features[results[k]].filepath);
				camera = features[results[k]].camera;


				camera.updateMVPMatrix();

				cv::Mat image;
				renderImage(image);

				char filename[256];
				sprintf(filename, "results/result_%s_%d.jpg", fileInfoList[i].baseName().toUtf8().constData(), k);
				cv::imwrite(filename, image);
			}
		}
	}
}

void GLWidget3D::extractFeatures(const std::string& filename, int pitch_angle_num, int yaw_angle_num, std::vector<BagOfFeature>& features) {
	glUniform1i(glGetUniformLocation(renderManager.program, "depthComputation"), 0);

	loadOFF(filename);

	int pitch_angle_step = 180 / pitch_angle_num;
	int yaw_angle_step = 360 / yaw_angle_num;

	for (int pitch_angle = -90; pitch_angle <= 90; pitch_angle += pitch_angle_step) {
		for (int yaw_angle = 0; yaw_angle < 360; yaw_angle += yaw_angle_step) {
			camera.xrot = pitch_angle;
			camera.yrot = yaw_angle;
			camera.zrot = 0.0f;
			camera.pos.z = 1.6f;

			camera.updateMVPMatrix();

			cv::Mat image;
			renderImage(image);

#if DEBUG
			QFileInfo finfo(QString(filename.c_str()));
			QString outname = "results/" + finfo.baseName() + QString("_%1_%2").arg(pitch_angle+90).arg(yaw_angle) + ".jpg";
			cv::imwrite(outname.toUtf8().constData(), image);
#endif

			image = 255 - image;

			BagOfFeature bof(image, filename, camera, 2.5f, 10.0f);
			features.push_back(bof);
		}
	}
}

void GLWidget3D::renderImage(cv::Mat& image) {
	glClearColor(1, 1, 1, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// Model view projection行列をシェーダに渡す
	glUniformMatrix4fv(glGetUniformLocation(renderManager.program, "mvpMatrix"),  1, GL_FALSE, &camera.mvpMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(renderManager.program, "mvMatrix"),  1, GL_FALSE, &camera.mvMatrix[0][0]);

	rb.pass1();
	drawScene(0);
	rb.pass2();
	drawScene(0);

	unsigned char* data = new unsigned char[sizeof(unsigned char) * 3 * width() * height()];
	glReadPixels(0, 0, width(), height(), GL_BGR, GL_UNSIGNED_BYTE, data);
	cv::Mat src_img(height(), width(), CV_8UC3, data);

	cv::cvtColor(src_img, src_img, CV_BGR2GRAY);
	cv::flip(src_img, src_img, 0);
	//cv::threshold(src_img, src_img, 230, 255, 0);

	cv::threshold(src_img, src_img, 128, 255, cv::THRESH_BINARY);

	src_img.convertTo(image, CV_32F, 1.0f, 0.0f);
				
	delete [] data;
}

void GLWidget3D::gaborFilterTest() {
	glUniform1i(glGetUniformLocation(renderManager.program, "depthComputation"), 0);

	loadOFF("psb_test/m0.off");

	int pitch_angle_step = 180;
	int yaw_angle_step = 360;

	camera.xrot = 0;
	camera.yrot = 0;
	camera.zrot = 0.0f;
	camera.pos.z = 1.6f;

	camera.updateMVPMatrix();

	cv::Mat image;
	renderImage(image);
	image = 255 - image;

	for (float sigma = 1.0f; sigma < 10.0f; sigma += 0.5f) {
		BagOfFeature bof(image, "psb_test/m0.off", camera, sigma, 10.0f);
	}
}

void GLWidget3D::kmeans(const std::vector<BagOfFeature>& features, std::vector<cv::Mat>& centroids) {
	int num = 0;
	for (int i = 0; i < features.size(); ++i) {
		num += features[i].features.size();
	}

	cv::Mat concatenatedFeatures(num, features[0].features[0].rows, CV_32F);
	int count = 0;
	for (int i = 0; i < features.size(); ++i) {
		for (int j = 0; j < features[i].features.size(); ++j) {
			for (int k = 0; k < features[i].features[j].rows; ++k) {
				cvutils::mat_set_value(concatenatedFeatures, count, k, cvutils::mat_get_value(features[i].features[j], k, 0));
			}
			count++;
		}
	}

	std::vector<int> labels;
	int attempts = 5;
	cv::Mat mu;
	//cv::kmeans(concatenatedFeatures, 100, labels, cv::TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10000, 0.0001), attempts, cv::KMEANS_PP_CENTERS, mu);
	cv::kmeans(concatenatedFeatures, 20, labels, cv::TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10000, 0.0001), attempts, cv::KMEANS_PP_CENTERS, mu);

	centroids.clear();
	for (int r = 0; r < mu.rows; ++r) {
		cv::Mat centroid(mu.cols, 1, CV_32F);
		for (int c = 0; c < mu.cols; ++c) {
			cvutils::mat_set_value(centroid, c, 0, cvutils::mat_get_value(mu, r, c));
		}
		centroids.push_back(centroid);
	}
}

