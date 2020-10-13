#include "../../lib/Input/Picture.h"

bool Picture::init(double & width, double & height, IplImage * inputPicture, double & timer, Ui_InterfaceClass & ui){
	width=inputPicture->width;
	height=inputPicture->height;
	while(width>900 || height>900 ){
		width/=1.1;
		height/=1.1;
	}

	while(width<300 || height<300 ){
		width*=1.1;
		height*=1.1;
	}

	IplImage * resizedImg = cvCreateImage(cvSize(width,height), inputPicture->depth, inputPicture->nChannels );
	cvResize(inputPicture,resizedImg,CV_INTER_LANCZOS4);
	inputPicture=resizedImg;	

	timer=-1;
	return true;
}
bool Picture::run(IplImage * inputPicture){return true;}
bool Picture::isAvailble(){return true;}