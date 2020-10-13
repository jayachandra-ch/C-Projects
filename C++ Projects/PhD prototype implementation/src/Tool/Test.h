#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class Test{
private:
	IplImage * img1;
	IplImage * img2;
public:
	Test(IplImage * img1);
};