WARNING: This is a work in progress. All features are not yet implemented, and the project is in
its early days.

# C++ Infrared LED Tracking program for Raspberry Pi
This program aims at developing an infrared LED tracking system using the Pi Noir camera module for Raspberry Pi.
The image analysis is done using the OpenCV library.

## Context
The program is meant to be used in a client-server application. The tracking program will extract positions from
the captured images, and relay them to the server.

## Capturing the images
I first tried to extract the positions from a video stream produced by `raspivid`, but the Raspberry was nowhere
close to be able to run an analysis program on a 1080p stream. Reducing the frequency of the stream (down to 2 fps)
caused problems to open the stream in OpenCV.

I then moved on to `raspistill`; which offered better image resolution. I quickly faced a latency problem, because
`raspistill` takes about 2 seconds to run and save the image. The workaround I found is to use `raspistill` in 
signal mode, because it allows to take pictures more frequently (around twice a second).

## Building on the Raspberry Pi
Here is the g++ line I use to build:
* ```g++ -ggdb `pkg-config --cflags opencv` -o main main.cpp `pkg-config --libs opencv` -std=c++0x```

### Building with a script
```
#!/bin/bash
echo "compiling $1"
if [[ $1 == *.c ]]
then
    gcc -ggdb `pkg-config --cflags opencv` -o `basename $1 .c` $1 `pkg-config --libs opencv`;
elif [[ $1 == *.cpp ]]
then
    g++ -ggdb `pkg-config --cflags opencv` -o `basename $1 .cpp` $1 `pkg-config --libs opencv`;
else
    echo "Please compile only .c or .cpp files"
fi
echo "Output file => ${1%.*}"
```
Found at https://help.ubuntu.com/community/OpenCV.

