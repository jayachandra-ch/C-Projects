#include "../../lib/Algorithm/FocusMapping.h"

void FocusMapping::run(IplImage * imageOrig,IplImage * workingImage){

	//Convert IplImage to HSV and keep the V im a grey picture
	cv::Mat image=cv::cvarrToMat(imageOrig);
	cv::Mat grayImg, hsvImg;
	cvtColor(image, grayImg, CV_BGR2GRAY);
	cvtColor(image, hsvImg, CV_BGR2HSV);
	uchar* grayDataPtr = grayImg.data;
	uchar* hsvDataPtr = hsvImg.data;
	for (int i = 0; i < image.rows; i++)
	{
		for (int j = 0; j < image.cols; j++)
		{
			const int hi = i*image.cols*3 + j*3,
				gi = i*image.cols + j;
			grayDataPtr[gi] = hsvDataPtr[hi + 2];
		}
	}
	IplImage * imageOrigGrey=new IplImage(grayImg);

	//Image informations
	int pictureWidth=imageOrigGrey->width;
	int pictureHeigth=imageOrigGrey->height;

	int nbBoxColumn=pictureWidth/8;
	int nbBoxRow=pictureHeigth/8;

	int r_nbBoxColumn=pictureWidth%8;
	int r_nbBoxRow=pictureHeigth%8;


	if(r_nbBoxColumn!=0)
		r_nbBoxColumn=8-r_nbBoxColumn;
	else
		r_nbBoxColumn=0;

	if(r_nbBoxRow!=0)
		r_nbBoxRow=8-r_nbBoxRow;
	else
		r_nbBoxRow=0;

	//Rescale to be a multiple of 8
	cv::Mat grey = padarray(imageOrigGrey, r_nbBoxColumn, r_nbBoxRow);

	//Apply DCT to the image with 8X8 blocks
	for(int i=0;i<nbBoxRow;i++){
		for(int j=0;j<nbBoxColumn;j++){

			//Fill a 8X8 block into a 1D array
			float greyMiniD[64];
			for(int ii=0;ii<8;ii++){
				for(int jj=0;jj<8;jj++){
					greyMiniD[(ii<<3)+jj]=grey.at<float>(i*8+ii,j*8+jj)/255;
				}
			}

			DCT(greyMiniD);

			//Assign DCT result to the original picture
			for(int ii=0;ii<8;ii++){
				for(int jj=0;jj<8;jj++){
					grey.at<float>(i*8+ii,j*8+jj)=abs(greyMiniD[(ii<<3)+jj]);
				}
			}

		}
	}

	//Initialize 64 float 1D array to 0
	float superBlock[64];
	memset(superBlock,0,64*sizeof(float));

	//Calculate superBlock. superBlock is the sum of each 8X8 block of the DCT picture
	for(int i=0;i<nbBoxRow;i++){
		for(int j=0;j<nbBoxColumn;j++){

			for(int m=(i<<3);m<((i<<3)+8);m++){
				for(int n=(j<<3);n<((j<<3)+8);n++){
					superBlock[(m-(i<<3))*8+n-(j<<3)]+=grey.at<float>(m,n);
				}
			}

		}
	}

	//Initialize 64 float 1D array to 0
	float imageout1D[64];
	memset(imageout1D,0,64*sizeof(float));

	//Process zigzag algorithm
	zigzagscan(superBlock, imageout1D,8,8);

	//Detect maximum values with theire position in the imageout1D variable
	std::vector<std::pair<float,float>> maxTab;
	peakdet(imageout1D,0.0001,maxTab);

	int nbMaxRows = maxTab.size();
	int nbMaxCols = 2;

	//set to 0 and count unrelevant value from the maxTab array
	for(int i=0;i<nbMaxRows;i++){
		if(maxTab[i].first<2 || maxTab[i].first>30)
			maxTab[i].first=0;
	}
	int increment = 0;
	float * primaryfrequencyvalues = (float*)malloc(sizeof(float)*nbMaxRows);
	for(int i=0;i<nbMaxRows;i++){
		primaryfrequencyvalues[i]=0;
		if(maxTab[i].first!=0){
			primaryfrequencyvalues[increment]=maxTab[i].first;
			++increment;
		}
	}
	int nonzerovaluecount=0;
	for(int i=0;i<nbMaxRows;i++){
		if(primaryfrequencyvalues[i] !=0){
			nonzerovaluecount++;
		}
	}

	//Detect x,y zigzag array coordinate of each none zero value into primaryfrequencyvalues 
	float ** frequencypositions = Tool::createArray(nonzerovaluecount,2);
	for(int i=0;i<nonzerovaluecount;i++){
		findPos(frequencypositions, primaryfrequencyvalues, i);	
	}

	
	int nbFreqRows = nonzerovaluecount;
	cv::Mat final;
	final =  cv::Mat::zeros(cv::Size(nbBoxColumn,nbBoxRow), CV_32F);

	int posX,posY;
	float val,sum;
	//Fill saliency matrix
	for(int i=0;i<nbBoxRow;i++){
		for(int j=0;j<nbBoxColumn;j++){
			sum=0;
			for(int k=0;k<nbFreqRows;k++){
				posX=(8*i+frequencypositions[k][0]);
				posY=(8*j+frequencypositions[k][1]);
				val=grey.at<float>(posX,posY);
				sum=sum+val;
			}
			final.at<float>(i,j)=sum;
		}
	}

	//Blur saliency matrix and increase pixels values
	final =cropFade(final,cv::Rect(5, 5, final.cols-10, final.rows-10), 5);
	cv::GaussianBlur(final,final, cv::Size(11,11), 4);
	for(int i=0;i<nbBoxRow;i++){
		for(int j=0;j<nbBoxColumn;j++){
			final.at<float>(i,j)=final.at<float>(i,j)*2.6;
		}
	}

	//Normalize final mtrix to get pixel between 0-255 and resize it to the original size
	float maxVal=0;
	for(int i=0;i<nbBoxRow;i++){
		for(int j=0;j<nbBoxColumn;j++){
			if(final.at<float>(i,j)>maxVal)
				maxVal=final.at<float>(i,j);
		}
	}
	for(int i=0;i<nbBoxRow;i++){
		for(int j=0;j<nbBoxColumn;j++){
			final.at<float>(i,j)=((final.at<float>(i,j)*255)/maxVal);
		}
	}
	cv::Mat finalSize;
	cv::resize(final,finalSize,cv::Size(pictureWidth,pictureHeigth));

	//Copy each pixel from finalSize matrix to workingImage
	for(int x=0;x<pictureHeigth;x++){
		for(int y=0;y<pictureWidth;y++){
			CvScalar cvPixel = cvGet2D( workingImage, x, y );
			CvScalar p;
			p.val[0]=p.val[1]=p.val[2]=p.val[3]=finalSize.at<float>(x, y)+cvPixel.val[0];
			cvSet2D (workingImage, x, y,p);
		}
	}
}


cv::Mat FocusMapping::cropFade(cv::Mat _img, cv::Rect _roi, int _maxFadeDistance)
{
	cv::Mat fadeMask = cv::Mat::ones(_img.size(), CV_8UC1);
	cv::rectangle(fadeMask, _roi, cv::Scalar(0),-1);

	cv::Mat dt;
	cv::distanceTransform(fadeMask > 0, dt, CV_DIST_L2 ,CV_DIST_MASK_PRECISE);

	// fade to a maximum distance:
	float maxFadeDist;

	if(_maxFadeDistance > 0)
		maxFadeDist = _maxFadeDistance;
	else
	{
		// find min/max vals
		double min,max;
		cv::minMaxLoc(dt,&min,&max);
		maxFadeDist = max;
	}
	dt = 1.0-(dt* 1.0/maxFadeDist); // values between 0 and 1 in fading region

	cv::Mat imgF;
	_img.convertTo(imgF,CV_32FC3);


	std::vector<cv::Mat> channels;
	cv::split(imgF,channels);

	// multiply pixel value with the quality weights for image 1
	for(unsigned int i=0; i<channels.size(); ++i)
		channels[i] = channels[i].mul(dt);

	cv::Mat outF;
	cv::merge(channels,outF);

	cv::Mat out;
	outF.convertTo(out,CV_32F);

	return out;
}


float zigzagarray[8][8]={
	{ 1, 2, 6, 7,15,16,28,29},
	{ 3, 5, 8,14,17,27,30,43},
	{ 4, 9,13,18,26,31,42,44},
	{10,12,19,25,32,41,45,54},
	{11,20,24,33,40,46,53,55},
	{21,23,34,39,47,52,56,61},
	{22,35,38,48,51,57,60,62},
	{36,37,49,50,58,59,63,64}
};
void FocusMapping::findPos(float ** frequencypositions, float * primaryfrequencyvalues,int pos){

	for(int i=0;i<8;i++){
		for(int j=0;j<8;j++){
			if(primaryfrequencyvalues[pos]==zigzagarray[i][j]){
				frequencypositions[pos][0]=i;
				frequencypositions[pos][1]=j;
			}
		}
	}
}


void FocusMapping::peakdet(float tab[64],int delta,std::vector<std::pair<float,float>> & maxTab){
	int max=-2147483648;
	int maxPos=0;

	int min=2147483647;
	int minPos=0;

	float currentVal;
	bool lookformax = true;

	for(int i=0;i<64;i++){
		currentVal=tab[i];
		if (currentVal > max){
			max = currentVal; 
			maxPos = i;
		}
		if (currentVal < min){
			min = currentVal; 
			minPos = i;
		}

		if (lookformax){
			if (currentVal < max-delta){
				std::pair<float, float> paire;
				paire.first = maxPos;
				paire.second = max;
				maxTab.push_back(paire);

				min = currentVal; 
				minPos = i;

				lookformax = false; 
			}
		}else{
			if (currentVal > min+delta){
				max = currentVal; 
				maxPos = i;

				lookformax = true; 
			}
		}
	}
}


cv::Mat FocusMapping::padarray(IplImage * image,int r_nbBoxColumn,int r_nbBoxRow){
	cv::Mat grey = cv::cvarrToMat(image,true);

	cv::Mat image_pad; 

	copyMakeBorder(grey,image_pad,0,r_nbBoxRow,0,r_nbBoxColumn,cv::BORDER_CONSTANT,cv::Scalar(0));
	image_pad.convertTo(image_pad, CV_32F);

	return image_pad;
}


float tab[64]={1,1,1,1,1,1,1,1,
	1.5,1.25,0.75,0.375,-0.375,-0.75,-1.25,-1.5,
	1,0.5,-0.5,-1,-1,-0.5,0.5,1,
	1.25,-0.375,-1.5,-0.75,0.75,1.5,0.375,-1.25,
	1,-1,-1,1,1,-1,-1,1,
	0.75,-1.5,0.375,1.25,-1.25,-0.375,1.5,-0.75,
	0.5,-1,1,-0.5,-0.5,1,-1,0.5,
	0.375,-0.75,1.25,-1.5,1.5,-1.25,0.75,-0.375};
void FocusMapping::DCT(float * block){
	double tmp2[64];
	double tmp[64];
	double sum;
	for(int i=0; i<8; i++){
		for(int j=0; j<8; j++){
			sum=0;
			for(int k=0;k<8;k++){
				sum=sum+tab[(i<<3)+k]*block[(k<<3)+j];
			}
			tmp[(i<<3)+j]=sum;
		}
	}
	for(int i=0; i<8; i++){
		for(int j=0; j<8; j++){
			sum=0;
			for(int k=0;k<8;k++){
				sum=sum+tmp[(i<<3)+k]*tab[(j<<3)+k];
			}
			block[(i<<3)+j]=sum;

		}
	}
}


void FocusMapping::zigzagscan(float in[8], float * out, int num_rows, int num_cols){
	out[0]=in[0];
	out[1]=in[1];
	out[2]=in[8];
	out[3]=in[16];
	out[4]=in[9];
	out[5]=in[2];
	out[6]=in[3];
	out[7]=in[10];
	out[8]=in[17];
	out[9]=in[24];
	out[10]=in[32];
	out[11]=in[25];
	out[12]=in[18];
	out[13]=in[11];
	out[14]=in[4];
	out[15]=in[5];
	out[16]=in[12];
	out[17]=in[19];
	out[18]=in[26];
	out[19]=in[33];
	out[20]=in[40];
	out[21]=in[48];
	out[22]=in[41];
	out[23]=in[34];
	out[24]=in[27];
	out[25]=in[20];
	out[26]=in[13];
	out[27]=in[6];
	out[28]=in[7];
	out[29]=in[14];
	out[30]=in[21];
	out[31]=in[28];
	out[32]=in[35];
	out[33]=in[42];
	out[34]=in[49];
	out[35]=in[56];
	out[36]=in[57];
	out[37]=in[50];
	out[38]=in[43];
	out[39]=in[36];
	out[40]=in[29];
	out[41]=in[22];
	out[42]=in[15];
	out[43]=in[23];
	out[44]=in[30];
	out[45]=in[37];
	out[46]=in[44];
	out[47]=in[51];
	out[48]=in[58];
	out[49]=in[59];
	out[50]=in[52];
	out[51]=in[45];
	out[52]=in[38];
	out[53]=in[31];
	out[54]=in[39];
	out[55]=in[46];
	out[56]=in[53];
	out[57]=in[60];
	out[58]=in[61];
	out[59]=in[54];
	out[60]=in[47];
	out[61]=in[55];
	out[62]=in[62];
	out[63]=in[63];
}

