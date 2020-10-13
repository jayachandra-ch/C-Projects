
//------------------------------------------------------------------------------

#define USE_HIGHGUI

#include <stdio.h>
#include <iostream>
//#include <tchar.h>

// OpenCV
#include <cv.h>
#include <cxcore.h>
#ifdef USE_HIGHGUI
	#include <highgui.h>
#endif

#ifndef UCHAR
	typedef unsigned char UCHAR;
#endif

#include "GraphUtils.h"

using namespace std;

extern bool graph_realTime;

//------------------------------------------------------------------------------
// Graphing functions
//------------------------------------------------------------------------------
const CvScalar BLACK = CV_RGB(0,0,0);
const CvScalar WHITE = CV_RGB(255,255,255);
//const CvScalar GREY = CV_RGB(150,150,150);
const CvScalar GREY = CV_RGB(255,0,0);

int countGraph = 0;	// Used by 'getGraphColor()'
CvScalar customGraphColor;
int usingCustomGraphColor = 0;


CvScalar getGraphColor(void)
{
	if (usingCustomGraphColor) {
		usingCustomGraphColor = 0;
		return customGraphColor;
	}

	countGraph++;
	switch (countGraph) {
	case 1:	return CV_RGB(60,60,255);	// light-blue
	case 2:	return CV_RGB(60,255,60);	// light-green
	case 3:	return CV_RGB(255,60,40);	// light-red
	case 4:	return CV_RGB(0,210,210);	// blue-green
	case 5:	return CV_RGB(180,210,0);	// red-green
	case 6:	return CV_RGB(210,0,180);	// red-blue
	case 7:	return CV_RGB(0,0,185);		// dark-blue
	case 8:	return CV_RGB(0,185,0);		// dark-green
	case 9:	return CV_RGB(185,0,0);		// dark-red
	default:
		countGraph = 0;	// start rotating through colors again.
		return CV_RGB(200,200,200);	// grey
	}
}
// Call 'setGraphColor()' to reset the colors that will be used for graphs.
void setGraphColor(int index)
{
	countGraph = index;
	usingCustomGraphColor = 0;	// dont use a custom color.
}
// Specify the exact color that the next graph should be drawn as.
void setCustomGraphColor(int R, int B, int G)
{
	customGraphColor = CV_RGB(R, G, B);
	usingCustomGraphColor = 1;	// show that it will be used.
}

// Draw the graph of an array of floats into imageDst or a new image, between minV & maxV if given.
// Remember to free the newly created image if imageDst is not given.
IplImage* drawFloatGraph(const float *arraySrc, int nArrayLength, IplImage *imageDst, float minV, float maxV, int width, int height, char *graphLabel, bool showScale)
{
	int w = width;
	int h = height;
	int b = 30;		// border around graph within the image
	if (w <= 20)
		//w = nArrayLength + b*2;	// width of the image
		w=500 + b*2;
		
	if (h <= 20)
		h = 220;

	int s = h - b*2;// size of graph height
	float xscale = 1.0;
	if (nArrayLength > 1)
		xscale = (w - b*2) / (float)(nArrayLength-1);	// horizontal scale
	IplImage *imageGraph;	// output image

	// Get the desired image to draw into.
	if (!imageDst) {
		// Create an RGB image for graphing the data
		imageGraph = cvCreateImage(cvSize(w,h), 8, 3);

		// Clear the image
		cvSet(imageGraph, WHITE);
	}
	else {
		// Draw onto the given image.
		imageGraph = imageDst;
	}
	if (!imageGraph) {
		cout << "ERROR in drawFloatGraph(): Couldn't create image of " << w << " x " << h << endl;
		exit(1);
	}
	CvScalar colorGraph = getGraphColor();	// use a different color each time.

	// If the user didnt supply min & mav values, find them from the data, so we can draw it at full scale.
	if (fabs(minV) < 0.0000001f && fabs(maxV) < 0.0000001f) {
		for (int i=0; i<nArrayLength; i++) {
			float v = (float)arraySrc[i];
			if (v < minV)
				minV = v;
			if (v > maxV)
				maxV = v;
		}
	}
	float diffV = maxV - minV;
	if (diffV == 0)
		diffV = 0.00000001f;	// Stop a divide-by-zero error
	float fscale = (float)s / diffV;

	// Draw the horizontal & vertical axis
	int y0 = cvRound(minV*fscale);
	cvLine(imageGraph, cvPoint(b,h-(b-y0)), cvPoint(w-b, h-(b-y0)), BLACK);
	cvLine(imageGraph, cvPoint(b,h-(b)), cvPoint(b, h-(b+s)), BLACK);

	// Write the scale of the y axis
	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_PLAIN,1.2,1.2, 0, 1,CV_AA);	// For OpenCV 1.1
	
	if (showScale) {
		//cvInitFont(&font,CV_FONT_HERSHEY_PLAIN,0.5,0.6, 0,1, CV_AA);	// For OpenCV 2.0
		CvScalar clr = GREY;
		char text[16];
        snprintf(text, sizeof(text)-1, "%.2f", maxV);
		cvPutText(imageGraph, text, cvPoint(1, b-4), &font, clr);
		// Write the scale of the x axis
        snprintf(text, sizeof(text)-1, "%d", (int)(nArrayLength-1) );
		//cvPutText(imageGraph, text, cvPoint(w-b+4-5*strlen(text), (h/2)+10), &font, clr);
		cvPutText(imageGraph, text, cvPoint(w-b+4-5*strlen(text), h-10), &font, clr);
	}

	// Draw the values
	CvPoint ptPrev = cvPoint(b,h-(b-y0));	// Start the lines at the 1st point.
	//float newValue,prevValue = 0.0001;		//For taking data values

	
	for (int i=0; i<nArrayLength; i++) 
	{
		int y = cvRound((arraySrc[i] - minV) * fscale);	// Get the values at a bigger scale
		int x = cvRound(i * xscale);
		CvPoint ptNew = cvPoint(b+x, h-(b+y));
		
		cvLine(imageGraph, ptPrev, ptNew, colorGraph, 1, CV_AA);	// Draw a line from the previous point to the new point
		ptPrev = ptNew;
	}
	
	//if(graph_realTime)
	{
		 char text[16];
		 snprintf(text, sizeof(text)-1, "%.2f", (float) arraySrc[nArrayLength-1]);
		 
		 if(nArrayLength!=1)
		 cvPutText(imageGraph, text, cvPoint(ptPrev.x-25, ptPrev.y), &font, CV_RGB(0,0,0));
    }

	// Write the graph label, if desired
	if (graphLabel != NULL && strlen(graphLabel) > 0) {
		//cvInitFont(&font,CV_FONT_HERSHEY_PLAIN, 0.5,0.7, 0,1,CV_AA);
		cvPutText(imageGraph, graphLabel, cvPoint(30, 10), &font, CV_RGB(0,0,0));	// black text
	}

	return imageGraph;
}

// Display a graph of the given int array. If background is provided, it will be drawn into
IplImage* showFloatGraph(const char *name, const float *arraySrc, int nArrayLength, int delay_ms, IplImage *background)
{
#ifdef USE_HIGHGUI
	// Draw the graph
	IplImage *imageGraph = drawFloatGraph(arraySrc, nArrayLength, background);

	// Display the graph into a window
    cvNamedWindow( name );
    cvShowImage( name, imageGraph );    

	cvWaitKey( 10 );		// Note that cvWaitKey() is required for the OpenCV window to show!
	cvWaitKey( delay_ms );	// Wait longer to make sure the user has seen the graph

	//cvReleaseImage(&imageGraph);
	
	return imageGraph;
#endif
}

