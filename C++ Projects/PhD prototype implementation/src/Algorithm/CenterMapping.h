#ifndef CENTERMAPPING_H_
#define CENTERMAPPING_H_

#include "opencv2/objdetect/objdetect.hpp"
#include "Runable.h"

class CenterMapping : public Runable{
private:
	cv::Mat pict;

public:
	void run(IplImage * imageOrig,IplImage * workingImage);
	float roundf(float x);
};
#endif
