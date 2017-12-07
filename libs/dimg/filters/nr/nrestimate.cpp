/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-18
 * Description : Wavelets YCrCb Noise Reduction settings estimation by image content analys.
 *               Wavelets theory is based on "À Trous" Discrete Wavelet Transform
 *               described into "The Handbook of Astronomical Image Processing" book
 *               from Richard Berry and James Burnell, chapter 18.
 *               See this wiki page for details:
 *               http://community.kde.org/Digikam/SoK2012/AutoNR
 *
 * Copyright (C) 2012-2013 by Sayantan Datta <sayantan dot knz at gmail dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// OpenCV includes

#include "libopencv.h"

// Qt includes.

#include <QTextStream>
#include <QFile>

// Local includes

#include "digikam_debug.h"
#include "nrestimate.h"
#include "nrfilter.h"

namespace Digikam
{

class NREstimate::Private
{
public:

    Private() :
       clusterCount(30),
       size(512)
    {
        for (int c = 0 ; c < 3; c++)
        {
            fimg[c] = 0;
        }
    }

    NRContainer prm;

    QString     path;   // Path to host log files.

    float*      fimg[3];
    const uint  clusterCount;
    const uint  size;   // Size of squared original image.
};

NREstimate::NREstimate(DImg* const img, QObject* const parent)
    : DImgThreadedAnalyser(parent, QLatin1String("NREstimate")),
      d(new Private)
{
    // Use the Top/Left corner of 256x256 pixels to analys noise contents from image.
    // This will speed-up computation time with OpenCV
    int w = (img->width()  > d->size) ? d->size : img->width();
    int h = (img->height() > d->size) ? d->size : img->height();
    setOriginalImage(img->copy(0, 0, w, h));
}

NREstimate::~NREstimate()
{
    delete d;
}

void NREstimate::setLogFilesPath(const QString& path)
{
    d->path = path;
}

void NREstimate::readImage() const
{
    DColor col;

    for (int c = 0; runningFlag() && (c < 3); c++)
    {
        d->fimg[c] = new float[m_orgImage.numPixels()];
    }

    int j = 0;

    for (uint y = 0; runningFlag() && (y < m_orgImage.height()); y++)
    {
        for (uint x = 0; runningFlag() && (x < m_orgImage.width()); x++)
        {
            col           = m_orgImage.getPixelColor(x, y);
            d->fimg[0][j] = col.red();
            d->fimg[1][j] = col.green();
            d->fimg[2][j] = col.blue();
            j++;
        }
    }
}

NRContainer NREstimate::settings() const
{
    return d->prm;
}

void NREstimate::startAnalyse()
{
    readImage();
    postProgress(5);

    //--convert fimg to CvMat*-------------------------------------------------------------------------------

    // convert the image into YCrCb color model
    NRFilter::srgb2ycbcr(d->fimg, m_orgImage.numPixels());

    // One dimentional CvMat which stores the image
    CvMat* points    = cvCreateMat(m_orgImage.numPixels(), 3, CV_32FC1);

    // matrix to store the index of the clusters
    CvMat* clusters  = cvCreateMat(m_orgImage.numPixels(), 1, CV_32SC1);

    // pointer variable to handle the CvMat* points (the image in CvMat format)
    float* pointsPtr = reinterpret_cast<float*>(points->data.ptr);

    for (uint x=0 ; runningFlag() && (x < m_orgImage.numPixels()) ; x++)
    {
        for (int y=0 ; runningFlag() && (y < 3) ; y++)
        {
            *pointsPtr++ = (float)d->fimg[y][x];
        }
    }

    // Array to store the centers of the clusters
    CvArr* centers = 0;

    qCDebug(DIGIKAM_DIMG_LOG) << "Everything ready for the cvKmeans2 or as it seems to";
    postProgress(10);

    //-- KMEANS ---------------------------------------------------------------------------------------------

    if (runningFlag())
    {
        cvKMeans2(points, d->clusterCount, clusters,
                  cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0), 3, 0, 0, centers, 0);
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "cvKmeans2 successfully run";
    postProgress(15);

    //-- Divide into cluster->columns, sample->rows, in matrix standard deviation ---------------------------

    QScopedArrayPointer<int> rowPosition(new int[d->clusterCount]);

    //the row position array would just make the hold the number of elements in each cluster

    for (uint i=0 ; runningFlag() && (i < d->clusterCount) ; i++)
    {
        //initializing the cluster count array
        rowPosition[i] = 0;
    }

    int rowIndex, columnIndex;

    for (uint i=0 ; runningFlag() && (i < m_orgImage.numPixels()) ; i++)
    {
        columnIndex = clusters->data.i[i];
        rowPosition[columnIndex]++;
    }

/*
    qCDebug(DIGIKAM_DIMG_LOG) << "Lets see what the rowPosition array looks like : ";

    for(uint i=0 ; runningFlag() && (i < d->clusterCount) ; i++)
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Cluster : "<< i << " the count is :" << rowPosition[i];
    }
*/

    qCDebug(DIGIKAM_DIMG_LOG) << "array indexed, and ready to find maximum";
    postProgress(20);

    //-- Finding maximum of the rowPosition array ------------------------------------------------------------

    int max = rowPosition[0];

    for (uint i=1 ; runningFlag() && (i < d->clusterCount) ; i++)
    {
        if (rowPosition[i] > max)
        {
            max = rowPosition[i];
        }
    }

    QString maxString;
    maxString.append(QString::number(max));

    qCDebug(DIGIKAM_DIMG_LOG) << QString::fromLatin1("maximum declared = %1").arg(maxString);
    postProgress(25);

    //-- Divide and conquer ---------------------------------------------------------------------------------

    CvMat* sd = 0;

    if (runningFlag())
    {
        sd = cvCreateMat(max, (d->clusterCount * points->cols), CV_32FC1);
    }

    postProgress(30);

    //-- Initialize the rowPosition array -------------------------------------------------------------------

    QScopedArrayPointer<int> rPosition(new int[d->clusterCount]);

    for (uint i=0 ; runningFlag() && (i < d->clusterCount) ; i++)
    {
        rPosition[i] = 0;
    }

    float* ptr = 0;

    qCDebug(DIGIKAM_DIMG_LOG) << "The rowPosition array is ready!";
    postProgress(40);

    for (uint i=0 ; runningFlag() && (i < m_orgImage.numPixels()) ; i++)
    {
        columnIndex = clusters->data.i[i];
        rowIndex    = rPosition[columnIndex];

        //moving to the right row
        ptr         = reinterpret_cast<float*>(sd->data.ptr + rowIndex*(sd->step));

        //moving to the right column
        for (int j=0 ; runningFlag() && (j < columnIndex) ; j++)
        {
            for(int z=0 ; runningFlag() && (z < (points->cols)) ; z++)
            {
                ptr++;
            }
        }

        for (int z=0 ; runningFlag() && (z < (points->cols)) ; z++)
        {
            *ptr++ = cvGet2D(points, i, z).val[0];
        }

        rPosition[columnIndex] = rPosition[columnIndex] + 1;
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "sd matrix creation over!";
    postProgress(50);

    //-- This part of the code would involve the sd matrix and make the mean and the std of the data -------------------

    CvScalar std;
    CvScalar mean;
    CvMat*   meanStore    = 0;
    CvMat*   stdStore     = 0;
    float*   meanStorePtr = 0;
    float*   stdStorePtr  = 0;
    int      totalcount   = 0; // Number of non-empty clusters

    if (runningFlag())
    {
        meanStore    = cvCreateMat(d->clusterCount, points->cols, CV_32FC1);
        stdStore     = cvCreateMat(d->clusterCount, points->cols, CV_32FC1);
        meanStorePtr = reinterpret_cast<float*>(meanStore->data.ptr);
        stdStorePtr  = reinterpret_cast<float*>(stdStore->data.ptr);
    }

    for (int i=0 ; runningFlag() && (i < sd->cols) ; i++)
    {
        if (runningFlag() && (rowPosition[(i/points->cols)] >= 1))
        {
            CvMat* workingArr = cvCreateMat(rowPosition[(i / points->cols)], 1, CV_32FC1);
            ptr               = reinterpret_cast<float*>(workingArr->data.ptr);

            for (int j=0 ; runningFlag() && (j < rowPosition[(i / (points->cols))]) ; j++)
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

    qCDebug(DIGIKAM_DIMG_LOG) << "Make the mean and the std of the data";
    postProgress(60);

    // -----------------------------------------------------------------------------------------------------------------

    if (runningFlag())
    {
        meanStorePtr = reinterpret_cast<float*>(meanStore->data.ptr);
        stdStorePtr  = reinterpret_cast<float*>(stdStore->data.ptr);
    }

    if (runningFlag() && !d->path.isEmpty())
    {
        QString logFile = d->path;
        logFile         = logFile.section(QLatin1Char('/'), -1);
        logFile         = logFile.left(logFile.indexOf(QLatin1Char('.')));
        logFile.append(QLatin1String("logMeanStd.txt"));

        QFile filems(logFile);

        if (filems.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream oms(&filems);
            oms << "Mean Data\n";

            for (int i=0 ; i < totalcount ; i++)
            {
                oms << *meanStorePtr++;
                oms << "\t";

                if ((i+1)%3 == 0)
                {
                    oms << "\n";
                }
            }

            oms << "\nStd Data\n";

            for (int i=0 ; i < totalcount ; i++)
            {
                oms << *stdStorePtr++;
                oms << "\t";

                if ((i+1)%3 == 0)
                {
                    oms << "\n";
                }
            }

            filems.close();

            qCDebug(DIGIKAM_DIMG_LOG) << "Done with the basic work of storing the mean and the std";
        }
    }

    postProgress(70);

    //-- Calculating weighted mean, and weighted std -----------------------------------------------------------

    QTextStream owms;
    QFile       filewms;

    if (runningFlag() && !d->path.isEmpty())
    {
        QString logFile2 = d->path;
        logFile2         = logFile2.section(QLatin1Char('/'), -1);
        logFile2         = logFile2.left(logFile2.indexOf(QLatin1Char('.')));
        logFile2.append(QLatin1String("logWeightedMeanStd.txt"));

        filewms.setFileName(logFile2);

        if (filewms.open(QIODevice::WriteOnly | QIODevice::Text))
            owms.setDevice(&filewms);
    }

    QString info;
    float   weightedMean = 0.0f;
    float   weightedStd  = 0.0f;
    float   datasd[3]    = {0.0f, 0.0f, 0.0f};

    for (int j=0 ; runningFlag() && (j < points->cols) ; j++)
    {
        meanStorePtr = reinterpret_cast<float*>(meanStore->data.ptr);
        stdStorePtr  = reinterpret_cast<float*>(stdStore->data.ptr);

        for (int moveToChannel=0 ; moveToChannel <= j ; moveToChannel++)
        {
            meanStorePtr++;
            stdStorePtr++;
        }

        for (uint i=0 ; i < d->clusterCount ; i++)
        {
            if (rowPosition[i] >= 1)
            {
                weightedMean += (*meanStorePtr) * rowPosition[i];
                weightedStd  += (*stdStorePtr)  * rowPosition[i];
                meanStorePtr += points->cols;
                stdStorePtr  += points->cols;
            }
        }

        weightedMean = weightedMean / (m_orgImage.numPixels());
        weightedStd  = weightedStd  / (m_orgImage.numPixels());
        datasd[j]    = weightedStd;

        if (!d->path.isEmpty())
        {
            owms << QLatin1String("\nChannel : ")     << j            << QLatin1String("\n");
            owms << QLatin1String("Weighted Mean : ") << weightedMean << QLatin1String("\n");
            owms << QLatin1String("Weighted Std  : ") << weightedStd  << QLatin1String("\n");
        }

        info.append(QLatin1String("\n\nChannel: "));
        info.append(QString::number(j));
        info.append(QLatin1String("\nWeighted Mean: "));
        info.append(QString::number(weightedMean));
        info.append(QLatin1String("\nWeighted Standard Deviation: "));
        info.append(QString::number(weightedStd));
    }

    if (runningFlag() && !d->path.isEmpty())
    {
        filewms.close();
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Info : " << info;
    postProgress(80);

    // -- adaptation ---------------------------------------------------------------------------------------

    double L = 1.2, LSoft = 0.9, Cr = 1.2, CrSoft = 0.9, Cb = 1.2, CbSoft = 0.9;

    if (runningFlag())
    {
        // for 16 bits images only
        if (m_orgImage.sixteenBit())
        {
            for (int i=0 ; i < points->cols ; i++)
            {
                datasd[i] = datasd[i] / 256;
            }
        }

        if (datasd[0] < 7)
            L = datasd[0] - 0.98;
        else if (datasd[0] >= 7 && datasd[0] < 8)
            L = datasd[0] - 1.2;
        else if (datasd[0] >= 8 && datasd[0] < 9)
            L = datasd[0] - 1.5;
        else
            L = datasd[0] - 1.7;

        if (L < 0)
            L = 0;

        if (L > 9)
            L = 9;

        Cr = datasd[2] * 0.8;
        Cb = datasd[1] * 0.8;

        if (Cr > 7)
            Cr = 7;

        if (Cb > 7)
            Cb = 7;

        L  = floorf(L  * 100) / 100;
        Cb = floorf(Cb * 100) / 100;
        Cr = floorf(Cr * 100) / 100;

        if ( L > 9 )
            LSoft = CrSoft = CbSoft = 0.8;
        else if ( L > 3)
            LSoft = CrSoft = CbSoft = 0.7;
        else
            LSoft = CrSoft = CbSoft = 0.6;
    }

    d->prm.thresholds[0] = L;
    d->prm.thresholds[1] = Cb;
    d->prm.thresholds[2] = Cr;
    d->prm.softness[0]   = LSoft;
    d->prm.softness[1]   = CbSoft;
    d->prm.softness[2]   = CrSoft;

    qCDebug(DIGIKAM_DIMG_LOG) << "All is completed";
    postProgress(90);

    //-- releasing matrices and closing files ----------------------------------------------------------------------

    cvReleaseMat(&sd);
    cvReleaseMat(&stdStore);
    cvReleaseMat(&meanStore);
    cvReleaseMat(&points);
    cvReleaseMat(&clusters);

    for (uint i = 0; i < 3; i++)
    {
        delete [] d->fimg[i];
    }

    postProgress(100);
}

}  // namespace Digikam
