 
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv/ml.h>

//#include <opencv/cv.h>
//#include <opencv/cxcore.h>
//#include <opencv/highgui.h>

#include "GraphUtils.h"
 
#include <stdio.h>

#include <iostream>

 
using namespace cv;
using namespace std;

#define PI 3.14159265

int selectedRange = 0;
Range rs[2];
bool draw = false; 
bool graph_realTime=true;

enum focus_measurement_method
{
   newLaplacian=0, varianceLaplacian, sobelEdge, varianceIntensity
};   

void help(char** argv)
{
    cout << "\nThis software will do the assessment of the camera focus and the image sharpness\n";
    cout << "Usage: " << argv[0] << " <focus-meashurement-method-number: 0, 1, 2 or 3>"<< endl;
    cout << "focus-meashurement-method" <<" 0: modifiedLaplacian"<< endl;
    cout << "focus-meashurement-method" <<" 1: varianceOfLaplacian"<< endl;
    cout << "focus-meashurement-method" <<" 2: sobelEdge"<< endl;
    cout << "focus-meashurement-method" <<" 3: varianceIntensity"<< endl;        
}

double modifiedLaplacian(const cv::Mat& src)
{
    cv::Mat M = (Mat_<double>(3, 1) << -1, 2, -1);
    cv::Mat G = cv::getGaussianKernel(3, -1, CV_64F);

    cv::Mat Lx;
    cv::sepFilter2D(src, Lx, CV_64F, M, G);

    cv::Mat Ly;
    cv::sepFilter2D(src, Ly, CV_64F, G, M);

    cv::Mat FM = cv::abs(Lx) + cv::abs(Ly);

    double focusMeasure = cv::mean(FM).val[0];
    return focusMeasure;
}


double varianceOfLaplacian(const cv::Mat& src)
{
    cv::Mat lap;
    cv::Laplacian(src, lap, CV_64F);

    cv::Scalar mu, sigma;
    cv::meanStdDev(lap, mu, sigma);

    double focusMeasure = sigma.val[0]*sigma.val[0];
    return focusMeasure;
}


double tenengrad(const cv::Mat& src, int ksize)
{
    cv::Mat Gx, Gy;
    cv::Sobel(src, Gx, CV_64F, 1, 0, ksize);
    cv::Sobel(src, Gy, CV_64F, 0, 1, ksize);

    cv::Mat FM = Gx.mul(Gx) + Gy.mul(Gy);

    double focusMeasure = cv::mean(FM).val[0];
    return focusMeasure;
}


double normalizedGraylevelVariance(const cv::Mat& src)
{
    cv::Scalar mu, sigma;
    cv::meanStdDev(src, mu, sigma);

    double focusMeasure = (sigma.val[0]*sigma.val[0]) / mu.val[0];
    return focusMeasure;
}

void on_mouse( int event, int x, int y, int flags, void* param )
{
	Mat* pm = (Mat*)param;
    switch( event )
    {
        case CV_EVENT_LBUTTONDOWN: 
        {    
			Mat tmp; (*((Mat*)param)).copyTo(tmp);
			imshow("input image", tmp);
			
			//rs[selectedRange*2].start = min(max(0,y),pm->cols-1);
			rs[selectedRange*2].start = min(max(0,y),pm->rows-1);//corrected by Hongshi
			//rs[selectedRange*2+1].start = min(max(0,x),pm->rows-1);
			rs[selectedRange*2+1].start = min(max(0,x),pm->cols-1);//corrected by Hongshi
			cout << "Begin " << x << "," << y << endl;
			draw = true;
        }
        break;
        
        case CV_EVENT_RBUTTONDOWN: 
        break;
        
        case CV_EVENT_LBUTTONUP:
        {			
			rs[selectedRange*2].end = min(max(0,y),pm->rows-1); 	
			rs[selectedRange*2+1].end = min(max(0,x),pm->cols-1);
			cout << "End " << x << "," << y << endl;
			draw = false;
        }
        break;
        
        case CV_EVENT_RBUTTONUP:
        break;
        
        case CV_EVENT_MOUSEMOVE:
		if(draw) {
			Mat _tmp; (*((Mat*)param)).copyTo(_tmp);
			if(selectedRange!=0)
				rectangle(_tmp,Point(rs[1].start,rs[0].start),Point(rs[1].end,rs[0].end),Scalar(255,0,0),2);
			else
				rectangle(_tmp,Point(rs[1].start,rs[0].start),Point(x,y),Scalar(255,0,0),2);
						
			imshow("input image",_tmp);
			//waitKey();
		}
        break;
               
    }
    
}
 
int main(int argc, char** argv)
{
    if(argc != 2)
    {
        help(argv);
        return -1;
    }
    
             
    int methodSelect=atoi(argv[1]);
      
    vector<float> focusMeasurement_vector;

	Mat image_original_noFlip;
	Mat image_original;
	Mat image_original_crop;
	
	VideoCapture sequence(0); // open the default camera
	//VideoCapture sequence(1);
	
	sequence.set(CV_CAP_PROP_FRAME_WIDTH , 1280);
    sequence.set(CV_CAP_PROP_FRAME_HEIGHT , 720);
	
    if(!sequence.isOpened())  // check if we succeeded
    {
        cerr << "Failed to capture video data from web camera!\n" << endl;
        return -1;
    }	
    
    sequence >> image_original_noFlip;
    
    if (image_original_noFlip.data == NULL)
    {      
		std::cerr << "ERROR: Could not capture video frame." << std::endl;
        return -1;
    }
    
    flip(image_original_noFlip, image_original, 1);
	
	string wndname0 = "cropped image";
	string wndname1 = "video frame | q or esc to quit";
	   
    
    rs[0] = Range(0, image_original.rows-1);
	rs[1] = Range(0, image_original.cols-1);

	
	namedWindow("input image", CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
	imshow("input image", image_original);
	
	cv::setMouseCallback("input image" , on_mouse, &image_original);
	
    
             cout<<""<<endl; 
			 cout << "Please define the target area with your mouse."<<endl; 
			 cout<<"When you are satisfied, press the key q to next step!"<<endl;
			 cout<<""<<endl;
             cout<<""<<endl; 
                              					
		    			 
		     while(true)
		     {			    
		         if(abs(rs[0].end-rs[0].start)<20||abs(rs[1].end-rs[1].start)<20)
	             {
                      cout<<""<<endl;
                      cout<<""<<endl;
                      cout<<"The selected area is too small!"<<endl;
                      cout<<"Please re-select the area! and press key q to to next step!"<<endl; 
                      
                      imshow("input image", image_original);                                                     
                  }
             
                  int c = waitKey();
				  if(c=='q') break;
		      }
	          
	          cv::setMouseCallback("input image", NULL, NULL); //stop the mouseCallback event
	          
	          if(abs(rs[0].end-rs[0].start)<20||abs(rs[1].end-rs[1].start)<20)
	          {
                   cout<<""<<endl;
                   cout<<""<<endl;
                   cout<<"The selected area is too small!"<<endl;
                   cout<<"The software cannot continue, and has to stop now!"<<endl;   
                   return -1;                                                   
              }
		   
		      //image_original(rs[0],rs[1]).copyTo(image_original_crop);
		      
		      int xStart, xEnd, yStart, yEnd;
		      
		      yStart=rs[0].start;            
              xStart=rs[1].start;            
             
              yEnd=rs[0].end;
              xEnd=rs[1].end;
                           
              if(xStart>xEnd)
              {  
				 int tempX;
                 tempX=xStart;
                 xStart=xEnd;
                 xEnd=tempX;
              }
              
              
              if(yStart>yEnd)
              {
                 int tempY;
                 tempY=yStart;
                 yStart=yEnd;
                 yEnd=tempY;
              }
              
                    
		      int Width=abs(xEnd-xStart);
		      int Height=abs(yEnd-yStart);
		      
		      image_original(Rect(xStart,yStart, Width, Height)).copyTo(image_original_crop);
		      
									
    namedWindow(wndname0, CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
    imshow(wndname0, image_original_crop);
    
    cout<<""<<endl;
    cout << "Press any key to continue processing frame by frame, and press the key q to complete."<<endl; 
    waitKey();
	

     
    
    int frame_counter=0;
    Mat frame_original_noFlip;
    Mat frame_original;
    Mat frame_original_crop;
    Mat frame_gray_crop;
    
    IplImage *imageGraph;
    
	for(;;)
    { 
		for(int i=0; i<5; i++)
		sequence >> frame_original_noFlip;
		
		if (image_original_noFlip.data == NULL)
        {      
		    std::cerr << "ERROR: video frame capture error." << std::endl;
            return -1;
        }
		
		flip(frame_original_noFlip, frame_original, 1);
                
        //frame_original(rs[0],rs[1]).copyTo(frame_original_crop);
        frame_original(Rect(xStart,yStart, Width, Height)).copyTo(frame_original_crop);
        
    
        cvtColor(frame_original_crop, frame_gray_crop, cv::COLOR_RGB2GRAY);
       
	    namedWindow(wndname1, CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO);
    
        imshow(wndname1, frame_original_crop);
        
        char filename[100];
        sprintf(filename, "frame_%d.png", frame_counter);
        
        //imwrite(filename, frame_original_crop);
        
        //waitKey(1);   

        printf("Processing Frame%d \n", frame_counter);

		

	    double focusMeasure_output; 
	  
	    focusMeasure_output=normalizedGraylevelVariance(frame_gray_crop);
	    
	    switch(methodSelect)
	           {
				   case 0: focusMeasure_output=modifiedLaplacian(frame_gray_crop);
				           break;
				           
				   case 1: focusMeasure_output=varianceOfLaplacian(frame_gray_crop);
				           break;
				           
				   case 2: focusMeasure_output=tenengrad(frame_gray_crop, 3);
				           break; 
				           
				   case 3: focusMeasure_output=normalizedGraylevelVariance(frame_gray_crop);
				           break;
				           
			    }
			    
			    	    
        focusMeasurement_vector.push_back(focusMeasure_output);
               
        
        imageGraph=showFloatGraph("Focus Measurement Realtime Measurement Chart", &focusMeasurement_vector[0], focusMeasurement_vector.size());
	    	
		       		 
        FILE *fp0;
	    char output_text_name[100]="Yan_focus_measurement.txt";

        //write to text file
        fp0=fopen(output_text_name,"at");

    
	    fprintf(fp0, "Frame:%d,", frame_counter);
	
	    fprintf(fp0, "       %.6f \n", focusMeasure_output);
	

		fclose(fp0);

		  	
		frame_counter++;
	 
		//waitKey(1);
		char key = (char)waitKey(30);
        if(key == 'q' || key == 'Q' || key == 27)
        break;
        
    }
      
    
    //showFloatGraph("Focus Measurement final Measurement Chart", &focusMeasurement_vector[0], focusMeasurement_vector.size());
    
    cvSaveImage("Focus_Measurement_Chart.png", imageGraph);
    
    cout<<""<<endl;
    cout << "Please press any key to end. Bye!)"<<endl;
    waitKey();
 
    return 0;
}
 
