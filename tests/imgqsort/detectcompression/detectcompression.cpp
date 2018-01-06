/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 28-07-2013
 * Description : Detects compression by analyzing the intensity of blocks
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// OpenCV includes

#include "libopencv.h"

// C++ includes

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace cv;
using namespace std;

// Global variable
Mat src, src_gray;
#define block_size 8

//TODO: should calibrate the THRESHOLD value
#define THRESHOLD 147

/* @function main */
int main( int /*argc*/, char** argv )
{
    int countblocks      = 0;
    int number_of_blocks = 0;
    int sum              = 0;
    vector<int> average_bottom, average_middle, average_top;

    // Load an image
    src = imread( argv[1] );

    if ( !src.data )
    {
        return -1;
    }

    // Convert the image to grayscale
    cvtColor( src, src_gray, CV_BGR2GRAY );

    //go through 8 blocks at a time horizontally
    //iterating through columns
    for (int i = 0; i < src_gray.rows; i++)
    {
        //calculating intensity of top column
        for (int j = 0; j < src_gray.cols; j+=8)
        {
            sum=0;

            for (int k=j; k<block_size; k++)
            {
                sum += (int)src_gray.at<uchar>(i, j);
            }

            average_top.push_back(sum/8);
        }

        //calculating intensity of middle column
        for (int j = 0; j < src_gray.cols; j+=8)
        {
            sum=0;

            for (int k = j; k < block_size; k++)
            {
                sum += (int)src_gray.at<uchar>(i+1, j);
            }

            average_middle.push_back(sum/8);
        }

        //calculating intensity of bottom column
        countblocks=0;

        for (int j = 0; j < src_gray.cols; j+=8)
        {
            sum=0;

            for (int k = j; k < block_size; k++)
            {
                sum += (int)src_gray.at<uchar>(i+2, j);
            }

            average_bottom.push_back(sum/8);
            countblocks++;
        }

        //check if the average intensity of 8 blocks in the top, middle and bottom rows are equal. If so increment number_of_blocks
        for (int j = 0; j < countblocks; j++)
        {
            if ((average_middle[j] == (average_top[j]+average_bottom[j])/2) && average_middle[j] > THRESHOLD)
            {
                number_of_blocks++;
            }
        }
    }

    average_bottom.clear();
    average_middle.clear();
    average_top.clear();

    //iterating through rows

    for (int j= 0; j < src_gray.cols; j++)
    {
        //calculating intensity of top row
        for (int i = 0; i < src_gray.rows; i+=8)
        {
            sum=0;

            for (int k=i; k<block_size; k++)
            {
                sum += (int)src_gray.at<uchar>(i, j);
            }

            average_top.push_back(sum/8);
        }

        //calculating intensity of middle row
        for (int i = 0; i < src_gray.rows; i+=8)
        {
            sum=0;

            for (int k = i; k < block_size; k++)
            {
                sum += (int)src_gray.at<uchar>(i, j+1);
            }

            average_middle.push_back(sum/8);
        }

        //calculating intensity of bottom row
        countblocks=0;

        for (int i = 0; i< src_gray.rows; i+=8)
        {
            sum=0;

            for (int k=i; k<block_size; k++)
            {
                sum += (int)src_gray.at<uchar>(i, j+2);
            }

            average_bottom.push_back(sum/8);
            countblocks++;
        }

        //check if the average intensity of 8 blocks in the top, middle and bottom rows are equal. If so increment number_of_blocks
        for (int i = 0; i < countblocks; i++)
        {
            if ((average_middle[i] == (average_top[i]+average_bottom[i])/2) && average_middle[i] > THRESHOLD)
            {
                number_of_blocks++;
            }
        }
    }

    std::cout << "Number of blocks: " << number_of_blocks << "\t\t";

    return 0;
}
