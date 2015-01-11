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
#include "camIpCli.h"


static int xioctl(int fd, int request, void* arg)
{
    int status;
    do 
    { 
        status = ioctl(fd, request, arg); 
    } 
    while (-1 == status && EINTR == errno);
    return status;
}

int main(int argc, char** argv)
{ 
    int portServeur;
    sigset_t mask;
    size_t tailleAd;
    struct hostent* infosServeur = NULL;
    struct sigaction actQuit;
    struct sockaddr_in adServeur; 
    const char* VideoDeviceName = VideoDeviceName0; 
    int  status;

    if (argc != 3)
    {
        fprintf(stderr, "Syntaxe incorrecte: %s NomMachine NumPort du serveur\n", argv[0]);
        return -1;
    }

    sigemptyset(&mask);               
    actQuit.sa_mask = mask;
    actQuit.sa_handler = handlerQuitte;   
    sigaction(SIGINT, &actQuit, NULL);   
    sigaction(SIGQUIT, &actQuit, NULL); 
    sigaction(SIGHUP, &actQuit, NULL);  
    portServeur = atoi(argv[2]);           
    infosServeur = gethostbyname(argv[1]);  
    if (infosServeur == NULL)
    {
        perror("Erreur lors de la récupération des informations serveur\n");
        close(skDesc);
        return -1;
    }  
    adServeur.sin_family = infosServeur->h_addrtype;    
    adServeur.sin_port = htons(portServeur);            
    memcpy(&adServeur.sin_addr, infosServeur->h_addr, infosServeur->h_length);

    printf("\n\t***Beagleboard***\n\n");
    printf("Name: %s\n", infosServeur->h_name); 
    printf("Ip addr: %s\n", inet_ntoa(adServeur.sin_addr));
    printf("Send UDP\n");

    skDesc = ouvreSocket(PORTLOCAL);
    if (skDesc == -1)
    {
        perror("ouvreSocket(PORTLOCAL) erreur ");
        return -1;
    }
    tailleAd = sizeof(adServeur); 

    int webcam = open(VideoDeviceName, O_RDWR|O_NONBLOCK, 0);
    if (-1 == webcam)
    {
        fprintf(stderr, "ERROR: cannot open the device %s\n", VideoDeviceName);
        exit(EXIT_FAILURE);
    }

    struct v4l2_capability cap;
    status = xioctl(webcam, VIDIOC_QUERYCAP, &cap);
    if (0 != status)
    {
        fprintf(stderr, "ERROR: ioctl VIDIOC_QUERYCAP returned status of %d\n", status);
    }
    if ( !(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) )
    {
        fprintf(stderr, "ERROR: the device does not support image capture\n");
        exit(EXIT_FAILURE);
    }
    if ( !(cap.capabilities & V4L2_CAP_STREAMING) )
    {
        fprintf(stderr, "WARNING: the device does not support streaming\n");
    }

    struct v4l2_cropcap cropcap;
    memset(&cropcap, 0, sizeof(cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    status = xioctl (webcam, VIDIOC_CROPCAP, &cropcap);
    if (0 == status)
    {
        struct v4l2_crop crop;
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect;

        status = xioctl(webcam, VIDIOC_S_CROP, &crop);
        if (0 != status)
        {
            if (EINVAL==errno)
                fprintf(stderr, "WARNING: the device does not support cropping\n");
            else ;
        }
    } 
    else 
    {        
        // Errors ignored
    }

    struct v4l2_format form;
    memset(&form, 0, sizeof(form)); 
    form.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    form.fmt.pix.width       = 320; 
    form.fmt.pix.height      = 240;
    form.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    form.fmt.pix.field       = V4L2_FIELD_ANY;
    status = xioctl(webcam, VIDIOC_S_FMT, &form);
    if (0 != status)
    {
        fprintf(stderr, "ERROR: ioctl VIDIOC_S_FMT returned status of %d (format %d is not supported)\n", 
                status, (int)form.fmt.pix.pixelformat);
        exit(EXIT_FAILURE);
    }

    int min = form.fmt.pix.width * 2;
    if (form.fmt.pix.bytesperline < min)
        form.fmt.pix.bytesperline = min;
    min = form.fmt.pix.bytesperline * form.fmt.pix.height;
    if (form.fmt.pix.sizeimage < min)
        form.fmt.pix.sizeimage = min;

    if (cap.capabilities & V4L2_CAP_STREAMING)
    {
        struct buffer* buffers   = NULL;
        unsigned int   n_buffers = 0;
        unsigned int i;

        struct v4l2_requestbuffers req;
        memset(&req, 0, sizeof(req));
        req.count               = 4;
        req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory              = V4L2_MEMORY_MMAP;

        status = xioctl(webcam, VIDIOC_REQBUFS, &req);
        if (0 != status)
        {
            fprintf(stderr, "ERROR: ioctl VIDIOC_REQBUFS returned status %d\n", status);
            exit (EXIT_FAILURE);
        }
        if (req.count < 2) 
        {
            fprintf (stderr, "ERROR: Insufficient buffer memory on %s\n", VideoDeviceName);
            exit (EXIT_FAILURE);
        }
        n_buffers = req.count;
        buffers = (struct buffer *)calloc(req.count, sizeof(*buffers));
        if (0 == buffers) 
        {
            fprintf (stderr, "ERROR: Out of memory\n");
            exit (EXIT_FAILURE);
        }

        for (i = 0; i < n_buffers; ++i) 
	{
            struct v4l2_buffer buf;
            memset(&buf, 0, sizeof(buf));
            buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory      = V4L2_MEMORY_MMAP;
            buf.index       = i;
            status = xioctl(webcam, VIDIOC_QUERYBUF, &buf);
            if (0 != status)
            {
                fprintf(stderr, "ERROR: ioctl VIDIOC_QUERYBUF for buffer %d returned status %d\n", i, status);
                exit (EXIT_FAILURE);
            }

            buffers[i].length = buf.length;
            buffers[i].start = mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, webcam, buf.m.offset);
            if (MAP_FAILED == buffers[i].start)
            {
                fprintf(stderr, "ERROR: MemoryMap failed for buffer %d of %d\n", i, n_buffers);
                exit (EXIT_FAILURE);
            }
	} 
        for (i = 0; i < n_buffers; ++i)
        {
            struct v4l2_buffer buf;
            memset(&buf, 0, sizeof(buf));
            buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory      = V4L2_MEMORY_MMAP;
            buf.index       = i;

            status = xioctl (webcam, VIDIOC_QBUF, &buf);
            if (0 != status)
            {
                fprintf(stderr, "ERROR: ioctl VIDIOC_QBUF returned status %d\n", status);
                exit (EXIT_FAILURE);
            }
        }

        enum v4l2_buf_type type1;    
        type1 = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        status = xioctl (webcam, VIDIOC_STREAMON, &type1);
        if (0 != status)
        {
            fprintf(stderr, "ERROR: ioctl VIDIOC_STREAMON returned status %d\n", status);
            exit (EXIT_FAILURE);
        }
        while (1)
        {
            struct timeval timeout;
            timeout.tv_sec = 2;
            timeout.tv_usec = 0;

            fd_set fds;
            FD_ZERO (&fds);
            FD_SET (webcam, &fds);
            status = select (webcam+1, &fds, NULL, NULL, &timeout);

            if (-1 == status)
            {
		    if (EINTR==errno)
		    {
			    continue;
		    }
		    fprintf (stderr, "ERROR: select failed\n");
		    exit (EXIT_FAILURE);
	    }
	    if (0 == status)
	    {
		    fprintf (stderr, "ERROR: webcam tiemout\n");
		    exit (EXIT_FAILURE);
	    }

	    struct v4l2_buffer buf;
	    memset(&buf, 0, sizeof(buf));
	    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	    buf.memory = V4L2_MEMORY_MMAP;
	    status = xioctl(webcam, VIDIOC_DQBUF, &buf);
	    if (0 != status)
	    {
		    if (EAGAIN == errno) 
		    {
			    continue;
		    } 
		    else
		    {
			    fprintf(stderr, "ERROR: ioctl VIDIOC_DQBUF returned status %d\n", status);
			    exit(EXIT_FAILURE);
		    }
	    }

	    assert(buf.index < n_buffers);

	    char* buffer = (char *)buffers[buf.index].start;
	    int bytes_read = buffers[buf.index].length;
	    int size = bytes_read;
	    int i;
	    for (i = 0; i < size; i++)
	    {
		    if (((unsigned char)buffer[i] == 0xFF) && ((unsigned char)buffer[i + 1] == 0xC0))
		    {
			    break;
		    }
	    }

	    cameraFrame = (char *)realloc(cameraFrame, sizeof(char) * size + huffmanTablesSize);

	    int offset = i + 19;
	    memcpy(cameraFrame, buffer, offset);
	    memcpy(cameraFrame + offset, huffmanTables, huffmanTablesSize);
	    offset += huffmanTablesSize;
	    memcpy(cameraFrame + offset, buffer + i + 19, size - i - 19);
	    cameraFrameSize = size + huffmanTablesSize;
	    int j; 
	    for (j = 0; j < ((cameraFrameSize/TAILLE) + 1); j++)
	    {
		    if (j == cameraFrameSize/TAILLE)
		    {
			    retVal = sendto(skDesc, cameraFrame, sizeof(char) * (cameraFrameSize - (TAILLE * j)), 0, (struct sockaddr*) &adServeur, tailleAd);
			    if (retVal == -1)
			    {
				    perror("sendto");
				    close(skDesc);
			    }
			    cameraFrame = cameraFrame - (int)((j) * TAILLE); 
		    }
		    else
		    {
			    retVal = sendto(skDesc, cameraFrame, sizeof(char) * TAILLE, 0, (struct sockaddr*) &adServeur, tailleAd);
			    if (retVal == -1)
			    {
				    perror("sendto");
				    close(skDesc);
			    }
			    cameraFrame += TAILLE;
		    }
	    }
	    status = xioctl (webcam, VIDIOC_QBUF, &buf);
	    if (0!=status)
	    {
		    fprintf(stderr, "ERROR: ioctl VIDIOC_QBUF returned status %d\n", status);
	    }
	}

	enum v4l2_buf_type type2;
	type2 = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	status = xioctl(webcam, VIDIOC_STREAMOFF, &type2);
	if (0 != status)
	{
		fprintf(stderr, "ERROR: ioctl VIDIOC_STREAMON returned status %d\n", status);
		exit (EXIT_FAILURE);
	}
	for (i = 0; i < n_buffers; ++i)
		status = munmap(buffers[i].start, buffers[i].length);
	if (0 != status)
	{
		fprintf(stderr, "ERROR: MemoryMap unmapping failed\n");
		exit (EXIT_FAILURE);
	}
    }

    close(webcam);
    close(skDesc);

    return 0;
}

void handlerQuitte(int numSig)
{
	printf("\nDisconect\n");
	close(skDesc);
	exit(0);
}

int ouvreSocket(int port)
{
	int skD;
	size_t tailleAd;
	struct sockaddr_in adLocale;

	adLocale.sin_family = AF_INET;                 
	adLocale.sin_port = htons(port);                
	adLocale.sin_addr.s_addr = htonl(INADDR_ANY);    
	skD = socket(AF_INET, SOCK_DGRAM, 0);       
	if (skD == -1)
	{
		perror("Erreor socket\n");   
		return -1;
	}
	tailleAd = sizeof(adLocale);
	retVal = bind(skD, (struct sockaddr*)&adLocale, tailleAd); 
	if (retVal == -1)
	{
		perror("Erreor bind\n");
		close(skD);  
		return -1;
	}
	return skD;
}
