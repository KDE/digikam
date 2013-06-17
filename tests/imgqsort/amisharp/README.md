AmISharp

Program to check whether an image is blurry or not. [Alpha Stage]


This program finds edges in a picture. It does so by using the fact that edges usually show a sharp change in pixel intensity.
Then, it returns the maximum value of the 
We first remove noise, by applying Gaussian filter.
Then, we convert the image to grayscale, as we don't need colors in this case.
To ensure normalize the contrast, we use equalizeHist();

The laplacian operator is applied on the image with takes the second derivative of the intensities of each pixel.
This is scaled by using convertScaleAbs();
We attempt to find if blurriness by getting the average intensity value after the laplacian operator has been applied.


The threshold value to say, whether the image is blurry or not, is determined experimentally.

To build this, 
Create a directory, Go to that directory in this folder, Type "cmake .." , then make it.

Make sure you have openCV Libraries on your computer.
