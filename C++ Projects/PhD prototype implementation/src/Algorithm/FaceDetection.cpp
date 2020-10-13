#include "../../lib/Algorithm/FaceDetection.h"

FaceDetection::FaceDetection(){
	face_cascade_name="./cascade/haarcascade_frontalface_alt.xml";
	error=false;
}

void FaceDetection::run(IplImage * imageOrig){
	// Load the cascade
	if (!face_cascade.load(face_cascade_name)){
		if(!error){
			QMessageBox(QMessageBox::Warning,"Error","Impossible to load Haar Cascade file",QMessageBox::Ok).exec();
			error=true;
		}
	};

	//Convert picture into matrix
	cv::Mat frame = cv::cvarrToMat(imageOrig,true);
	if (!frame.empty()){
		detect(frame);//run the detection
	}
	else{
		if(!error){
			QMessageBox(QMessageBox::Warning,"Error","No captured frame",QMessageBox::Ok).exec();
			error=true;
		}
	}
}

void FaceDetection::detect(cv::Mat frame)
{
	cv::Mat frame_gray;

	cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
	equalizeHist(frame_gray, frame_gray);

	face_cascade.detectMultiScale(frame_gray, lstFaces, 1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(50,50));
}

