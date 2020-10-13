#ifndef CompressedVideo_H_
#define CompressedVideo_H_
#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#ifdef __STDINT_H
#undef __STDINT_H
#endif
#include "stdint.h"
#endif

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/mem.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
}

#include <QMessageBox>
#include "opencv2/opencv.hpp"
#include "Video.h"


class CompressedVideo : public Video{
private:
	AVFormatContext * pFormatCtx;
	AVCodecContext  * pCodecCtx ;
	AVFrame  * pFrame ;
	AVFrame  * pFrameRGB;
	uint8_t  * buffer;
	struct SwsContext * sws_ctx;
	int videoStream;
	AVPacket        packet;
	std::string file;

public:
	CompressedVideo(std::string file);
	void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame);
	bool run(IplImage * imageOrig);
	void free();

	bool init(double & width, double & heigth, IplImage * inputPicture, double & timer, Ui_InterfaceClass & ui);
	bool isAvailble();

	AVCodecContext  * getPCodexCtx(){
		return pCodecCtx;
	}
};

#endif
