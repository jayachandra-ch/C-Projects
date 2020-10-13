#include "../../lib/Tool/Gaussian.h"

Gaussian::Gaussian(cv::Size size, float sigmaX, float sigmaY){
	if(sigmaY==0)
		filter=createGaussianFilter(size,sigmaX,sigmaX);
	else
		filter=createGaussianFilter(size,sigmaX,sigmaY);
}

float Gaussian::gaussianCoeff(float u, float v, float d0, float d1)
{
	return exp(-(((u*u)/(2*d0*d0))+((v*v)/(2*d1*d1))));
}

cv::Mat Gaussian::createGaussianFilter(cv::Size size,  float sigmaX, float sigmaY)
{
	cv::Mat ghpf(size, CV_32F  );

	cv::Point center(size.width / 2, size.height / 2);


	//Create a gaussian matrix
	for(int u = -ghpf.rows/2; u < ghpf.rows/2; u++){
		for(int v = -ghpf.cols/2; v < ghpf.cols/2; v++){
			ghpf.at<float>(u+ghpf.rows/2, v+ghpf.cols/2) = gaussianCoeff(u, v, sigmaX, sigmaY);
		}
	}

	//Find the gaussian matrix's maximum 
	float maxVal=0;
	for(int i=0;i<ghpf.rows;i++){
		for(int j=0;j<ghpf.cols;j++){
			if(ghpf.at<float>(i,j)>maxVal)
				maxVal=ghpf.at<float>(i,j);
		}
	}

	for(int i=0;i<ghpf.rows;i++){
		for(int j=0;j<ghpf.cols;j++){
			ghpf.at<float>(i,j)=(ghpf.at<float>(i,j)/maxVal)*255;
		}
	}

	return ghpf;
}

cv::Mat Gaussian::getFilter(){
	return filter;
}