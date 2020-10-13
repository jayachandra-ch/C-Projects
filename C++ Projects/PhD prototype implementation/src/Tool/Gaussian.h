#ifndef GAUSSIAN_H_
#define GAUSSIAN_H_

class Gaussian{
private:
	cv::Mat filter;
	double gaussianCoeff(double u, double v, double d0, double d1);
	double pixelDistance(double u, double v);

	cv::Mat createGaussianFilter(cv::Size size, double signaX, double signaY);

public:
	Gaussian(cv::Size size, double signaX, double signaY=0);
	cv::Mat getFilter();
};

#endif