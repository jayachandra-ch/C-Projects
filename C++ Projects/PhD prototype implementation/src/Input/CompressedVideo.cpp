#include "../../lib/Input/CompressedVideo.h"

CompressedVideo::CompressedVideo(std::string fileStr){
	CompressedVideo::file=fileStr;
	const char * file = fileStr.c_str();
	pFormatCtx = NULL;
	int i;
	pCodecCtx = NULL;
	AVCodec *pCodec = NULL;
	pFrame = NULL; 
	pFrameRGB = NULL;

	int numBytes;
	buffer = NULL;

	AVDictionary    *optionsDict = NULL;
	sws_ctx = NULL;

	// Register all formats and codecs
	av_register_all();

	// Open video file
	if(avformat_open_input(&pFormatCtx, file, NULL, NULL)!=0){
		QMessageBox(QMessageBox::Warning,"Error","Could not open the file",QMessageBox::Ok).exec();
		return;
	}

	// Retrieve stream information
	if(avformat_find_stream_info(pFormatCtx, NULL)<0){
		QMessageBox(QMessageBox::Warning,"Error","Couldn't find stream information",QMessageBox::Ok).exec();
		return;
	}

	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, file, 0);

	// Find the first video stream
	videoStream=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++){
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
			videoStream=i;
			break;
		}
	}
	if(videoStream==-1){
		QMessageBox(QMessageBox::Warning,"Error","Didn't find a video stream",QMessageBox::Ok).exec();
		return;
	}

	// Get a pointer to the codec context for the video stream
	pCodecCtx=pFormatCtx->streams[videoStream]->codec;

	// Find the decoder for the video stream
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL){
		QMessageBox(QMessageBox::Warning,"Error","Unsupported codec",QMessageBox::Ok).exec();
		return;
	}
	// Open codec
	if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0){
		QMessageBox(QMessageBox::Warning,"Error","Could not open codec",QMessageBox::Ok).exec();
		return;
	}

	// Allocate video frame
	pFrame=av_frame_alloc();

	// Allocate an AVFrame structure
	pFrameRGB=av_frame_alloc();
	if(pFrameRGB==NULL){
		QMessageBox(QMessageBox::Warning,"Error","Could not allocate frame",QMessageBox::Ok).exec();
		return;
	}
	// Determine required buffer size and allocate buffer
	numBytes=avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width,pCodecCtx->height);
	buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

	sws_ctx =
		sws_getContext
		(
		pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width,
		pCodecCtx->height,
		//PIX_FMT_BGR24,
		PIX_FMT_YUV420P,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
		);

	// Assign appropriate parts of buffer to image planes in pFrameRGB
	// Note that pFrameRGB is an AVFrame, but AVFrame is a superset
	// of AVPicture
	avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_YUV420P/*PIX_FMT_RGB24*/, pCodecCtx->width, pCodecCtx->height);
}

bool CompressedVideo::init(double & width, double & height, IplImage * inputPicture, double & timer, Ui_InterfaceClass & ui){
	width=pCodecCtx->width;
	height=pCodecCtx->height;
	*inputPicture=*cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,3);
	cv::VideoCapture stream = cv::VideoCapture(file);
	timer = stream.get(CV_CAP_PROP_FPS);
	stream.release();
	updateInterface(ui);
	return true;
}

bool CompressedVideo::isAvailble(){return true;}

bool CompressedVideo::run(IplImage * imageOrig){
	
	int frameFinished;
	bool frameIsOk=false;
	// Read frames
	while(!frameIsOk){
		if(av_read_frame(pFormatCtx, &packet)>=0) {
			// Is this a packet from the video stream?
			if(packet.stream_index==videoStream) {
				// Decode video frame
				
				frameIsOk=true;
				avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, 
					&packet);
				if(frameFinished) {
					
					sws_scale
						(
						sws_ctx,
						(uint8_t const * const *)pFrame->data,
						pFrame->linesize,
						0,
						pCodecCtx->height,
						pFrameRGB->data,
						pFrameRGB->linesize
						);
					cv::Mat img(pFrame->height,pFrame->width,CV_8UC1,pFrameRGB->data[0]); 
					//imshow("img",img);
					//cv::waitKey();
					cvtColor(img, img, CV_YUV2BGR);
					uint8_t* pixelPtr = (uint8_t*)img.data;
					int cn = img.channels();
					for(int i=0;i<img.rows;i++){
						for(int j=0;j<img.cols;j++){
							imageOrig->imageData[i*img.cols*cn + j*cn + 0] = pixelPtr[i*img.cols*cn + j*cn + 0]; // B
							imageOrig->imageData[i*img.cols*cn + j*cn + 1] = pixelPtr[i*img.cols*cn + j*cn + 1]; // G
							imageOrig->imageData[i*img.cols*cn + j*cn + 2] = pixelPtr[i*img.cols*cn + j*cn + 2]; // R
							//imageOrig->imageData[i*img.cols*cn + j*cn] = pixelPtr[i*img.cols*cn + j*cn]; 
						}
					}
				}			
			}			
		}else{
			free();
			return false;
		}
	}
	av_free_packet(&packet);
	return true;
}

void CompressedVideo::free(){
	// Free the RGB image
	av_free(buffer);
	av_free(pFrameRGB);

	// Free the YUV frame
	av_free(pFrame);

	// Close the codec
	avcodec_close(pCodecCtx);

	// Close the video file
	avformat_close_input(&pFormatCtx);
}