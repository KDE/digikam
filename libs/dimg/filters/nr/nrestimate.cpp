/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-18
 * Description : Wavelets YCrCb Noise Reduction settings estimation.
 *
 * Copyright (C) 2012 by Sayantan Datta <sayantan dot knz at gmail dot com>
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C++ includes

#include <cmath>
#include <cfloat>
#include <iostream>

// Qt includes.

#include <QTextStream>
#include <QFile>

// Kde include

#include <kdebug.h>

// Local includes

#include "nrestimate.h"
#include "nrfilter.h"
#include "libopencv.h"

namespace Digikam
{

class NREstimate::Private
{
public:

    Private():
       clusterCount(30)
    {
        for (int c = 0 ; c < 3; c++)
        {
            fimg[c] = 0;
        }
    }

    DImg      img;

    QString   path;

    float*    fimg[3];
    int       sampleCount;
    int       width;
    int       height;
    const int clusterCount;
    float     clip;
};

NREstimate::NREstimate(const DImg& img)
    : d(new Private)
{
    d->img = img;
}

NREstimate::~NREstimate()
{
    delete d;
}

void NREstimate::readImage() const
{
    DColor col;
    d->width       = d->img.width();
    d->height      = d->img.height();
    d->sampleCount = d->width*d->height;
    d->clip        = d->img.sixteenBit() ? 65535.0 : 255.0;

    for (int c = 0; c < 3; c++)
    {
        d->fimg[c] = new float[d->width * d->height];
    }

    int j = 0;

    for (int y = 0; y < d->height; y++)
    {
        for (int x = 0; x < d->width; x++)
        {
            col           = d->img.getPixelColor(x, y);
            d->fimg[0][j] = col.red();
            d->fimg[1][j] = col.green();
            d->fimg[2][j] = col.blue();
            ++j;
        }
    }
}

NRContainer NREstimate::estimateNoise() const
{
    readImage();

    //--convert fimg to CvMat*-------------------------------------------------------------------------------
    int i, j, z;

    // convert the image into YCrCb color model
    NRFilter::srgb2ycbcr(d->fimg, d->sampleCount);

    // One dimentional CvMat which stores the image
    CvMat* points    = cvCreateMat(d->sampleCount, 3, CV_32FC1);

    // matrix to store the index of the clusters
    CvMat* clusters  = cvCreateMat(d->sampleCount, 1, CV_32SC1);

    // pointer variable to handle the CvMat* points (the image in CvMat format)
    float* pointsPtr = (float*)points->data.ptr;

    for (int x=0 ; x < d->sampleCount ; x++)
    {
        for (int y=0 ; y < 3 ; y++)
        {
            *pointsPtr++ = (float)d->fimg[y][x];
        }
    }

    // Array to store the centers of the clusters
    CvArr* centers = 0;

    kDebug() << "Everything ready for the cvKmeans2 or as it seems to";

    //-- KMEANS ---------------------------------------------------------------------------------------------

    cvKMeans2(points, d->clusterCount, clusters, cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 10, 1.0), 3, 0, 0, centers, 0);

    kDebug() << "cvKmeans2 succesfully run";

    //-- Divide into cluster->columns, sample->rows, in matrix standard deviation ---------------------------

    int rowPosition[d->clusterCount];

    //the row position array would just make the hold the number of elements in each cluster

    for (i=0 ; i < d->clusterCount ; i++)
    {
        //initializing the cluster count array
        rowPosition[i] = 0;
    }

    int rowIndex, columnIndex;

    for (i=0 ; i < d->sampleCount ; i++)
    {
        columnIndex = clusters->data.i[i];
        rowPosition[columnIndex]++;
    }

/*
    kDebug() << "Lets see what the rowPosition array looks like : ";

    for(i=0 ; i < d->clusterCount ; i++)
    {
        kDebug() << "Cluster : "<<i<<" the count is :"<<rowPosition[i];
    }
*/

    kDebug() << "array indexed, and ready to find maximum";

    //-- Finding maximum of the rowPosition array ------------------------------------------------------------

    int max = rowPosition[0];

    for (i=1 ; i < d->clusterCount ; i++)
    {
        if(rowPosition[i] > max)
        {
            max = rowPosition[i];
        }
    }

    QString maxString;
    maxString.append(QString::number(max));

    kDebug() << QString("maximum declared = %1").arg(maxString);

    //-- Divide and conquer ---------------------------------------------------------------------------------

    CvMat* sd = cvCreateMat(max, (d->clusterCount * points->cols), CV_32FC1);

    //-- Initialize the rowPosition array -------------------------------------------------------------------

    int rPosition[d->clusterCount];

    for (i=0 ; i < d->clusterCount ; i++)
    {
        rPosition[i]=0;
    }

    float* ptr = (float*)sd->data.ptr;

    kDebug() << "The rowPosition array is ready!";

    for (i=0 ; i < d->sampleCount ; i++)
    {
        columnIndex = clusters->data.i[i];
        rowIndex    = rPosition[columnIndex];

        //moving to the right row
        ptr         = (float*)(sd->data.ptr + rowIndex*(sd->step));

        //moving to the right column
        for (int j=0 ; j < columnIndex ; j++)
        {
            for(z=0 ; z < (points->cols) ; z++)
            {
                ptr++;
            }
        }

        for (z=0 ; z < (points->cols) ; z++)
        {
            *ptr++ = cvGet2D(points, i, z).val[0];
        }

        rPosition[columnIndex] = rPosition[columnIndex] + 1;
    }

    kDebug() << "sd matrix creation over!";

    //-- This part of the code would involve the sd matrix and make the mean and the std of the data -------------------

    CvScalar std;
    CvScalar mean;
    CvMat* meanStore    = cvCreateMat(d->clusterCount, points->cols, CV_32FC1);
    CvMat* stdStore     = cvCreateMat(d->clusterCount, points->cols, CV_32FC1);
    float* meanStorePtr = (float*)(meanStore->data.ptr);
    float* stdStorePtr  = (float*)(stdStore->data.ptr);

    // The number of non-empty clusters
    int totalcount      = 0;

    for (i=0 ; i < sd->cols ; i++)
    {
        if(rowPosition[(i/points->cols)] >= 1)
        {
            CvMat* workingArr = cvCreateMat(rowPosition[(i / points->cols)], 1, CV_32FC1);
            ptr               = (float*)(workingArr->data.ptr);

            for (j=0 ; j < rowPosition[(i / (points->cols))] ; j++)
            {
                *ptr++ = cvGet2D(sd, j, i).val[0];
            }

            cvAvgSdv(workingArr, &mean, &std);
            *meanStorePtr++ = (float)mean.val[0];
            *stdStorePtr++  = (float)std.val[0];
            totalcount++;
            cvReleaseMat(&workingArr);
        }
    }

    kDebug() << "Make the mean and the std of the data";

    // -- creating QTextStream (remember to close file) ----------------------------------------------------------------

    QString logFile = d->path;
    logFile         = logFile.section('/', -1);
    logFile         = logFile.left(logFile.indexOf('.'));
    logFile.append("logMeanStd.txt");
    QFile filems (logFile);
    filems.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream ott(&filems);
    meanStorePtr    = (float*)meanStore->data.ptr;
    stdStorePtr     = (float*)stdStore->data.ptr;
    ott << "Mean Data\n";

    for(i=0 ; i < totalcount ; i++)
    {
        ott << *meanStorePtr++;
        ott << "\t";

        if((i+1)%3 == 0)
        {
            ott << "\n";
        }
    }

    ott << "\nStd Data\n";

    for (i=0 ; i < totalcount ; i++)
    {
        ott << *stdStorePtr++;
        ott << "\t";

        if((i+1)%3 == 0)
        {
            ott << "\n";
        }
    }

    filems.close();

    QFile file ("logWeightedMeanStd.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&file);

    kDebug() << "Done with the basic work of storing the mean and the std";

    //-- Calculating weighted mean, and weighted std -----------------------------------------------------------

    QString info;
    float weightedMean = 0.0f;
    float weightedStd  = 0.0f;
    float datasd[3];

    for (j=0 ; j < points->cols ; j++)
    {
        meanStorePtr = (float*) meanStore->data.ptr;
        stdStorePtr  = (float*) stdStore->data.ptr;

        for (int moveToChannel=0 ; moveToChannel <= j ; moveToChannel++)
        {
            meanStorePtr++;
            stdStorePtr++;
        }

        for (i=0 ; i < d->clusterCount ; i++)
        {
            if(rowPosition[i] >= 1)
            {
                weightedMean += (*meanStorePtr) * rowPosition[i];
                weightedStd  += (*stdStorePtr)  * rowPosition[i];
                meanStorePtr += points->cols;
                stdStorePtr  += points->cols;
            }
        }

        weightedMean = weightedMean / (d->sampleCount);
        weightedStd  = weightedStd  / (d->sampleCount);

        out << "\nChannel : " << j <<"\n";
        out << "Weighted Mean : " << weightedMean <<"\n";
        out << "Weighted Std  : " << weightedStd <<"\n";
        info.append("\n\nChannel: ");
        info.append(QString::number(j));
        info.append("\nWeighted Mean: ");
        info.append(QString::number(weightedMean));
        info.append("\nWeighted Standard Deviation: ");
        info.append(QString::number(weightedStd));

        datasd[j] = weightedStd;
    }

    kDebug() << "Info is ready";

    // -- adaptation ---------------------------------------------------------------------------------------

    float L, LSoft = 0.6, Cr, CrSoft = 0.6, Cb, CbSoft = 0.6;

    //for 16 bit images only:
    if (d->clip == 65535)
    {
        for(i=0 ; i < points->cols ; i++)
        {
            datasd[i] = datasd[i] / 256;
        }
    }

    if(datasd[0] < 7)
        L = datasd[0] - 0.98;

    if(datasd[0] >= 7 && datasd[0] < 8)
        L = datasd[0] - 1.2;

    if(datasd[0] >= 8 && datasd[0] < 9)
        L = datasd[0] - 1.5;
    else
        L = datasd[0] - 1.7;

    if(L < 0)
        L = 0;

    if(L > 9)
        L = 9;

    Cr = datasd[2] / 2;
    Cb = datasd[1] / 2;

    if(Cr > 7)
        Cr = 7;

    if(Cb > 7)
        Cb = 7;

    L  = floorf(L  * 100) / 100;
    Cb = floorf(Cb * 100) / 100;
    Cr = floorf(Cr * 100) / 100;

    NRContainer prm;

    prm.thresholds[0] = L;
    prm.thresholds[2] = Cr;
    prm.thresholds[1] = Cb;
    prm.softness[0]   = LSoft;
    prm.softness[2]   = CrSoft;
    prm.softness[1]   = CbSoft;

    kDebug() << "All is completed";

    //-- releasing matrices and closing files ----------------------------------------------------------------------

    cvReleaseMat(&sd);
    cvReleaseMat(&stdStore);
    cvReleaseMat(&meanStore);
    cvReleaseMat(&points);
    cvReleaseMat(&clusters);

    for (i = 0; i < 3; i++)
    {
        delete [] d->fimg[i];
    }

    file.close();
    return prm;
}

}  // namespace Digikam
