#include "../../lib/Algorithm/FaceMapping.h"

FaceMapping::FaceMapping(){
	faceDetection = new FaceDetection();
}

void FaceMapping::run(IplImage * imageOrig,IplImage * workingImage){
	
	faceDetection->run(imageOrig);
	
	std::vector<cv::Rect> faces = faceDetection->getLstFaces();

	for (int i = 0; i < faces.size(); i++) 
	{
		//Image informations
		float sigmaX,sigmaY;
		float dims[2];
		int P[2];
		float val;

		sigmaX=(faces[i].width/6)*1.6;
		sigmaY=sigmaX;

		dims[0]=faces[i].width;
		dims[1]=faces[i].height;

		P[0] = Tool::roundf(dims[0]/2);
		P[1] = Tool::roundf(dims[1]/2);

		gauss = new Gaussian(cv::Size(P[1]*2+1, P[0]*2+1), sigmaX);
		
		int gaussPosX=0;
		int gaussPosY=0;

		cv::Point pt1(faces[i].x, faces[i].y); 
		cv::Point pt2((faces[i].x + faces[i].height), (faces[i].y + faces[i].width));

		//Gaussian filter applying on workingImage thanks to faces boxes
		for(int x=pt1.x;x<pt2.x;x++){
			for(int y=pt1.y;y<pt2.y;y++){
				CvScalar cvPixel = cvGet2D( workingImage, y, x );
				CvScalar p;
				//Originaly, boxes are squares. This test has been added to get a circle instead of square.
				if(gauss->getFilter().at<float>(gaussPosX, gaussPosY)>40)
					p.val[0]=p.val[1]=p.val[2]=p.val[3]=gauss->getFilter().at<float>(gaussPosX, gaussPosY)+cvPixel.val[0];
				else
					p.val[0]=p.val[1]=p.val[2]=p.val[3]=cvPixel.val[0];
				cvSet2D (workingImage, y, x,p);
				gaussPosX++;
			}
			gaussPosX=0;
			gaussPosY++;
		}
	}
}