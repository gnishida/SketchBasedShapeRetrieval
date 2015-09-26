#include "BagOfFeature.h"
#include "CVUtils.h"
#include "TopNSearch.h"

#define DEBUG	0

BagOfFeature::BagOfFeature(const cv::Mat& image, const std::string& filepath, const Camera& camera, float sigma, float lmbd, int patch_width, int patch_height) {
	this->filepath = filepath;
	this->camera = camera;
	extractFeatures(image, sigma, lmbd, patch_width, patch_height);
}

void BagOfFeature::computeHistogram(const std::vector<cv::Mat>& visualWords) {
	histogram = cv::Mat::zeros(visualWords.size(), 1, CV_32F);

	for (int i = 0; i < features.size(); ++i) {
		float min_dist = std::numeric_limits<float>::max();
		int min_id = -1;

		for (int j = 0; j < visualWords.size(); ++j) {
			float dist = cv::norm(features[i] - visualWords[j]);
			if (dist < min_dist) {
				min_dist = dist;
				min_id = j;
			}
		}

		cvutils::mat_add_value(histogram, min_id, 0, 1);
	}

	histogram /= features.size();
}

void BagOfFeature::extractFeatures(const cv::Mat& image, float sigma, float lmbd, int patch_width, int patch_height) {
	std::vector<cv::Mat> filteredImages;

	{
		for (int k = 0; k < 4; ++k) {
			float theta = (float)k * 45.0f / 180.0f * 3.14159265f;
			//cv::Mat kernel = cv::getGaborKernel(cv::Size(21, 21), sigma, theta, lmbd, 0.5f, 0.0, CV_32F);
			cv::Mat kernel = cv::getGaborKernel(cv::Size(31, 31), sigma, theta, lmbd, 0.5f, 0.0, CV_32F);
			kernel /= 1.0 * cv::sum(kernel)[0];

			cv::Mat dest;
			cv::filter2D(image, dest, CV_32F, kernel);

			cv::threshold(dest, dest, 125, 255, cv::THRESH_BINARY);

			/*
			char filename1[256];
			sprintf(filename1, "results/gabor_%.1lf_%.1lf_%.1lf.jpg", sigma, lmbd, theta);
			cv::imwrite(filename1, dest);
			*/

			/*
			char filename2[256];
			sprintf(filename2, "results/gabor_normalized_%.1lf_%.1lf_%.1lf.jpg", sigma, lmbd, theta);
			cvutils::mat_save(filename2, dest, true);
			*/

			filteredImages.push_back(dest);
		}
	}


	float patch_w_interval = (float)(filteredImages[0].cols - patch_width) / 31.0f;
	float patch_h_interval = (float)(filteredImages[0].rows - patch_height) / 31.0f;
	float tile_w = patch_width / 4.0f;
	float tile_h = patch_height / 4.0f;
	
	for (int u = 0; u < 32; ++u) {
		for (int v = 0; v < 32; ++v) {
			cv::Mat f = cv::Mat::zeros(4 * 4 * filteredImages.size(), 1, CV_32F);

			// もとの画像で、このパッチ内に線がなければ、スキップ
			{
				int x1 = u * patch_w_interval;
				int y1 = v * patch_h_interval;
				int x2 = std::min(x1 + patch_width, image.cols);
				int y2 = std::min(y1 + patch_height, image.rows);
	
				cv::Mat roi(image, cv::Rect(x1, y1, x2 - x1, y2 - y1));
				if (cv::sum(roi)[0] < 0.1f) continue;
			}

			for (int k = 0; k < filteredImages.size(); ++k) {
				for (int s = 0; s < 4; ++s) {
					for (int t = 0; t < 4; ++t) {
						float value_sum = 0.0f;
						int count = 0;

						for (int x = 0; x < tile_w; ++x) {
							for (int y = 0; y < tile_h; ++y) {
								int xx = u * patch_w_interval + s * tile_w + x;
								int yy = v * patch_h_interval + t * tile_h + y;

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
			
			/*{
				cv::Mat f2(8, 8, CV_64F);
				for (int r = 0; r < 2; ++r) {
					for (int c = 0; c < 2; ++c) {
						for (int r2 = 0; r2 < 4; ++r2) {
							for (int c2 = 0; c2 < 4; ++c2) {
								cvutils::mat_set_value(f2, r * 4 + r2, c * 4 + c2, cvutils::mat_get_value(f, r * 32 + c * 16 + r2 * 4 + c2, 0));
							}
						}
					}
				}
			
				char filename2[256];
				sprintf(filename2, "results/feature_normalized_%d_%d.jpg", u, v);
				cvutils::mat_save(filename2, f2, true);
			}*/

			if (cv::sum(f)[0] > 0.0f) {
				f /= cv::norm(f);
				features.push_back(f);
			}
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
	//std::cout << h1 - h2 << std::endl;

	return cvutils::mat_dot(h1, h2) / cv::norm(h1) / cv::norm(h2);
}
