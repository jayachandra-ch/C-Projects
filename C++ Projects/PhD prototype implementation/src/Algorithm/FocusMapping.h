#ifndef FOCUSMAPPING_H_
#define FOCUSMAPPING_H_

#include "opencv2/imgproc/imgproc.hpp"

class FocusMapping{
private:

public:
	FocusMapping();
	void run(IplImage * imageOrig,IplImage * workingImage);
	
};

#endif
