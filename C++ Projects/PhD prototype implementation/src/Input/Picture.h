#ifndef PICTURE_H_
#define PICTURE_H_
#include "Input.h"

class Picture :public Input{
	bool init(double & width, double & height, IplImage * inputPicture, double & timer, Ui_InterfaceClass & ui);
	bool run(IplImage * inputPicture);
	bool isAvailble();
};

#endif