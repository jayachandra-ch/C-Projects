#ifndef FACEMAPPING_H_
#define FACEMAPPING_H_

#include "opencv2/imgproc/imgproc.hpp"
#include "Gaussian.h"
#include "FaceDetection.h"

class FaceMapping{
private:
	Gaussian * gauss;
	FaceDetection * faceDetection;

public:
	FaceMapping();
	void run(IplImage * imageOrig,IplImage * workingImage);
	float roundf(float x);
};

#endif
