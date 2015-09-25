#include "CVUtils.h"

namespace cvutils {

/**
 * 行列の指定した行、列の値を返却する。
 *
 * @param m		行列
 * @param r		行
 * @param c		列
 * @return		値
 */
double mat_get_value(const cv::Mat& m, int r, int c) {
	if (m.type() == CV_8U) {
		return m.at<uchar>(r, c);
	} else if (m.type() == CV_8UC3) {
		return m.at<cv::Vec3i>(r, c)[0];
	} else if (m.type() == CV_32F) {
		return m.at<float>(r, c);
	} else if (m.type() == CV_64F) {
		return m.at<double>(r, c);
	} else {
		return 0.0;
	}
}

/**
 * 行列の指定した行、列の値を更新する。
 *
 * @param m		行列
 * @param r		行
 * @param c		列
 * @return		値
 */
void mat_set_value(cv::Mat& m, int r, int c, double val) {
	if (m.type() == CV_8U) {
		m.at<uchar>(r, c) = val;
	} else if (m.type() == CV_8UC3) {
		m.at<cv::Vec3i>(r, c)[0] = val;
		m.at<cv::Vec3i>(r, c)[1] = val;
		m.at<cv::Vec3i>(r, c)[2] = val;
	} else if (m.type() == CV_32F) {
		m.at<float>(r, c) = val;
	} else if (m.type() == CV_64F) {
		m.at<double>(r, c) = val;
	}
}

void mat_add_value(cv::Mat& m, int r, int c, double val) {
	if (m.type() == CV_8U) {
		m.at<uchar>(r, c) += val;
	} else if (m.type() == CV_8UC3) {
		m.at<cv::Vec3i>(r, c)[0] += val;
		m.at<cv::Vec3i>(r, c)[1] += val;
		m.at<cv::Vec3i>(r, c)[2] += val;
	} else if (m.type() == CV_32F) {
		m.at<float>(r, c) += val;
	} else if (m.type() == CV_64F) {
		m.at<double>(r, c) += val;
	}
}

/**
 * 行列の要素和を返却する。
 *
 * @param m		行列
 */
double mat_sum(const cv::Mat& m) {
	return cv::sum(m)[0];
}

double mat_dot(const cv::Mat& m1, const cv::Mat& m2) {
	double ret = 0.0f;

	if (m1.cols == 1 && m2.cols == 1) {
		for (int i = 0; i < m1.rows; ++i) {
			ret += mat_get_value(m1, i, 0) * mat_get_value(m2, i, 0);
		}
	} else if (m1.cols == 1 && m2.rows == 1) {
		for (int i = 0; i < m1.rows; ++i) {
			ret += mat_get_value(m1, i, 0) * mat_get_value(m2, 0, i);
		}
	} else if (m1.rows == 1 && m2.cols == 1) {
		for (int i = 0; i < m1.cols; ++i) {
			ret += mat_get_value(m1, 0, i) * mat_get_value(m2, i, 0);
		}
	} else {
		for (int i = 0; i < m1.cols; ++i) {
			ret += mat_get_value(m1, 0, i) * mat_get_value(m2, 0, i);
		}
	}

	return ret;
}

/**
 * 行列matを画像として保存する。ただし、1が白色、0が黒色となる。
 *
 * @param filename		ファイル名
 * @param mat			行列
 * @normalize			trueなら、最小値を黒、最大値を白色となるよう、スケールする
 */
void mat_save(char* filename, const cv::Mat& mat, bool normalize) {
	cv::Mat img;

	if (normalize) {
		double max_val;
		double min_val;
		double min, max;
		cv::minMaxLoc(mat, &min_val, &max_val);

		double scale = 255.0 / (max_val - min_val);
		mat.convertTo(img, CV_32F, scale, -min_val * scale);
	}

	//cv::flip(mat, img, 0);
	cv::imwrite(filename, img);
}

void mat_resize(cv::Mat& m, const cv::Size& size, bool keepAspectRatio) {
	if (!keepAspectRatio) {
		cv::resize(m, m, size);
	} else {
		cv::Size orig_size = m.size();

		float scale_w = (float)size.width / m.cols;
		float scale_h = (float)size.height / m.rows;
		if (scale_w < scale_h) {
			cv::resize(m, m, cv::Size(size.width, orig_size.height * scale_w));
		} else {
			cv::resize(m, m, cv::Size(orig_size.width * scale_h, size.height));
		}

		cv::Mat temp = m.clone();
		m = cv::Mat::zeros(size, m.type());
		if (scale_w < scale_h) {
			cv::Mat roi(m, cv::Rect(0, (m.rows - temp.rows) * 0.5, m.cols, temp.rows));
			temp.copyTo(roi);
		} else {
			cv::Mat roi(m, cv::Rect((m.cols - temp.cols) * 0.5, 0, temp.cols, m.rows));
			temp.copyTo(roi);
		}
	}
}

/**
 * 背景色が白色、線の色が黒色の時に、線を囲むbounding boxを計算する。
 */
cv::Rect computeBoundingBoxFromImage(const cv::Mat& img) {
	cv::Rect rect;

	cv::Mat row_img, col_img;
	cv::reduce(img, row_img, 0, CV_REDUCE_MIN);
	cv::reduce(img, col_img, 1, CV_REDUCE_MIN);

	{ // horizontal scan
		bool flag = false;
		for (int c = 0; c < row_img.cols; ++c) {
			double val = mat_get_value(row_img, 0, c);

			if (!flag) {
				if (val == 0) {
					rect.x = c;
					flag = true;
				}
			} else {
				if (val > 0) {
					rect.width = c - rect.x + 1;
					break;
				}
			}
		}
	}

	{ // vertical scan
		bool flag = false;
		for (int r = 0; r < col_img.rows; ++r) {
			double val = mat_get_value(col_img, r, 0);

			if (!flag) {
				if (val == 0) {
					rect.y = r;
					flag = true;
				}
			} else {
				if (val > 0) {
					rect.height = r - rect.y + 1;
					break;
				}
			}
		}
	}

	return rect;
}

}