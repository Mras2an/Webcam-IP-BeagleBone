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
#include "ServUDP.h"

int main(int argc, char** argv)
{ 
    int portLocal;
    sigset_t mask;
    size_t tailleAd;
    struct sockaddr_in adClient; 
    struct sigaction actQuit;
    unsigned char buffer[SIZE_BUFFER];
    char key;
    int rc;
    int err_jpeg =0;

    unsigned long jpg_size = JPEG_SIZE;
    struct jpeg_decompress_struct cinfo;                    
    struct jpeg_error_mgr jerr;
    Mat imageCam2;
    Mat imageCam(240,320, CV_8UC3);
    if (argc != 2) 
    {
        fprintf(stderr, "Syntaxe incorrecte: %s NumPort\n", argv[0]);
        return -1;
    }

    sigemptyset(&mask);             
    actQuit.sa_mask = mask;
    actQuit.sa_handler = handlerQuitte;   
    sigaction(SIGINT, &actQuit, NULL);   
    sigaction(SIGQUIT, &actQuit, NULL);  
    sigaction(SIGHUP, &actQuit, NULL);   
    portLocal = atoi(argv[1]);
    skDesc = ouvreSocket(portLocal);
    if (skDesc == -1)
    {
        perror("ouvreSocket(portLocal) erreur");   
        return -1;
    }
    tailleAd = sizeof(adClient);
    printf("### Serveur beagleboard : %d ###\n", portLocal);

    unsigned char * Image = (unsigned char *)malloc(sizeof(char *) * 614400); 
    while (key != 'Q' && key != 'q')
    {
        if (buffer[0] == 0xFF && buffer[1] == 0xD8 && buffer[2] == 0xFF)
        {
            for (int t = 0; t < 10/2 ; t++)
            {
                if (t == 9/2)
                {
                    memcpy(Image, buffer, sizeof(char) * SIZE_UDP_END);
                    Image -= SIZE_BUFF_UDP * t;
                }
                else
                {
                    memcpy(Image, buffer, sizeof(char) * SIZE_BUFF_UDP);
                    Image += SIZE_BUFF_UDP;
                }
                retVal = recvfrom(skDesc, &buffer, sizeof(char) * SIZE_BUFF_UDP, 0, 
                        (struct sockaddr*) &adClient, (socklen_t *)(&tailleAd));
                if (retVal == -1)
                {
                    perror("recvfrom");
                    close(skDesc);
                    return -1;
                }

            }
            cinfo.err = jpeg_std_error(&jerr);	
            jpeg_create_decompress(&cinfo);

            jpeg_mem_src(&cinfo,(unsigned char *)Image, jpg_size);
            rc = jpeg_read_header(&cinfo, TRUE);
            if (rc != 1) 
            {
                //exit(EXIT_FAILURE);
            }

            jpeg_start_decompress(&cinfo);
            int width = cinfo.output_width;
            int height = cinfo.output_height;
            int pixel_size = cinfo.output_components;

            int bmp_size = width * height * pixel_size;
            bmp_size = width * height * pixel_size;
            bmp_buffer = (unsigned char*)malloc(bmp_size);
            int row_stride = width * pixel_size;
            while (cinfo.output_scanline < cinfo.output_height) 
            {
                unsigned char *buffer_array[1];
                buffer_array[0] = bmp_buffer + \
                                  (cinfo.output_scanline) * row_stride;
               err_jpeg = jpeg_read_scanlines(&cinfo, buffer_array, 1);
            }
            jpeg_finish_decompress(&cinfo);
            jpeg_destroy_decompress(&cinfo);
            
            memcpy(imageCam.data, bmp_buffer, bmp_size);
            cvtColor(imageCam,imageCam,CV_BGR2RGB); 
            resize(imageCam, imageCam2, Size(imageCam.cols * 2, imageCam.rows * 2), 0, 0,  INTER_LINEAR);
            imshow("Display", imageCam2);        
            key = cvWaitKey(1);
        }
        else{
            retVal = recvfrom(skDesc, &buffer, sizeof(char) * SIZE_BUFF_UDP, 0, 
                    (struct sockaddr*) &adClient, (socklen_t *)(&tailleAd));
            if (retVal == -1)
            {
                perror("recvfrom");
                close(skDesc);
                return -1;
            }
        }
    }
    return 0;
}

void handlerQuitte(int numSig)
{
    printf("\nDisconect : Done\n");
    close(skDesc);
    cvDestroyWindow("Display");
    exit (0);
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
        perror("ErreOr socket\n");   
        return -1;
    }   
    tailleAd = sizeof(adLocale);
    retVal = bind(skD, (struct sockaddr*) &adLocale, tailleAd); 
    if (retVal == -1)
    {
        perror("Erreor bind\n");
        close(skD);    
        return -1;
    }
    return skD;
}
