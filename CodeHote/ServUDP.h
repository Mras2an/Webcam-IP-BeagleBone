/******************************************************************                                              
*
* Exemple to "Webcam IP" with UDP for Linux embedded.
* I suggest you to optimize this code because it has many 
* modification to improve these performances.
*
* Developed and modified by Mras2an for the HD3000 camera.
* 
* This software is provided as is and it comes with no warranties
* of any type. v4l original source : gdansk, UDP original source : 
* igm, jpeg : Thomas G, OpenCV : OpenCV doc.
*
* LICENSE TERMS:
* Redistribution and use in source and binary forms, with or 
* without modification, are permitted provided that the following
* conditions are met:
* 1\ please cite:
* [Mras2an] (2013) This project use: v4l driver,compression and
* decompression JPEG, UDP protocol transfer, openCV image show.
* Retrieved from http://www.youtube.com/watch?v=eAylw-LBKrA
* and http://beagleboard.org/project/Streaming+video+BBB/
*
******************************************************************/

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h> 
#include <assert.h>
#include <syslog.h>
#include <opencv2/core/core.hpp>                                   
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <jpeglib.h>

#define SIZE_BUFF_UDP   65000/2
#define SIZE_BUFFER     70000/2
#define SIZE_IMG        614400/2
#define TJFLAG_FASTDCT  2048
#define JPEG_SIZE       724306/2
#define SIZE_UDP_END    8184

void handlerQuitte(int numSig);
int ouvreSocket(int port);

using namespace cv;
using namespace std;

unsigned char *bmp_buffer;
int retVal;
int skDesc;
const static size_t JPEG_BUF_SIZE = 16384;

