/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Sorter
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

#include "libopencv.h"
#include "imgqsort.h"

// C++ includes

#include <cmath>
#include <cfloat>
#include <cstdio>

// Qt includes

#include <QTextStream>
#include <QFile>

// Local includes

#include "digikam_debug.h"
#include "mixerfilter.h"
#include "nrfilter.h"
#include "nrestimate.h"

// To switch on/off log trace file.
// #define TRACE 1

using namespace cv;

namespace Digikam
{

class ImgQSort::Private
{
public:

    Private() :
        clusterCount(30),                   //used for k-means clustering algorithm in noise detection
        size(512)
    {
        for (int c = 0 ; c < 3; c++)
        {
            fimg[c] = 0;
        }

        // Setting the default values

        edgeThresh        = 1;
        lowThreshold      = 0.4;
        ratio             = 3;
        kernel_size       = 3;
        blurrejected      = 0.0;
        blur              = 0.0;
        acceptedThreshold = 0.0;
        pendingThreshold  = 0.0;
        rejectedThreshold = 0.0;
        label             = 0;
        running           = true;
    }

    float*               fimg[3];
    const uint           clusterCount;
    const uint           size;              // Size of squared original image.

    Mat                  src;               // Matrix of the original source image
    Mat                  src_gray;          // Matrix of the grayscaled source image
    Mat                  detected_edges;    // Matrix containing only edges in the image

    int                  edgeThresh;        // threshold above which we say that edges are present at a point
    int                  ratio;             // lower:upper threshold for canny edge detector algorithm
    int                  kernel_size;
    // kernel size for the Sobel operations to be performed internally
    // by the edge detector

    double               lowThreshold;

    DImg                 image;             // original image
    DImg                 neimage;           // noise estimation image[ for color]

    ImageQualitySettings imq;

    double               blurrejected;
    double               blur;

    double               acceptedThreshold;
    double               pendingThreshold;
    double               rejectedThreshold;

    QString              path;              // Path to host result file

    PickLabel*           label;

    volatile bool        running;
};

ImgQSort::ImgQSort(const DImg& img, const ImageQualitySettings& imq, PickLabel* const label)
    : d(new Private)
{
    // Reading settings from GUI
    d->imq.detectBlur         = imq.detectBlur;
    d->imq.detectNoise        = imq.detectNoise;
    d->imq.detectCompression  = imq.detectCompression;
    d->imq.detectOverexposure = imq.detectOverexposure;
    d->imq.lowQRejected       = imq.lowQRejected;
    d->imq.mediumQPending     = imq.mediumQPending;
    d->imq.highQAccepted      = imq.highQAccepted;
    d->imq.speed              = imq.speed;
    d->imq.rejectedThreshold  = imq.rejectedThreshold;
    d->imq.pendingThreshold   = imq.pendingThreshold;
    d->imq.acceptedThreshold  = imq.acceptedThreshold;
    d->imq.blurWeight         = imq.blurWeight;
    d->imq.noiseWeight        = imq.noiseWeight;
    d->imq.compressionWeight  = imq.compressionWeight;
    d->image                  = img;
    d->neimage                = img;
    d->label                  = label;
}

ImgQSort::~ImgQSort()
{
    delete d;
}

void ImgQSort::startAnalyse()
{
    // For Noise Estimation
    // Use the Top/Left corner of 256x256 pixels to analyse noise contents from image.
    // This will speed-up computation time with OpenCV.

    readImage();

    double blur             = 0.0;
    short  blur2            = 0;
    double noise            = 0.0;
    int    compressionlevel = 0;
    float  finalquality     = 0.0;
    int    exposurelevel    = 0;

    // If blur option is selected in settings, run the blur detection algorithms
    if (d->running && d->imq.detectBlur)
    {
        // Returns blur value between 0 and 1.
        // If NaN is returned just assign NoPickLabel
        blur  = blurdetector();
        qCDebug(DIGIKAM_DATABASE_LOG) << "Amount of Blur present in image is  : " << blur;

        // Returns blur value between 1 and 32767.
        // If 1 is returned just assign NoPickLabel
        blur2 = blurdetector2();
        qCDebug(DIGIKAM_DATABASE_LOG) << "Amount of Blur present in image [using LoG Filter] is : " << blur2;
    }

    if (d->running && d->imq.detectNoise)
    {
        // Some images give very low noise value. Assign NoPickLabel in that case.
        // Returns noise value between 0 and 1.
        noise = noisedetector();
        qCDebug(DIGIKAM_DATABASE_LOG) << "Amount of Noise present in image is : " << noise;
    }

    if (d->running && d->imq.detectCompression)
    {
        // Returns number of blocks in the image.
        compressionlevel = compressiondetector();
        qCDebug(DIGIKAM_DATABASE_LOG) << "Amount of compression artifacts present in image is : " << compressionlevel;
    }

    if (d->running && d->imq.detectOverexposure)
    {
        // Returns if there is overexposure in the image
        exposurelevel = exposureamount();
        qCDebug(DIGIKAM_DATABASE_LOG) << "Exposure level present in image is : " << exposurelevel;
    }

#ifdef TRACE

    QFile filems("imgqsortresult.txt");

    if (filems.open(QIODevice::Append | QIODevice::Text))
    {
        QTextStream oms(&filems);
        oms << "File:" << d->image.originalFilePath() << endl;

        if (d->imq.detectBlur)
        {
            oms << "Blur Present:" << blur << endl;
            oms << "Blur Present(using LoG filter):"<< blur2 << endl;
        }

        if (d->imq.detectNoise)
        {
            oms << "Noise Present:" << noise << endl;
        }

        if (d->imq.detectCompression)
        {
            oms << "Compression Present:" << compressionlevel << endl;
        }

        if (d->imq.detectOverexposure)
        {
            oms << "Exposure Present:" << exposurelevel << endl;
        }
    }

#endif // TRACE

    // Calculating finalquality

    // All the results to have a range of 1 to 100.
    if (d->running)
    {
        float finalblur        = (blur*100)  + ((blur2/32767)*100);
        float finalnoise       = noise*100;
        float finalcompression = (compressionlevel / 1024) * 100; // we are processing 1024 pixels size image

        finalquality           = finalblur*d->imq.blurWeight   +
                                 finalnoise*d->imq.noiseWeight +
                                 finalcompression*d->imq.compressionWeight;

        finalquality           = finalquality / 100;

        // Assigning PickLabels

        if (finalquality == 0.0)
        {
            // Algorithms have not been run. So return noPickLabel
            *d->label = NoPickLabel;
        }
        else if (finalquality < d->imq.rejectedThreshold)
        {
            *d->label = RejectedLabel;
        }
        else if (finalquality > d->imq.rejectedThreshold && finalquality < d->imq.acceptedThreshold)
        {
            *d->label = PendingLabel;
        }
        else
        {
            *d->label = AcceptedLabel;
        }
    }
    else
    {
        *d->label = NoPickLabel;
    }
}

void ImgQSort::cancelAnalyse()
{
    d->running = false;
}

void ImgQSort::readImage() const
{
    MixerContainer settings;
    settings.bMonochrome = true;

    MixerFilter mixer(&d->image, 0L, settings);
    mixer.startFilterDirectly();
    d->image.putImageData(mixer.getTargetImage().bits());

    d->src      = Mat(d->image.numPixels(), 3, CV_8UC3); // Create a matrix containing the pixel values of original image
    d->src_gray = Mat(d->image.numPixels(), 1, CV_8UC1); // Create a matrix containing the pixel values of grayscaled image

    if (d->imq.detectNoise)
    {
        DColor col;

        for (int c = 0; d->running && (c < 3); c++)
        {
            d->fimg[c] = new float[d->neimage.numPixels()];
        }

        int j = 0;

        for (uint y = 0; d->running && (y < d->neimage.height()); y++)
        {
            for (uint x = 0; d->running && (x < d->neimage.width()); x++)
            {
                col           = d->neimage.getPixelColor(x, y);
                d->fimg[0][j] = col.red();
                d->fimg[1][j] = col.green();
                d->fimg[2][j] = col.blue();
                j++;
            }
        }
    }
}

void ImgQSort::CannyThreshold(int, void*) const
{
    // Reduce noise with a kernel 3x3.
    blur(d->src_gray, d->detected_edges, Size(3,3));

    // Canny detector.
    Canny(d->detected_edges, d->detected_edges, d->lowThreshold, d->lowThreshold*d->ratio,d-> kernel_size);
}

double ImgQSort::blurdetector() const
{
    d->lowThreshold   = 0.4;
    d->ratio          = 3;
    double maxval     = 0.0;
    ImgQSort::CannyThreshold(0, 0);

    double average    = mean(d->detected_edges)[0];
    int* const maxIdx = new int[sizeof(d->detected_edges)];
    minMaxIdx(d->detected_edges, 0, &maxval, 0, maxIdx);

    double blurresult = average / maxval;

    qCDebug(DIGIKAM_DATABASE_LOG) << "The average of the edge intensity is " << average;
    qCDebug(DIGIKAM_DATABASE_LOG) << "The maximum of the edge intensity is " << maxval;
    qCDebug(DIGIKAM_DATABASE_LOG) << "The result of the edge intensity is  " << blurresult;

    delete [] maxIdx;
    return blurresult;
}

short ImgQSort::blurdetector2() const
{
    // Algorithm using Laplacian of Gaussian Filter to detect blur.
    Mat out;
    Mat noise_free;
    qCDebug(DIGIKAM_DATABASE_LOG) << "Algorithm using LoG Filter started";

    // To remove noise from the image
    GaussianBlur(d->src_gray, noise_free, Size(3,3), 0, 0, BORDER_DEFAULT);

    // Aperture size of 1 corresponds to the correct matrix
    int kernel_size = 3;
    int scale       = 1;
    int delta       = 0;
    int ddepth      = CV_16S;

    Laplacian(noise_free, out, ddepth, kernel_size, scale, delta, BORDER_DEFAULT);

    // noise_free:  The input image without noise.
    // out:         Destination (output) image
    // ddepth:      Depth of the destination image. Since our input is CV_8U we define ddepth = CV_16S to avoid overflow
    // kernel_size: The kernel size of the Sobel operator to be applied internally. We use 3 ihere

    short maxLap = -32767;

    for (int i = 0; i < out.rows; i++)
    {
        for (int j = 0; j < out.cols; j++)
        {
            short value = out.at<short>(i,j);

            if (value > maxLap)
            {
                maxLap = value ;
            }
        }
    }

    return maxLap;
}

double ImgQSort::noisedetector() const
{
    double noiseresult = 0.0;

    //--convert fimg to CvMat*-------------------------------------------------------------------------------

    // Convert the image into YCrCb color model.
    NRFilter::srgb2ycbcr(d->fimg, d->neimage.numPixels());

    // One dimentional CvMat which stores the image.
    CvMat* points    = cvCreateMat(d->neimage.numPixels(), 3, CV_32FC1);

    // Matrix to store the index of the clusters.
    CvMat* clusters  = cvCreateMat(d->neimage.numPixels(), 1, CV_32SC1);

    // Pointer variable to handle the CvMat* points (the image in CvMat format).
    float* pointsPtr = reinterpret_cast<float*>(points->data.ptr);

    for (uint x=0 ; d->running && (x < d->neimage.numPixels()) ; x++)
    {
        for (int y=0 ; d->running && (y < 3) ; y++)
        {
            *pointsPtr++ = (float)d->fimg[y][x];
        }
    }

    // Array to store the centers of the clusters.
    CvArr* centers = 0;

    qCDebug(DIGIKAM_DATABASE_LOG) << "Everything ready for the cvKmeans2 or as it seems to";

    //-- KMEANS ---------------------------------------------------------------------------------------------

    if (d->running)
    {
        cvKMeans2(points, d->clusterCount, clusters,
                  cvTermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0), 3, 0, 0, centers, 0);
    }

    qCDebug(DIGIKAM_DATABASE_LOG) << "cvKmeans2 successfully run";

    //-- Divide into cluster->columns, sample->rows, in matrix standard deviation ---------------------------

    QScopedArrayPointer<int> rowPosition(new int[d->clusterCount]);

    // The row position array would just make the hold the number of elements in each cluster.

    for (uint i=0 ; d->running && (i < d->clusterCount) ; i++)
    {
        // Initializing the cluster count array.
        rowPosition[i] = 0;
    }

    int rowIndex, columnIndex;

    for (uint i=0 ; d->running && (i < d->neimage.numPixels()) ; i++)
    {
        columnIndex = clusters->data.i[i];
        rowPosition[columnIndex]++;
    }

    qCDebug(DIGIKAM_DATABASE_LOG) << "array indexed, and ready to find maximum";

    //-- Finding maximum of the rowPosition array ------------------------------------------------------------

    int max = rowPosition[0];

    for (uint i=1 ; d->running && (i < d->clusterCount) ; i++)
    {
        if (rowPosition[i] > max)
        {
            max = rowPosition[i];
        }
    }

    QString maxString;
    maxString.append(QString::number(max));

    qCDebug(DIGIKAM_DATABASE_LOG) << QString::fromUtf8("maximum declared = %1").arg(maxString);

    //-- Divide and conquer ---------------------------------------------------------------------------------

    CvMat* sd = 0;

    if (d->running)
    {
        sd = cvCreateMat(max, (d->clusterCount * points->cols), CV_32FC1);
    }

    //-- Initialize the rowPosition array -------------------------------------------------------------------

    QScopedArrayPointer<int> rPosition(new int[d->clusterCount]);

    for (uint i=0 ; d->running && (i < d->clusterCount) ; i++)
    {
        rPosition[i] = 0;
    }

    float* ptr = 0;

    qCDebug(DIGIKAM_DATABASE_LOG) << "The rowPosition array is ready!";

    for (uint i=0 ; d->running && (i < d->neimage.numPixels()) ; i++)
    {
        columnIndex = clusters->data.i[i];
        rowIndex    = rPosition[columnIndex];

        // Moving to the right row.

        ptr         = reinterpret_cast<float*>(sd->data.ptr + rowIndex*(sd->step));

        // Moving to the right column.

        for (int j=0 ; d->running && (j < columnIndex) ; j++)
        {
            for (int z=0 ; d->running && (z < (points->cols)) ; z++)
            {
                ptr++;
            }
        }

        for (int z=0 ; d->running && (z < (points->cols)) ; z++)
        {
            *ptr++ = cvGet2D(points, i, z).val[0];
        }

        rPosition[columnIndex] = rPosition[columnIndex] + 1;
    }

    qCDebug(DIGIKAM_DATABASE_LOG) << "sd matrix creation over!";

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

    for (int i=0 ; d->running && (i < sd->cols) ; i++)
    {
        if (d->running && (rowPosition[(i/points->cols)] >= 1))
        {
            CvMat* workingArr = cvCreateMat(rowPosition[(i / points->cols)], 1, CV_32FC1);
            ptr               = reinterpret_cast<float*>(workingArr->data.ptr);

            for (int j=0 ; d->running && (j < rowPosition[(i / (points->cols))]) ; j++)
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

    qCDebug(DIGIKAM_DATABASE_LOG) << "Make the mean and the std of the data";

    // -----------------------------------------------------------------------------------------------------------------

    if (d->running)
    {
        meanStorePtr = reinterpret_cast<float*>(meanStore->data.ptr);
        stdStorePtr  = reinterpret_cast<float*>(stdStore->data.ptr);
    }

    qCDebug(DIGIKAM_DATABASE_LOG) << "Done with the basic work of storing the mean and the std";

    //-- Calculating weighted mean, and weighted std -----------------------------------------------------------

    QString info;
    float   weightedMean = 0.0F;
    float   weightedStd  = 0.0F;
    float   datasd[3]    = {0.0F, 0.0F, 0.0F};

    for (int j=0 ; d->running && (j < points->cols) ; j++)
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

    qCDebug(DIGIKAM_DATABASE_LOG) << "Info : " << info;

    // -- adaptation ---------------------------------------------------------------------------------------

    if (d->running)
    {
        // For 16 bits images only.

        if (d->neimage.sixteenBit())
        {
            for (int i=0 ; i < points->cols ; i++)
            {
                datasd[i] = datasd[i] / 256;
            }
        }

        noiseresult = ((datasd[0]/2)+(datasd[1]/2)+(datasd[2]/2))/3;

        qCDebug(DIGIKAM_DATABASE_LOG) << "All is completed";

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
    }

    return noiseresult;
}

int ImgQSort::compressiondetector() const
{
    //FIXME: set threshold value to an acceptable standard to get the number of blocking artifacts
    const int THRESHOLD  = 30;
    const int block_size = 8;
    int countblocks      = 0;
    int number_of_blocks = 0;
    int sum              = 0;

    std::vector<int> average_bottom, average_middle, average_top;

    // Go through 8 blocks at a time horizontally
    // iterating through columns.

    for (int i = 0; d->running && i < d->src_gray.rows; i++)
    {
        // Calculating intensity of top column.

        for (int j = 0; j < d->src_gray.cols; j+=8)
        {
            sum = 0;

            for (int k = j; k < block_size; k++)
            {
                sum += (int)d->src_gray.at<uchar>(i, j);
            }

            average_top.push_back(sum/8);
        }

        // Calculating intensity of middle column.

        for (int j = 0; j < d->src_gray.cols; j+=8)
        {
            sum = 0;

            for (int k = j; k < block_size; k++)
            {
                sum += (int)d->src_gray.at<uchar>(i+1, j);
            }

            average_middle.push_back(sum/8);
        }

        // Calculating intensity of bottom column.

        countblocks = 0;

        for (int j = 0; j < d->src_gray.cols; j+=8)
        {
            sum = 0;

            for (int k = j; k < block_size; k++)
            {
                sum += (int)d->src_gray.at<uchar>(i+2, j);
            }

            average_bottom.push_back(sum/8);
            countblocks++;
        }

        // Check if the average intensity of 8 blocks in the top, middle and bottom rows are equal.
        // If so increment number_of_blocks.

        for (int j = 0; j < countblocks; j++)
        {
            if ((average_middle[j] == (average_top[j]+average_bottom[j])/2) &&
                average_middle[j] > THRESHOLD)
            {
                number_of_blocks++;
            }
        }
    }

    average_bottom.clear();
    average_middle.clear();
    average_top.clear();

    // Iterating through rows.

    for (int j = 0; d->running && j < d->src_gray.cols; j++)
    {
        // Calculating intensity of top row.

        for (int i = 0; i < d->src_gray.rows; i+=8)
        {
            sum = 0;

            for (int k = i; k < block_size; k++)
            {
                sum += (int)d->src_gray.at<uchar>(i, j);
            }

            average_top.push_back(sum/8);
        }

        // Calculating intensity of middle row.

        for (int i= 0; i < d->src_gray.rows; i+=8)
        {
            sum = 0;

            for (int k = i; k < block_size; k++)
            {
                sum += (int)d->src_gray.at<uchar>(i, j+1);
            }

            average_middle.push_back(sum/8);
        }

        // Calculating intensity of bottom row.

        countblocks = 0;

        for (int i = 0; i < d->src_gray.rows; i+=8)
        {
            sum = 0;

            for (int k = i; k < block_size; k++)
            {
                sum += (int)d->src_gray.at<uchar>(i, j+2);
            }

            average_bottom.push_back(sum/8);
            countblocks++;
        }

        // Check if the average intensity of 8 blocks in the top, middle and bottom rows are equal.
        // If so increment number_of_blocks.

        for (int i = 0; i < countblocks; i++)
        {
            if ((average_middle[i] == (average_top[i]+average_bottom[i])/2) &&
                average_middle[i] > THRESHOLD)
            {
                number_of_blocks++;
            }
        }
    }

    return number_of_blocks;
}

int ImgQSort::exposureamount() const
{
    /// Separate the image in 3 places ( B, G and R )

    std::vector<Mat> bgr_planes;

    split(d->src, bgr_planes);

    /// Establish the number of bins
    int histSize           = 256;

    /// Set the ranges ( for B,G,R) )
    float range[]          = { 0, 256 } ;
    const float* histRange = { range };

    bool uniform           = true;
    bool accumulate        = false;

    Mat b_hist, g_hist, r_hist;

    /// Compute the histograms
    calcHist(&bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate);
    calcHist(&bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate);
    calcHist(&bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate);

    // Draw the histograms for B, G and R
    int hist_w = 512;
    int hist_h = 400;

    Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

    /// Normalize the histograms:

    normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
    normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());
    normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat());

    /// Sum the histograms
    Scalar rmean,gmean,bmean;
    rmean             = mean(r_hist);
    gmean             = mean(g_hist);
    bmean             = mean(b_hist);

    int exposurelevel = (rmean[0] + gmean[0] + bmean[0]) / 3;

    return exposurelevel;
}

}  // namespace Digikam
