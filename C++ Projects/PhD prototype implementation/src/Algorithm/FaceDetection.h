#ifndef FACEDETECTION_H_
#define FACEDETECTION_H_
 
#include <vector>
#include "opencv2/objdetect/objdetect.hpp"

class FaceDetection{
private:
	std::string face_cascade_name;
	cv::CascadeClassifier face_cascade;
	

public:
	std::vector<cv::Rect> lstFaces;

	FaceDetection();
	void run(IplImage * imageOrig);
	void detect(cv::Mat frame);
	//std::vector<cv::Rect> getLstFaces();
	/*void detectAndDisplay(cv::Mat frame);
	float roundf(float x);
	void detect(cv::Mat frame);
	double ** createArray(int nbLin, int nbCol);
	void freeArray(double **arr);
	double pixelDistance(double u, double v);
	double gaussianCoeff(double u, double v, double d0);
	cv::Mat createGaussianHighPassFilter(cv::Size size, double cutoffInPixels);*/
};

#endif