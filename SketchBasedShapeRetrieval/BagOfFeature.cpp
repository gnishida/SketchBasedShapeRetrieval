#include "BagOfFeature.h"
#include "CVUtils.h"
#include "TopNSearch.h"

BagOfFeature::BagOfFeature(const cv::Mat& image, const std::string& filepath, const Camera& camera, float sigma, float lmbd) {
	this->filepath = filepath;
	this->camera = camera;
	extractFeatures(image, sigma, lmbd);
}

void BagOfFeature::computeHistogram(const std::vector<cv::Mat>& visualWords) {
	histogram = cv::Mat::zeros(visualWords.size(), 1, CV_32F);

	for (int i = 0; i < features.size(); ++i) {
		float max_score = 0.0f;
		int max_id = -1;

		for (int j = 0; j < visualWords.size(); ++j) {
			float score = cv::norm(features[i] - visualWords[j]);
			if (score > max_score) {
				max_score = score;
				max_id = j;
			}
		}

		cvutils::mat_add_value(histogram, max_id, 0, 1);
	}

	histogram /= features.size();
}

void BagOfFeature::extractFeatures(const cv::Mat& image, float sigma, float lmbd) {
	std::vector<cv::Mat> filteredImages;

	{
		for (int k = 0; k < 4; ++k) {
			float theta = (float)k * 45.0f / 180.0f * 3.14159265f;
			cv::Mat kernel = cv::getGaborKernel(cv::Size(21, 21), sigma, theta, lmbd, 0.5f, 0.0, CV_32F);
			kernel /= 1.0 * cv::sum(kernel)[0];

			cv::Mat dest;
			cv::filter2D(image, dest, CV_32F, kernel);

			cv::Mat dest2;
			cv::pow(dest, 2, dest2);

			/*
			char filename[256];
			sprintf(filename, "results/gabor_%.1lf_%.1lf_%.1lf.jpg", sigma, lmbd, theta);
			cvutils::mat_save(filename, dest2, true);
			*/

			filteredImages.push_back(dest2);
		}
	}


	float patch_w = filteredImages[0].cols / 32.0f;
	float patch_h = filteredImages[0].rows / 32.0f;
	float tile_w = ceil(patch_w / 4.0f);
	float tile_h = ceil(patch_h / 4.0f);
	
	for (int u = 0; u < 32; ++u) {
		for (int v = 0; v < 32; ++v) {
			cv::Mat f = cv::Mat::zeros(4 * 4 * filteredImages.size(), 1, CV_32F);

			{
				int x1 = u * patch_w;
				int y1 = v * patch_h;
				int x2 = std::min(x1 + (int)patch_w, image.cols);
				int y2 = std::min(y1 + (int)patch_h, image.rows);
	
				cv::Mat roi(image, cv::Rect(x1, y1, x2 - x1, y2 - y1));
				if (cv::sum(roi)[0] < 0.001f) continue;
			}

			for (int k = 0; k < filteredImages.size(); ++k) {
				for (int s = 0; s < 4; ++s) {
					for (int t = 0; t < 4; ++t) {
						float value_sum = 0.0f;
						int count = 0;

						for (int x = 0; x < tile_w; ++x) {
							for (int y = 0; y < tile_h; ++y) {
								int xx = u * patch_w + s * tile_w + x;
								int yy = v * patch_h + t * tile_h + y;

								if (xx < filteredImages[k].cols && yy < filteredImages[k].rows) {
									value_sum += cvutils::mat_get_value(filteredImages[k], yy, xx);
									count++;
								}
							}
						}

						cvutils::mat_set_value(f, s + t * 4 + k * 16, 0, value_sum / count);
					}
				}
			}

			f /= cv::norm(f);
			features.push_back(f);
		}
	}
}

void BagOfFeature::findSimilarModels(const std::vector<BagOfFeature>& bofs, std::vector<int>& results, int n) {
	TopNSearch<int> tns;

	for (int i = 0; i < bofs.size(); ++i) {
		float score = similarity(histogram, bofs[i].histogram);
		tns.add(score, i);
	}

	std::vector<int> top = tns.topN(n, TopNSearch<int>::ORDER_DESC);
	for (int i = 0; i < top.size(); ++i) {
		results.push_back(top[i]);
	}
}

float BagOfFeature::similarity(const cv::Mat& h1, const cv::Mat& h2) {
	return cvutils::mat_dot(h1, h2) / cv::norm(h1) / cv::norm(h2);
}
