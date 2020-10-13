#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace std;
using namespace cv;

#define	DVA_WIN_SZ_X	500		// The defualt diagnostic windows
#define	DVA_WIN_SZ_Y	400

#define	DVA_VID_SZ_Y	1080		// The input video is scales to fixed frame size
//#define	DVA_VID_SZ_Y	540
#define	DVA_LANE_WIDTH	3.4		// Assume uk lane is 3.4 meters wide

#define DIST(X1,Y1,X2,Y2) sqrt(((X2-X1) * (X2-X1)) + ((Y2-Y1) * (Y2-Y1)));

const cv::Scalar	COLOR_RED(0., 0., 255.);
const cv::Scalar	COLOR_GREEN(0., 255., 0.);
const cv::Scalar	COLOR_BLUIE(225., 0., 0.);

// Measure time and fps
int64 start_time, end_time;
double average_FPS = 0;

const double TICK_FREQUENCY = cv::getTickFrequency();

// Start measuring time
void startMeasuringTime() 
{
    start_time = cv::getTickCount();
}

// Stop measuring time and output FPS
void stopMeasuringTime() 
{
    end_time = cv::getTickCount();
    double time_per_frame = (double)((end_time - start_time) / TICK_FREQUENCY);
    double curr_FPS = 1. / time_per_frame;
    average_FPS = (3 * average_FPS + curr_FPS) / 4;

    std::cout << "Average FPS = " << average_FPS << "\n";
}


class LineDetect
{
	enum LineType
	{
		LT_CENTRE_LANE	= 1,		// Seperates opposing lines of traffic
		LT_LEFT_LANE	= 2,
		LT_RIGHT_LANE	= 3,
		LT_VEHICLE_DASH	= 4,
	};

public:
	LineDetect();
	LineDetect(cv::Point start, cv::Point end);
	// LineDetect(double gradient,double intersect,int length, );
	cv::Point GetStart(void);
	cv::Point GetEnd(void);
	bool operator<(const LineDetect& s2) const;
	bool operator>(const LineDetect& s2) const;
	bool operator==(const LineDetect& s2) const;
	double GetGradient(void);
	double GetIntersect(void);

	static void cart2PolarCoef(cv::Point &start, cv::Point &end, double &angle, size_t &length, double &k, double &b);
	static bool detectLane(std::vector<LineDetect> &lanes, LineDetect &newline, cv::Rect roi, unsigned int min=20, unsigned int max=200);
private:
	cv::Point pointA;
	cv::Point pointB;
	size_t length;
	double angle;
	double m;	// y = mx + b
	double b;	// y = mx + b
	int totMatch;	// count of possitive observations
	int totTest;	// total attempts to check
};

LineDetect::LineDetect()
{
}

LineDetect::LineDetect(cv::Point startPoint, cv::Point endPoint)
{
	pointA = startPoint;
	pointB = endPoint;
	cart2PolarCoef(pointA,pointB, angle, length, m, b);
	//count = 1;
}
cv::Point LineDetect::GetStart(void)
{
	return pointA;
}
cv::Point LineDetect::GetEnd(void)
{
	return pointB;
}
double LineDetect::GetGradient(void)
{
	return m;
}
double LineDetect::GetIntersect(void)
{
	return b;
}

bool LineDetect::operator==(const LineDetect& s2) const
{
	if(this->b < s2.b*0.1 && abs(this->m) < s2.m*0.1)
		if(this->b > s2.b*0.1 && abs(this->m) > s2.m*0.1)
			return 1;
	return 0;
}
bool LineDetect::operator>(const LineDetect& s2) const
{
	// y=mx+b
	// this->b must be larger
	// and/or the abs(gradient) must be steeper
	if(this->b > s2.b*0.1 && abs(this->m) > s2.m*0.1)
		return 1;
	else
		return 0;
}
bool LineDetect::operator<(const LineDetect& s2) const
{
	// y=mx+b
	// this->b must be larger
	// and/or the abs(gradient) must be steeper
	if(this->b < s2.b*0.1 && abs(this->m) < s2.m*0.1)
		return 1;
	else
		return 0;
}


void LineDetect::cart2PolarCoef(cv::Point &start, cv::Point &end, double &angle, size_t &length, double &m, double &c)
{
	// Convert from cartesian to polar
	int dx = end.x - start.x;
	int dy = end.y - start.y;

	angle = atan2f(dy, dx) * 180/CV_PI;
	length = sqrt((dx * dx) + (dy * dy));

	// calculate line coefficients: y = mx + c
	dx = (dx == 0) ? 1 : dx; // prevent DIV/0!
	m = dy/(double)dx;
	c = start.y - m*start.x;
}

//
// f(m<-0.10 && length>frame.rows*0.10 && lineStart.x < centreLineStart.x)
/*
 * 	@param std::vector<LineDetect> lanes
*/
bool LineDetect::detectLane(std::vector<LineDetect> &lines, LineDetect &newline, cv::Rect roi, unsigned int min, unsigned int max)
{
	lines.push_back(newline);

	if(lines.size()<min) {
		return false;
	}
	if(lines.size()>=max) {
		lines.erase(lines.begin());
	}

	int i = 0;
	double sum_gradient = 0.0;
	double sum_intersect = 0.0;
	for(std::vector<LineDetect>::iterator it = lines.begin(); it != lines.end(); ++it, i++) {
		sum_gradient += it->GetGradient();
		sum_intersect += it->GetIntersect();
	}
	double average_intersect = sum_intersect / i;
	double average_gradient = sum_gradient / i;

	// Y=MX+C
	// MX=Y-C
	// X=(Y-C)/M


	if(fabsf(average_gradient)<0.15) {
                // is the slope shallow?
		// TODO: Should I move the lane clasification code into here?
		newline = LineDetect(
			cv::Point( roi.x, average_intersect),
			cv::Point( roi.width, average_intersect) );
		



	} else	if(average_gradient<0) {
		double blx = roi.x;
		double bly = average_gradient*roi.x+average_intersect;


		// Negative slope (left hand lanes)
		if(bly>roi.y+roi.height) {
			blx = (roi.y+roi.height-average_intersect)/average_gradient;
			bly = roi.y+roi.height;
		}
		newline = LineDetect(
			cv::Point(   blx,  bly),
			cv::Point(	(roi.y-average_intersect)/average_gradient, 	roi.y ));
	} else {
		// possitive slop (centre lane, and far right lane)
		double blx = roi.x+roi.width;
		double bly = average_gradient*(roi.x+roi.width)+average_intersect;

		if(bly>roi.y+roi.height) {
			blx = (roi.y+roi.height-average_intersect)/average_gradient;
			bly = roi.y+roi.height;
		}
		newline = LineDetect(
			cv::Point(   blx,  bly),
			cv::Point(	(roi.y-average_intersect)/average_gradient, 	roi.y ));

	}
/*
	if(bly<roi.y) {
		blx = (roi.y-average_intersect)/average_gradient;;
		bly = roi.y+roi.height;
	}
*/

	return true;
}


void solveLinearEquation(double m1, double c1, double m2, double c2, double &x, double &y) {
	x = ( (c1-c2) / (m2-m1) ) ;
	y = ( (c1*m2 - c2*m1) / (m2-m1) ) ;
}


int main(int argc, char* argv[])
{
	
	if (argc != 2)
    {
        cout << "Call this program like this: " << endl;
        cout << "./LaneDetection_YanHaar_vehicleDetection videofile" << endl;
        return 1;
    }
	
	std::string VIDEO_FILE_NAME;
		
	VIDEO_FILE_NAME = argv[1];
	
	int showMainFrame = 0;
	int showEdgeFrame = 0;
	int showHiddenFrame = 0;
	int useLineDetection = 0;
	int useVehicleDetection = 0;
	int isVerbose = 0;
	std::vector<LineDetect> lanes;
	std::vector<LineDetect> leftlines;
	std::vector<LineDetect> centrelines;
	std::vector<LineDetect> rightlines;
	std::vector<LineDetect> dash;
	
	std::vector<LineDetect> centrelines_left;
	std::vector<LineDetect> centrelines_right;

	const char *ClasifierFile = NULL;     // arguments
	
	//ClasifierFile = "cars3.xml";
	ClasifierFile = "cars_Yan.xml";
	
	showMainFrame = 1;
	isVerbose = 1;
	showEdgeFrame = 1;
	showHiddenFrame = 1;
	useLineDetection = 1;
	useVehicleDetection = 1;
	int req_fps = 20;


	cv::VideoCapture cap(VIDEO_FILE_NAME);
	
	if (!cap.isOpened())
	{
		std::cout << "!!! Failed to open file: " << argv[1] << std::endl;
		return -1;
	}
	

	cout << "CV_VERSION:" << CV_VERSION << endl;
	cout << "CV_CAP_PROP_FRAME_WIDTH:" << cap.get(CV_CAP_PROP_FRAME_WIDTH) << endl;
	cout << "CV_CAP_PROP_FRAME_HEIGHT:" << cap.get(CV_CAP_PROP_FRAME_HEIGHT) << endl;
	cout << "CV_CAP_PROP_FPS:" << cap.get(CV_CAP_PROP_FPS) << endl;
	cout << "CV_CAP_PROP_FRAME_COUNT:" << cap.get(CV_CAP_PROP_FRAME_COUNT) << endl;

    //double scale_output = ((double)DVA_VID_SZ_Y/(double)cap.get(CV_CAP_PROP_FRAME_HEIGHT)); // scale on x axis, maintain aspect ratio

	cv::CascadeClassifier carCascade;
	if(useVehicleDetection)
	{
		carCascade.load(ClasifierFile);
	}

	if(showMainFrame)
	{
		cvNamedWindow("mainWin", 0);
		cvResizeWindow("mainWin", DVA_WIN_SZ_X, DVA_WIN_SZ_Y);
		cvMoveWindow("mainWin",0,DVA_WIN_SZ_Y);
	}

	if(showHiddenFrame)
	{
		cvNamedWindow("testWin", 0);
		cvResizeWindow("testWin", DVA_WIN_SZ_X, DVA_WIN_SZ_Y);
		cvMoveWindow("testWin",DVA_WIN_SZ_X,0);
	}

	if(showEdgeFrame)
	{
		cvNamedWindow("edgeWin", 0);
		cvResizeWindow("edgeWin", DVA_WIN_SZ_X, DVA_WIN_SZ_Y);
		cvMoveWindow("EdgeWin",DVA_WIN_SZ_X,DVA_WIN_SZ_Y);
	}

	

	cv::Mat img;
	cv::Mat frame;	// The main colour window
	cv::Mat frame_bw;   // Black and white frame for Haar image detection
	cv::Mat frame_edge; // Frame for line and edge detection

	int cap_CV_CAP_PROP_FPS = cap.get(CV_CAP_PROP_FPS);
	int fps = cap_CV_CAP_PROP_FPS;
	if(fps==0) fps=50;
	int fps_step = fps/req_fps;
	if(fps_step==0) fps_step=1;
	int FrameCols = 0;
	int FrameRows = 0;
	LineDetect dashLine(cv::Point(0,FrameRows),cv::Point(FrameCols,FrameRows));
	LineDetect leftLane;
	LineDetect centreLane;
	LineDetect rightLane;
	
	LineDetect centreLane_left;
	LineDetect centreLane_right;
	
	cv::Point roiPA;
	cv::Point roiPB;
	
	cv::Point vanishingPoint(0,0);
		

		if(isVerbose) {
			std::cout << ",POS_MSEC=" << cap.get(CV_CAP_PROP_POS_MSEC)
					<< ",CV_CAP_PROP_POS_FRAMES=" << cap.get(CV_CAP_PROP_POS_FRAMES)
					<< ",cols="<<frame.cols<<",rows="<<frame.rows
					<< ",cols="<<FrameCols<<",rows="<<FrameRows;
		}
		
    
    bool end_of_process = false;
    
	while (!end_of_process)
	{   
		
		startMeasuringTime();
				
		cap >> img;
		
		if (img.empty())
			break;
		
		// Scale image to ideally 640 x 480, but maintain the aspect ratio;
		//cv::resize(img,frame,cv::Size(854,DVA_VID_SZ_X),0,0,cv::INTER_LINEAR);
		double scale = ((double)DVA_VID_SZ_Y/(double)img.rows); // scale on x axis, maintain aspect ratio
		cv::resize(img,frame,cv::Size((int)(scale*(double)img.cols),DVA_VID_SZ_Y),0,0,cv::INTER_LINEAR);
		//int cap_frame_cols = frame.cols;
		
		// Create BW blurred frame
		cvtColor( frame, frame_bw, CV_BGR2GRAY );
		
		cv::GaussianBlur(frame_bw, frame_bw, cvSize(7,7), 0, 0, cv::BORDER_DEFAULT );
	
		// Calculate the centre line to help identify lanes left and right
		cv::Point centreLineStart(frame.cols*0.50,0);
		cv::Point centreLineEnd(frame.cols*0.50,frame.rows*1.00);

		std::vector<cv::Vec4i> lines;
	
		//cv::Canny( frame_bw, frame_edge, 30, 60, 3);
		cv::Canny( frame_bw, frame_edge, 50, 150, 3);

		cv::HoughLinesP( frame_edge, lines, 1, CV_PI/180, 80, 10, 5 );

				
		roiPA = cv::Point(frame.cols*0.30,frame.rows*0.60);
		roiPB = cv::Point(frame.cols*0.70,frame.rows*0.30);
		
				
		cv::Rect RoiRect(roiPA,roiPB);

		cv::Mat frameBwRoi=frame_bw(RoiRect);	// Black & white region of interest for car detection
		cv::Mat frameRoi=frame(RoiRect);	// Colour region of interest
		//cv::rectangle(frame, RoiRect, cvScalar(255,0,0), 2);

               
		// start line detection
		int centreLane_right_counter=0;
        int centreLane_left_counter=0;
		
		if (useLineDetection)
		{
			for( size_t i = 0; showMainFrame==1 && i < lines.size(); i++ )
			{
				cv::Point lineStart(lines[i][0],lines[i][1]);
				cv::Point lineEnd(lines[i][2],lines[i][3]);
				double angle=0;
				size_t length=0;
				double m=0.0;
				double b=0.0;
				LineDetect newline(lineStart, lineEnd);
			
				cv::Point offset;
				cv::Size wholesize;
				frameRoi.locateROI(wholesize, offset);
				cv::Point vanishingPoint_frame(vanishingPoint.x+offset.x,vanishingPoint.y+offset.y); // create point on the parent frame
//				cv::Rect CollisionRoi(roiPA,cv::Point(frame.cols*0.90,frame.rows*0.50));
				
				//cv::Rect CollisionRoi(roiPA,cv::Point(frame.cols*0.90,vanishingPoint.y*1.2)); 
				cv::Rect CollisionRoi(roiPA,cv::Point(frame.cols*0.70,vanishingPoint.y*1.1)); 
               

				if(lineStart.y < frame.rows*0.4 || lineEnd.y < frame.rows*0.4) {
					// Ignore lines at the top half of the frame
					continue;
				}

				// Given start and end point calculate the angle and line coefficients
				LineDetect::cart2PolarCoef(lineStart, lineEnd, angle, length, m, b);

				// Find a dash board if we can
				if(fabsf(m)<0.15    	                       // is the slope shallow?
					&& length>frame.cols*0.05              // is the line fairly long
					&& lineStart.y > frame.rows*0.60       // is the line at the bottom 30%
					&& lineEnd.y > frame.rows*0.60
				)
				{
					// Probably detected part of the dashboard
#if 1
					if(LineDetect::detectLane(dash, newline, CollisionRoi, 20)) {
						dashLine = newline;
					}
					//roiPA.y = dashLine.GetIntersect();
#endif				

#if 1               
                    if(isVerbose>1){
					std::cout << "Green "
							<< " x1=" << lineStart.x << " y1=" << lineStart.y
							<< " x2=" << lineEnd.x << " y2=" << lineEnd.y
							<< " angle=" << angle << " length=" << length
							<< " gradient m=" << fabs(m) << " b=" << b << std::endl;
#endif
				    }
				}
				else if(m>0.15 && length>frame.rows*0.1 && lineStart.x > centreLineStart.x)
				{
					// Detect centre lane marker - Right of centre line (YELLOW)
					// Detect lane marker - Left of centre line (WHITE)
					if(LineDetect::detectLane(centrelines_right, newline, CollisionRoi, 20)) {
						centreLane_right = newline;
						centreLane_right_counter++;
					}


					line( frame, lineStart, lineEnd, cv::Scalar(0,255,255), 3 );
					if(isVerbose>1){
						std::cout << ",Yellow"
								<< ",x1=" << lineStart.x << " y1=" << lineStart.y
								<< ",x2=" << lineEnd.x << " y2=" << lineEnd.y
								<< ",angle=" << angle << " length=" << length
								<< ",gradient m=" << m << " b=" << b;
					}

				}
				else if(m<-0.15 && length>frame.rows*0.1 && lineStart.x < centreLineStart.x)
				{
					// Detect centre lane marker - Right of centre line (YELLOW)
					// Detect lane marker - Left of centre line (WHITE)
					if(LineDetect::detectLane(centrelines_left, newline, CollisionRoi, 20)) {
						centreLane_left = newline;
						centreLane_left_counter++;
					}


					line( frame, lineStart, lineEnd, cv::Scalar(255,0,255), 3 );
					if(isVerbose>1){
						std::cout << ",Pink"
								<< ",x1=" << lineStart.x << " y1=" << lineStart.y
								<< ",x2=" << lineEnd.x << " y2=" << lineEnd.y
								<< ",angle=" << angle << " length=" << length
								<< ",gradient m=" << m << " b=" << b;
					}

				}
				else
				{
#if 0
					// everything else (BLACK)
					line( frame, lineStart, lineEnd, cv::Scalar(0,0,0), 3 );
					if(isVerbose>1){
						std::cout << "Black"
								<< " x1=" << lineStart.x << " y1=" << lineStart.y
								<< " x2=" << lineEnd.x << " y2=" << lineEnd.y
								<< " angle=" << angle << " length=" << length
								<< " gradient m=" << fabs(m) << " b=" << b << std::endl;
					}
#endif
				}
			}
		}

		
		if(centreLane_right_counter>0&&centreLane_left_counter>0)
		{
		    line( frame, centreLane_left.GetStart(), centreLane_left.GetEnd(), cv::Scalar(255,255,255), 4 );
		    line( frame, centreLane_right.GetStart(), centreLane_right.GetEnd(), cv::Scalar(255,255,255), 4 );
		}


		{
			
			// Calculate the vanishing point and draw it as a circle
			double vpx=0.0;
			double vpy=0.0;
			solveLinearEquation(centreLane_left.GetGradient(), centreLane_left.GetIntersect(), centreLane_right.GetGradient(), centreLane_right.GetIntersect(), vpx, vpy);
			
			vanishingPoint = cv::Point((int)vpx, (int)vpy);
			
			//if(vpy<0.4*frame.rows)
			if((int)vpy<0.4*frame.rows || (int) vpy>=frame.rows )
			{
				vanishingPoint = cv::Point(0.5*frame.cols, 0.4*frame.rows);
			}
			
			//cv::circle(img, vanishingPoint, 10, cv::Scalar( 255, 50, 0 ),-1);
			
		}

		
		if(useVehicleDetection)
		{
			std::vector<cv::Rect> cars;
			
			carCascade.detectMultiScale(frameBwRoi,cars,1.05, 3,CV_HAAR_DO_CANNY_PRUNING,cv::Size(30,30),cv::Size(250,250));

            std::cout << "Total: " << cars.size() << " cars detected." << std::endl;
  
            for(unsigned int i=0; i<cars.size(); i++)			
            {
                cv::rectangle(frameRoi, cars[i], COLOR_GREEN, 2);
            }
            
				  
		}
		

		if(showMainFrame)
			cv::imshow("mainWin", frame);
			
	    //cv::imshow("mainWin_grey", frame_bw);

		if(isVerbose) {
			std::cout << std::endl;
		}

		
		if(showHiddenFrame)
			//cv::imshow("testWin", frame_bw);
			cv::imshow("testWin", frameBwRoi);

		if(showEdgeFrame)
			cv::imshow("edgeWin", frame_edge);

		char key = (char)waitKey(1);
		if (key == 27)
			end_of_process = true;
			
		stopMeasuringTime();
	}
	
	return 0;
}
