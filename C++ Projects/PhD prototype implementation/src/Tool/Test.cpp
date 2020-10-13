#include "../../lib/Tool/Test.h"

Test::Test(IplImage * img1){
	Test::img1=img1;
	const char* PATH = "D:\\Thomas guibert\\JAYSK VISUAL SALIENCY MODEL (Thomas)\\saliency.bmp";

	img2=cvLoadImage(PATH);

	IplImage * workingImage=cvCreateImage(cvSize(img1->width,img1->height),IPL_DEPTH_8U,img1->nChannels);
	cvSet(workingImage, cvScalar(255,255,255));

	for(int i=0;i<img1->height*img1->width*img1->nChannels;i++){
			workingImage->imageData[i]=abs(img1->imageData[i]-img2->imageData[i]);
	}
	cvNamedWindow("c++ output");
	cvShowImage("c++ output",Test::img1);
	cvNamedWindow("matlab output");
	cvShowImage("matlab output",img2);
	cvNamedWindow("difference");
	cvShowImage("difference",workingImage);
}