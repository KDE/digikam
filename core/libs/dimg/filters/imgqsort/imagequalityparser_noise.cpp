/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Parser
 *
 * Copyright (C) 2013-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagequalityparser_p.h"

namespace Digikam
{

double ImageQualityParser::noiseDetector() const
{
    double noiseresult = 0.0;

    //--convert fimg to CvMat*-------------------------------------------------------------------------------

    // Convert the image into YCrCb color model.
    NRFilter::srgb2ycbcr(d->fimg, d->neimage.numPixels());

    // One dimensional CvMat which stores the image.
    CvMat* points    = cvCreateMat(d->neimage.numPixels(), 3, CV_32FC1);

    // Matrix to store the index of the clusters.
    CvMat* clusters  = cvCreateMat(d->neimage.numPixels(), 1, CV_32SC1);

    // Pointer variable to handle the CvMat* points (the image in CvMat format).
    float* pointsPtr = reinterpret_cast<float*>(points->data.ptr);

    for (uint x = 0 ; d->running && (x < d->neimage.numPixels()) ; ++x)
    {
        for (int y = 0 ; d->running && (y < 3) ; ++y)
        {
            *pointsPtr++ = (float)d->fimg[y][x];
        }
    }

    // Array to store the centers of the clusters.
    CvArr* centers = 0;

    qCDebug(DIGIKAM_DIMG_LOG) << "Everything ready for the cvKmeans2 or as it seems to";

    //-- KMEANS ---------------------------------------------------------------------------------------------

    if (d->running)
    {
        cvKMeans2(points,
                  d->clusterCount,
                  clusters,
                  cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
                  3,
                  0,
                  0,
                  centers,
                  0);
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "cvKmeans2 successfully run";

    //-- Divide into cluster->columns, sample->rows, in matrix standard deviation ---------------------------

    QScopedArrayPointer<int> rowPosition(new int[d->clusterCount]);

    // The row position array would just make the hold the number of elements in each cluster.

    for (uint i = 0 ; d->running && (i < d->clusterCount) ; ++i)
    {
        // Initializing the cluster count array.
        rowPosition[i] = 0;
    }

    int rowIndex, columnIndex;

    for (uint i = 0 ; d->running && (i < d->neimage.numPixels()) ; ++i)
    {
        columnIndex = clusters->data.i[i];
        rowPosition[columnIndex]++;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "array indexed, and ready to find maximum";

    //-- Finding maximum of the rowPosition array ------------------------------------------------------------

    int max = rowPosition[0];

    for (uint i = 1 ; d->running && (i < d->clusterCount) ; ++i)
    {
        if (rowPosition[i] > max)
        {
            max = rowPosition[i];
        }
    }

    QString maxString;
    maxString.append(QString::number(max));

    qCDebug(DIGIKAM_DIMG_LOG) << QString::fromUtf8("maximum declared = %1").arg(maxString);

    //-- Divide and conquer ---------------------------------------------------------------------------------

    CvMat* sd = 0;

    if (d->running)
    {
        sd = cvCreateMat(max, (d->clusterCount * points->cols), CV_32FC1);
    }

    //-- Initialize the rowPosition array -------------------------------------------------------------------

    QScopedArrayPointer<int> rPosition(new int[d->clusterCount]);

    for (uint i = 0 ; d->running && (i < d->clusterCount) ; ++i)
    {
        rPosition[i] = 0;
    }

    float* ptr = 0;

    qCDebug(DIGIKAM_DIMG_LOG) << "The rowPosition array is ready!";

    for (uint i = 0 ; d->running && (i < d->neimage.numPixels()) ; ++i)
    {
        columnIndex = clusters->data.i[i];
        rowIndex    = rPosition[columnIndex];

        // Moving to the right row.

        ptr         = reinterpret_cast<float*>(sd->data.ptr + (rowIndex * sd->step));

        // Moving to the right column.

        for (int j = 0 ; d->running && (j < columnIndex) ; ++j)
        {
            for (int z = 0 ; d->running && (z < (points->cols)) ; ++z)
            {
                ptr++;
            }
        }

        for (int z = 0 ; d->running && (z < (points->cols)) ; ++z)
        {
            *ptr++ = cvGet2D(points, i, z).val[0];
        }

        rPosition[columnIndex] = rPosition[columnIndex] + 1;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "sd matrix creation over!";

    //-- This part of the code would involve the sd matrix and make the mean and the std of the data -------------

    CvScalar std;
    CvScalar mean;
    CvMat*   meanStore    = 0;
    CvMat*   stdStore     = 0;
    float*   meanStorePtr = 0;
    float*   stdStorePtr  = 0;
    int      totalcount   = 0; // Number of non-empty clusters.

    if (d->running)
    {
        meanStore    = cvCreateMat(d->clusterCount, points->cols, CV_32FC1);
        stdStore     = cvCreateMat(d->clusterCount, points->cols, CV_32FC1);
        meanStorePtr = reinterpret_cast<float*>(meanStore->data.ptr);
        stdStorePtr  = reinterpret_cast<float*>(stdStore->data.ptr);
    }

    for (int i = 0 ; d->running && (i < sd->cols) ; ++i)
    {
        if (d->running && (rowPosition[(i / points->cols)] >= 1))
        {
            CvMat* workingArr = cvCreateMat(rowPosition[(i / points->cols)], 1, CV_32FC1);
            ptr               = reinterpret_cast<float*>(workingArr->data.ptr);

            for (int j = 0 ; d->running && (j < rowPosition[(i / (points->cols))]) ; ++j)
            {
                *ptr++ = cvGet2D(sd, j, i).val[0];
            }

            cvAvgSdv(workingArr, &mean, &std);
            *meanStorePtr++ = (float)mean.val[0];
            *stdStorePtr++  = (float)std.val[0];
            ++totalcount;
            cvReleaseMat(&workingArr);
        }
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Make the mean and the std of the data";

    // -----------------------------------------------------------------------------------------------------------------

    if (d->running)
    {
        meanStorePtr = reinterpret_cast<float*>(meanStore->data.ptr);
        stdStorePtr  = reinterpret_cast<float*>(stdStore->data.ptr);
    }

    // Remove clang warnings.
    (void)meanStorePtr;
    (void)stdStorePtr;

    qCDebug(DIGIKAM_DIMG_LOG) << "Done with the basic work of storing the mean and the std";

    //-- Calculating weighted mean, and weighted std -----------------------------------------------------------

    QString info;
    float   weightedMean = 0.0F;
    float   weightedStd  = 0.0F;
    float   datasd[3]    = { 0.0F, 0.0F, 0.0F };

    for (int j = 0 ; d->running && (j < points->cols) ; ++j)
    {
        meanStorePtr = reinterpret_cast<float*>(meanStore->data.ptr);
        stdStorePtr  = reinterpret_cast<float*>(stdStore->data.ptr);

        for (int moveToChannel = 0 ; moveToChannel <= j ; ++moveToChannel)
        {
            meanStorePtr++;
            stdStorePtr++;
        }

        for (uint i = 0 ; i < d->clusterCount ; ++i)
        {
            if (rowPosition[i] >= 1)
            {
                weightedMean += (*meanStorePtr) * rowPosition[i];
                weightedStd  += (*stdStorePtr)  * rowPosition[i];
                meanStorePtr += points->cols;
                stdStorePtr  += points->cols;
            }
        }

        weightedMean = weightedMean / (d->neimage.numPixels());
        weightedStd  = weightedStd  / (d->neimage.numPixels());
        datasd[j]    = weightedStd;

        info.append(QLatin1String("\n\nChannel: "));
        info.append(QString::number(j));
        info.append(QLatin1String("\nWeighted Mean: "));
        info.append(QString::number(weightedMean));
        info.append(QLatin1String("\nWeighted Standard Deviation: "));
        info.append(QString::number(weightedStd));
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Info : " << info;

    // -- adaptation ---------------------------------------------------------------------------------------

    if (d->running)
    {
        // For 16 bits images only.

        if (d->neimage.sixteenBit())
        {
            for (int i = 0 ; i < points->cols ; ++i)
            {
                datasd[i] = datasd[i] / 256;
            }
        }

        noiseresult = ((datasd[0] / 2) + (datasd[1] / 2) + (datasd[2] / 2)) / 3;

        qCDebug(DIGIKAM_DIMG_LOG) << "All is completed";

        //-- releasing matrices and closing files ----------------------------------------------------------------------

        cvReleaseMat(&sd);
        cvReleaseMat(&stdStore);
        cvReleaseMat(&meanStore);
        cvReleaseMat(&points);
        cvReleaseMat(&clusters);

        for (uint i = 0 ; i < 3 ; ++i)
        {
            delete [] d->fimg[i];
        }
    }

    return noiseresult;
}

} // namespace Digikam
