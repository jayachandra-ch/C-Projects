#ifndef CAMERA_H_
#define CAMERA_H_

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class Camera {
    private:
		IplImage * input;
        CvCapture * camera;

    public:
        Camera();
		IplImage * getInput();
              
};


#endif 
