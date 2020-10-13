#include "../../lib/Algorithm/CenterMapping.h"

void CenterMapping::run(IplImage * imageOrig,IplImage * workingImage){

	//Image informations
	int pictureWidth=imageOrig->width;
	int pictureHeigth=imageOrig->height;

	int nbBoxColumn=pictureWidth/8;
	int nbBoxRow=pictureHeigth/8;

	float xwidth=0.95;
	float yheight=xwidth;

	float focusweight=0.35;
	float centerweight=1-focusweight;

	float sigmaX=(xwidth*nbBoxColumn)/6;
	float sigmaY=(yheight*nbBoxRow)/6;

	float dims[2];
	dims[0]=nbBoxColumn;
	dims[1]=nbBoxRow;

	int P[2];
	P[0] = Tool::roundf(dims[0]/2);
	P[1] = Tool::roundf(dims[1]/2);

	Gaussian * gauss = new Gaussian(cv::Size(P[0]*2, P[1]*2), sigmaY,sigmaX);

	//Detect gauss max value 
	float maxVal=0;
	for(int i=0;i<nbBoxRow;i++){
		for(int j=0;j<nbBoxColumn;j++){
			if(gauss->getFilter().at<float>(i,j)>maxVal)
				maxVal=gauss->getFilter().at<float>(i,j);
		}
	}

	//Fill salmap matrix with the gaussian filtre with values between 0-255
	cv::Mat salmap(cv::Size(nbBoxColumn,nbBoxRow),CV_32F);
	for(int i=0;i<nbBoxRow;i++){
		for(int j=0;j<nbBoxColumn;j++){
			salmap.at<float>(i,j)=(gauss->getFilter().at<float>(i,j)*255)/maxVal;
		}
	}

	//Resize salmap picture to get the original picture size
	cv::Mat centermapping(cv::Size(pictureWidth,pictureHeigth),CV_32F);
	cv::resize(salmap,centermapping,cv::Size(pictureWidth,pictureHeigth));

	//Print pixel from centermapping to workingImage and add a weight to each pixel value
	for(int x=0;x<pictureHeigth;x++){
		for(int y=0;y<pictureWidth;y++){
			CvScalar cvPixel = cvGet2D( workingImage, x, y );
			CvScalar p;
			p.val[0]=p.val[1]=p.val[2]=p.val[3]=(centermapping.at<float>(x, y)*centerweight+cvPixel.val[0]*focusweight);
			cvSet2D (workingImage, x, y,p);
		}
	}
}

