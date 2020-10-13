#ifndef PROCESS_H_
#define PROCESS_H_
#include Runable.h
class Process{
private:
	std::vector<Runable> lstRunable;
public:
	void attach (Runable runable);
	void remove (Renable runable);
	void run(IplImage * imageOrig,IplImage * workingImage);
}
#endif