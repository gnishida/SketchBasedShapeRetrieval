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
#include "ShapeMatching.h"

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

	renderingMode = RENDERING_MODE_REGULAR;
	depthSensitivity = 6000.0f;
	normalSensitivity = 1.0f;
	useThreshold = true;
	threshold = 0.25f;

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

	if (renderingMode == RENDERING_MODE_REGULAR) {
		glUniform1i(glGetUniformLocation(renderManager.program, "lineRendering"), 0);
	} else {
		glUniform1i(glGetUniformLocation(renderManager.program, "lineRendering"), 1);
		glUniform1f(glGetUniformLocation(renderManager.program, "depthSensitivity"), depthSensitivity);
		glUniform1f(glGetUniformLocation(renderManager.program, "normalSensitivity"), normalSensitivity);
		glUniform1i(glGetUniformLocation(renderManager.program, "useThreshold"), useThreshold ? 1 : 0);
		glUniform1f(glGetUniformLocation(renderManager.program, "threshold"), threshold);
	}
	
	renderManager.renderAll();
}

void GLWidget3D::loadObject(const std::string& filename) {
	renderManager.removeObjects();

	std::vector<Vertex> vertices;
	QFileInfo info(filename.c_str());
	if (info.suffix() == "obj") {
		OBJLoader::load(filename, vertices);
		renderManager.addObject("shape", "", vertices);
	} else {
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
	}

	renderManager.centerObjects();	
}

void GLWidget3D::galifTest() {
	ShapeMatching shapeMatching(this, width(), height(), 30, 4.0f, 10.0f, width() * 0.05f, height() * 0.05f, true);

	// 現在のレンダリングモードをバックアップ
	int currentRenderingMode = renderingMode;
	renderingMode = RENDERING_MODE_LINE;

	shapeMatching.learn("d:\\dataset\\windows\\", 1, 1);
	float accuracy = shapeMatching.test("d:\\dataset\\window_sketch\\", 1);

	std::cout << "Accuracy: " << accuracy << std::endl;

	// 元のレンダリングモードに戻す
	renderingMode = currentRenderingMode;
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

	loadObject("psb_test/m0.off");

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

	BagOfFeature bof(image, "psb_test/m0.off", camera, 4.0f, 10.0f, width() * 0.2f, height() * 0.2f);
}

void GLWidget3D::parameterOptimization() {
	// 現在のレンダリングモードをバックアップ
	int currentRenderingMode = renderingMode;
	renderingMode = RENDERING_MODE_LINE;

	int numCentroids[] = {10, 20, 30, 40, 50, 60};
	float feature_size[] = {0.1, 0.2, 0.3, 0.4};
	bool useTfidf[] = {true, false};

	float best_accuracy = 0.0f;
	int best_numCentroids;
	float best_feature_size;
	bool best_useTfidf;
	for (int i = 0; i < 6; ++i) {
		for (int j = 0; j < 4; ++j) {
			for (int k = 0; k < 2; ++k) {
				ShapeMatching shapeMatching(this, width(), height(), numCentroids[i], 4.0f, 10.0f, width() * feature_size[j], height() * feature_size[j], useTfidf[k]);
			
				shapeMatching.learn("d:\\dataset\\windows\\", 1, 1);
				float accuracy = shapeMatching.test("d:\\dataset\\window_sketch\\", 1);
				std::cout << "#centroids: " << numCentroids[i] << ", Feature size: " << feature_size[j] << ", UseTfidf: " << useTfidf[k] << ", Accuracy: " << accuracy << std::endl;
				if (accuracy > best_accuracy) {
					best_accuracy = accuracy;
					best_numCentroids = numCentroids[i];
					best_feature_size = feature_size[j];
					best_useTfidf = useTfidf[k];
				}
			}
		}
	}

	std::cout << "======================================" << std::endl;
	std::cout << "Best accuracy: " << best_accuracy << std::endl;
	std::cout << "Best #centroids: " << best_numCentroids << std::endl;
	std::cout << "Best feature_size: " << best_feature_size << std::endl;
	std::cout << "Best useTfidf: " << best_useTfidf << std::endl;

	// 元のレンダリングモードに戻す
	renderingMode = currentRenderingMode;

}