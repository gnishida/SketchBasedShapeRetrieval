#include "ShapeMatching.h"
#include <QDir>
#include "GLWidget3D.h"
#include "CVUtils.h"

ShapeMatching::ShapeMatching(GLWidget3D* glWidget3D, int width, int height, int vocabulary_size, float sigma, float lmbd, int patch_width, int patch_height, bool useTfidf) {
	this->glWidget3D = glWidget3D;
	this->width = width;
	this->height = height;
	this->vocabulary_size = vocabulary_size;
	this->sigma = sigma;
	this->lmbd = lmbd;
	this->patch_width = patch_width;
	this->patch_height = patch_height;
	this->useTfidf = useTfidf;
}


ShapeMatching::~ShapeMatching(void) {
}

void ShapeMatching::learn(const std::string& traindata_dir, int pitch_angle_num, int yaw_angle_num) {
	QDir dir(traindata_dir.c_str());
	QStringList filters;
	filters << "*.off" << "*.obj";
	QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot);

	// featuresの各要素は、各画像(つまり、各3Dモデルの各ビュー)の特徴量
	features.clear();
	for (int i = 0; i < fileInfoList.size(); ++i) {
		std::cout << "extracting features... " << fileInfoList[i].fileName().toUtf8().constData() << std::endl;
		//extractFeatures(fileInfoList[i].absoluteFilePath().toUtf8().constData(), 18, 36, features);
		extractFeatures(fileInfoList[i].absoluteFilePath().toUtf8().constData(), pitch_angle_num, yaw_angle_num, features);
	}

	// k-meansで、BoF特徴量のクラスタリングを行う
	kmeans(features, centroids, frequencies);

	std::cout << "==========================" << std::endl;
	std::cout << "#Views: " << features.size() << std::endl;
	std::cout << "#frequencies: " << frequencies.size() << std::endl;
	std::cout << "==========================" << std::endl;
	


	// 各3Dモデルの各ビューについて、BoF特徴量をヒストグラムとして計算する
	for (int i = 0; i < features.size(); ++i) {
		features[i].computeHistogram(centroids, frequencies, useTfidf);
	}
}

void ShapeMatching::extractFeatures(const std::string& filename, int pitch_angle_num, int yaw_angle_num, std::vector<BagOfFeature>& features) {
	glUniform1i(glGetUniformLocation(glWidget3D->renderManager.program, "depthComputation"), 0);

	glWidget3D->loadObject(filename);

	int pitch_angle_step = 90 / pitch_angle_num;
	int yaw_angle_step = 360 / yaw_angle_num;

	for (int pitch_angle = 0; pitch_angle < 90; pitch_angle += pitch_angle_step) {
		for (int yaw_angle = 0; yaw_angle < 360; yaw_angle += yaw_angle_step) {
			glWidget3D->camera.xrot = pitch_angle;
			glWidget3D->camera.yrot = yaw_angle;
			glWidget3D->camera.zrot = 0.0f;
			glWidget3D->camera.pos.z = 1.6f;

			glWidget3D->camera.updateMVPMatrix();

			cv::Mat image;
			glWidget3D->renderImage(image);

#if DEBUG
			QFileInfo finfo(QString(filename.c_str()));
			QString outname = "results/" + finfo.baseName() + QString("_%1_%2").arg(pitch_angle+90).arg(yaw_angle) + ".jpg";
			cv::imwrite(outname.toUtf8().constData(), image);
#endif

			image = 255 - image;

			BagOfFeature bof(image, filename, glWidget3D->camera, sigma, lmbd, patch_width, patch_height);
			features.push_back(bof);
		}
	}
}

float ShapeMatching::test(const std::string& testdata_dir, int topN) {
	QDir dir(testdata_dir.c_str());
	QStringList filters;
	filters << "*.jpg" << "*.png";
	QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot);
	int correctCount = 0;
	for (int i = 0; i < fileInfoList.size(); ++i) {
		std::cout << "sketch features... " << fileInfoList[i].fileName().toUtf8().constData() << std::endl;

		cv::Mat sketch = cv::imread(fileInfoList[i].absoluteFilePath().toUtf8().constData(), 0);
		cv::threshold(sketch, sketch, 128, 255, cv::THRESH_BINARY);
		sketch = 255 - sketch;
		sketch.convertTo(sketch, CV_32F, 1.0f, 0.0f);


		// スケッチサイズを画面サイズに合わせる
		cvutils::mat_resize(sketch, cv::Size(width, height), true);

		BagOfFeature bof(sketch, fileInfoList[i].absoluteFilePath().toUtf8().constData(), glWidget3D->camera, sigma, lmbd, patch_width, patch_height);
		bof.computeHistogram(centroids, frequencies, useTfidf);

		std::vector<int> results;
		bof.findSimilarModels(features, results, topN);

		for (int k = 0; k < results.size(); ++k) {
			glWidget3D->loadObject(features[results[k]].filepath);
			glWidget3D->camera = features[results[k]].camera;


			glWidget3D->camera.updateMVPMatrix();

			cv::Mat image;
			glWidget3D->renderImage(image);

			char filename[256];
			sprintf(filename, "results/result_%s_%d.jpg", fileInfoList[i].baseName().toUtf8().constData(), k);
			cv::imwrite(filename, image);
		}

		QFileInfo info(features[results[0]].filepath.c_str());
		if (info.baseName() ==fileInfoList[i].baseName()) {
			correctCount++;
		}
	}

	return (float)correctCount / fileInfoList.size();
}

void ShapeMatching::kmeans(const std::vector<BagOfFeature>& features, std::vector<cv::Mat>& centroids, std::vector<float>& frequencies) {
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
	cv::kmeans(concatenatedFeatures, vocabulary_size, labels, cv::TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10000, 0.0001), attempts, cv::KMEANS_PP_CENTERS, mu);

	centroids.clear();
	for (int r = 0; r < mu.rows; ++r) {
		cv::Mat centroid(mu.cols, 1, CV_32F);
		for (int c = 0; c < mu.cols; ++c) {
			cvutils::mat_set_value(centroid, c, 0, cvutils::mat_get_value(mu, r, c));
		}
		centroids.push_back(centroid);
	}

	// 各クラスタの頻度を計算する。
	// 頻度が少ないクラスタほど、重要度が大きい。ヒストグラム生成時に、この頻度を考慮する。
	frequencies.resize(centroids.size(), 0);
	for (int i = 0; i < labels.size(); ++i) {
		frequencies[labels[i]]++;
	}
	for (int i = 0; i < frequencies.size(); ++i) {
		frequencies[i] /= (float)labels.size();
	}
}