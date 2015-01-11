#Webcam IP

Exemple to "Webcam IP" with UDP for Linux embedded.
I suggest you to optimize this code because it has many 
modification to improve these performances.

Developed and modified by Mras2an for the HD3000 camera.

##DEMO :
<a href="http://www.youtube.com/embed/eAylw-LBKrA
" target="_blank"><img src="http://i1.ytimg.com/vi/eAylw-LBKrA/sddefault.jpg" 
alt="WebcamIP" width="420" height="315" border="5" /></a>

This software is provided as is and it comes with no warranties
of any type. v4l original source : gdansk, UDP original source : 
igm, huffman tables : Charles M, jpeg : Thomas G, OpenCV : OpenCV doc.

LICENSE TERMS:
Redistribution and use in source and binary forms, with or 
without modification, are permitted provided that the following
conditions are met:

1\ please cite:
[Mras2an] http://mras2an.webnode.fr/ (2013) This project use: v4l driver,compression and
decompression JPEG, UDP protocol transfer, openCV image show.
Retrieved from http://www.youtube.com/watch?v=eAylw-LBKrA
and http://beagleboard.org/project/Streaming+video+BBB/

#Build :

##Dependence:
```
sudo apt-get install libopencv-dev libjpeg62-dev libturbojpeg1-dev libopencv-objdetect-dev libhighgui-dev
```

###Hote :
```
$ make
```
###Beaglebone :
you must export your toolchain if you compile on your host
```
$ make GCC_PREFIX="prefix-toolchain"
```
you can compile on Beaglebone with 
```
$ make
```
###Raspberry pi :
you must export your toolchain if you compile on your host
```
$ make GCC_PREFIX="prefix-toolchain" CFLAGS=
```
you can compile on raspberry pi with 
```
$ make CFLAGS=
```
