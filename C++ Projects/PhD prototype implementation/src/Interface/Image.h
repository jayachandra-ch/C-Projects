#ifndef IMAGE_H_
#define IMAGE_H_

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv/cv.h>
#include "FaceMapping.h"
#include "FocusMapping.h"

class Image{
private:
	IplImage * imageOrig;
	IplImage * imageOrigGrey;
	IplImage * workingImage;
	FaceMapping * mapping_face;
	FocusMapping * mapping_focus;

public:
	Image(IplImage * image,FaceMapping * mapping_face,FocusMapping * mapping_focus);
	~Image();
	void setImage(IplImage * image);
	void saillencyDetection();
	IplImage * display();
	void init(IplImage * image);
	cv::Mat padarray(IplImage * image,int nbBoxColumn,int nbBoxRow);
	void Image::ictD(double ** block);
	cv::Mat ict(cv::Mat image,int i, int j);
	float Image::roundf(float x);
	void zigzagscan(double tab[][8], double * out,int num_rows, int num_cols);


	void peakdet(double tab[2][64],int delta,std::vector<std::pair<double,double>> & maxTab);
	void Image::findPos(double ** frequencypositions, double * primaryfrequencyvalues,int pos);
	double ** createArray(int nbRow, int nbCol);
	double ** createArray2(int nbRow, int nbCol);
	void freeArray(double **arr);
	cv::Mat Image::cropFade(cv::Mat _img, cv::Rect _roi, int _maxFadeDistance);
};

#endif