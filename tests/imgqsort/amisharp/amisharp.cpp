/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2013-06-17
 * @brief  a command line tool to estimate amount of  blur in an image.
 *
 * @author Copyright (C) 2013-2014 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *         Copyright (C) 2013-2014 by Gowtham Ashok
 *         <a href="mailto:gwty93 at gmail dot com">gwty93 at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */
 
 #include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
#include<iostream>

using namespace cv;

int main( int argc, char** argv )
{
    Mat src, src_gray, dst;
    int kernel_size = 3;
    int scale = 1;
    int delta = 0;
    int ddepth = CV_16S;
    int c;
    // Load an image
    src = imread( argv[1] );

    //for demonstration
    namedWindow("Demonstration");
    imshow("Demonstration",src);
    std::cout<<"Original Image"<<std::endl;
    waitKey(0);


    if( !src.data )
    {
        return -1;
    }

    // Remove noise by blurring with a Gaussian filter
    GaussianBlur( src, src, Size(3,3), 0, 0, BORDER_DEFAULT );
    std::cout<<"Image after Removing noise"<<std::endl;

    imshow("Demonstration",src);
    waitKey(0);

    // Convert the image to grayscale
    cvtColor( src, src_gray, CV_RGB2GRAY );
    std::cout<<"Grayscale Image"<<std::endl;

    imshow("Demonstration",src_gray);
    waitKey(0);

    // equalizing Histogram to enhance contrast
    equalizeHist(src_gray, src_gray );
    std::cout<<"Image after enhancing  contrast"<<std::endl;

    imshow("Demonstration",src_gray);
    waitKey(0);

    // Apply Laplace function
    Mat abs_dst;
    Laplacian( src_gray, dst, ddepth, kernel_size, scale, delta, BORDER_DEFAULT );
    int x,y;
    convertScaleAbs( dst, abs_dst );
    std::cout<<"Image after applying Laplace Function"<<std::endl;

    imshow("Demonstration",abs_dst);
    waitKey(0);

    /*
    for( y=0; y<abs_dst.rows; y++ )
    {
        const short* dsttemp = abs_dst.ptr<short>(y);
        for(x=0; x<abs_dst.cols;x++ )
        {
                count++;
                value+=dsttemp[x];
        }
    }
    */
    double average=mean(abs_dst)[0];
    double maxval;
    int *maxIdx=(int* )malloc(sizeof(abs_dst));
    minMaxIdx(abs_dst, 0, &maxval, 0, maxIdx);

    double blurresult=average/maxval;
    std::cout<<"The average of the edge intensity is "<<average<<std::endl;
    std::cout<<"The maximum of the edge intensity is "<<maxval<<std::endl;
    std::cout<<"The result of the edge intensity is "<<blurresult<<std::endl;

    return 0;
}
