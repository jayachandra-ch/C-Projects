#include "../../lib/Interface/Image.h"

Image::Image(IplImage * image,Process * process){
	Image::imageOrig=image;
	Image::process = process;
	workingImage=cvCreateImage(cvSize(imageOrig->width,imageOrig->height),IPL_DEPTH_8U,imageOrig->nChannels);
	cvSet(workingImage, cvScalar(0,0,0));
}

void Image::setImage(IplImage * image,bool focus, bool center, bool face){
	imageOrig=image;
	cvSet(workingImage, cvScalar(0,0,0));   
	process->run(imageOrig,workingImage);
}

IplImage * Image::display(){
	return workingImage;
}


