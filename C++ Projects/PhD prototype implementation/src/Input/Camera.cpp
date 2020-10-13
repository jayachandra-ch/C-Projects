#include "../../lib/Input/Camera.h"

Camera::Camera()  {
	camera = cvCreateCameraCapture( CV_CAP_ANY );
 }

Camera::~Camera(){
	cvReleaseCapture(&camera);
}

bool Camera::isAvailble(){
	if(!camera){
		QMessageBox(QMessageBox::Warning,"Error","No Camera detected",QMessageBox::Ok).exec();
		return false;
	}
	return true;
}

bool Camera::init(double & width, double & height, IplImage * inputPicture, double & timer, Ui_InterfaceClass & ui){
	if(run(inputPicture)){
		width=input->width;
		height=input->height;
		timer=5;
		updateInterface(ui);
		return true;
	}
	return false;
}

bool Camera::run(IplImage * inputPicture){
	input=NULL;
	input=cvQueryFrame(camera);
	if(input == NULL) {
		return false;
	}
	*inputPicture=*input;
	return true;
}



