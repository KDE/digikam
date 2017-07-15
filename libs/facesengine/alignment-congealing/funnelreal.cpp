/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2013-06-14
 * Description : Alignment by Image Congealing.
 *               Funneling for complex, realistic images
 *               using sequence of distribution fields learned from congealReal
 *               Gary B. Huang, Vidit Jain, and Erik Learned-Miller.
 *               Unsupervised joint alignment of complex images.
 *               International Conference on Computer Vision (ICCV), 2007.
 *
 * Copyright (C) 2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2007 by Gary B. Huang, UMass-Amherst
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

#include "funnelreal.h"

// C++ includes

#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// Qt includes

#include <QFileInfo>
#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

class FunnelReal::Private
{
public:

    Private()
        : isLoaded(false),
          numParams(4),          // similarity transforms - x translation, y translation, rotation, uniform scaling
          windowSize(4),
          maxProcessAtOnce(600), // set based on memory limitations,
          outerDimW(150),
          outerDimH(150),
          innerDimW(100),
          innerDimH(100),
          siftHistDim(4),
          siftBucketsDim(8),
          siftDescDim((4*windowSize*windowSize*siftBucketsDim) / (siftHistDim*siftHistDim)),
          numRandPxls(0),
          numFeatureClusters(0),
          edgeDescDim(0)
    {
/*
        if(outerDimW - innerDimW < 2*windowSize)
        {
            qCDebug(DIGIKAM_FACESENGINE_LOG) << "difference between outerDimW and innerDimW is not greater than window size for SIFT descriptor)";
            return -1;
        }

        if( (outerDimW - innerDimW) % 2 != 0)
        {
            qCDebug(DIGIKAM_FACESENGINE_LOG) << "shrinking innerDimW by 1 so outerDimW - innerDimW is divisible by 2";
            --innerDimW;
        }

        if(outerDimH - innerDimH < 2*windowSize)
        {
            qCDebug(DIGIKAM_FACESENGINE_LOG) << "difference between outerDimH and innerDimH is not greater than window size for SIFT descriptor)";
            return -1;
        }

        if( (outerDimH - innerDimH) % 2 != 0)
        {
            qCDebug(DIGIKAM_FACESENGINE_LOG) << "shrinking innerDimH by 1 so outerDimH - innerDimH is divisible by 2";
            --innerDimH;
        }
*/
        paddingW = ((outerDimW - innerDimW) - 2*windowSize) / 2;
        paddingH = ((outerDimH - innerDimH) - 2*windowSize) / 2;
    }

    void loadTrainingData(const QString& path);
    void computeGaussian(std::vector<std::vector<float> > &Gaussian, int windowSize) const;

    /// Part 1: Fills originalFeatures from the image data
    void computeOriginalFeatures(std::vector<std::vector<std::vector<float> > > &originalFeatures,
                                 const cv::Mat& image,
                                 const int width, const int height) const;

    /// Part 2: Returns a small vector containg transformation parameters
    std::vector<float> computeTransform(const std::vector<std::vector<std::vector<float> > > &originalFeatures,
                                 const int width, const int height) const;

    /// Part 3: Applies the transformation (v, form computeTransform) to the given image, returns the result.
    cv::Mat applyTransform      (const cv::Mat& image,
                                 const std::vector<float> &v,
                                 int h, int w) const;

    // Utilities
    void getSIFTdescripter      (std::vector<float> &descripter,
                                 const std::vector<std::vector<float> > &m,
                                 const std::vector<std::vector<float> > &theta,
                                 int x, int y, int windowSize, int histDim, int bucketsDim,
                                 const std::vector<std::vector<float> > &Gaussian) const;
    float computeLogLikelihood  (const std::vector<std::vector<float> > &logDistField,
                                 const std::vector<std::vector<float> > &fids, int numFeatureClusters) const;
    void  getNewFeatsInvT       (std::vector<std::vector<float> > &newFIDs,
                                 const std::vector<std::vector<std::vector<float> > > &originalFeats,
                                 const std::vector<float> &vparams,
                                 float centerX, float centerY) const;

public:

    bool                                            isLoaded;

    const int                                       numParams;
    const int                                       windowSize;
    const int                                       maxProcessAtOnce;

    int                                             outerDimW;
    int                                             outerDimH;
    int                                             innerDimW;
    int                                             innerDimH;
    int                                             paddingW;
    int                                             paddingH;

    const int                                       siftHistDim;
    const int                                       siftBucketsDim;
    const int                                       siftDescDim;

    /// Training data
    int                                             numRandPxls;
    int                                             numFeatureClusters;
    int                                             edgeDescDim;
    std::vector<std::vector<float> >                centroids;
    std::vector<float>                              sigmaSq;
    std::vector<std::pair<int, int> >               randPxls;
    std::vector<std::vector<std::vector<float> > >  logDFSeq;
    std::vector<std::vector<float> >                Gaussian;
};

FunnelReal::FunnelReal()
    : d(new Private)
{
    QString trainingFile = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).last() + QString::fromLatin1("/") +
                           QString::fromLatin1("digikam/alignment-congealing/face-funnel.data");

    if (!QFileInfo(trainingFile).exists())
    {
        qCritical(DIGIKAM_FACESENGINE_LOG) << "Training data for Congealing/Funnel not found. Should be at" << trainingFile;
        return;
    }

    d->loadTrainingData(trainingFile);
}

FunnelReal::~FunnelReal()
{
    delete d;
}

cv::Mat FunnelReal::align(const cv::Mat& inputImage)
{
    if (!d->isLoaded)
    {
        return inputImage;
    }

    cv::Mat scaled, grey, image;
    // resize to outer window size
    cv::resize(inputImage, scaled, cv::Size(d->outerDimW, d->outerDimH), 0, 0,
               ( (d->outerDimW < inputImage.cols) ? cv::INTER_AREA : cv::INTER_CUBIC) );
    // ensure it's grayscale
    if (scaled.channels() > 1)
    {
        cvtColor(scaled, grey, CV_RGB2GRAY);
    }
    else
    {
        grey = scaled;
    }

    // convert to float
    grey.convertTo(image, CV_32F);

    const int height = image.rows - 2*d->windowSize;
    const int width  = image.cols - 2*d->windowSize;

    std::vector<std::vector<std::vector<float> > > originalFeatures;

    d->computeOriginalFeatures(originalFeatures, image, width, height);

    std::vector<float> v = d->computeTransform(originalFeatures, width, height);

    return d->applyTransform(inputImage, v, d->outerDimH, d->outerDimW);
}

void FunnelReal::Private::loadTrainingData(const QString& path)
{
    try
    {
        std::ifstream trainingInfo(path.toLocal8Bit().data());
        trainingInfo.exceptions(std::ifstream::badbit);

        trainingInfo >> numFeatureClusters >> edgeDescDim;

        std::vector<float> cRow(edgeDescDim, 0);
        centroids = std::vector<std::vector<float> >(numFeatureClusters, cRow);
        sigmaSq   = std::vector<float>(numFeatureClusters);

        for(int i = 0; i < numFeatureClusters; i++)
        {
            for(int j = 0; j < edgeDescDim; j++)
            {
                trainingInfo >> centroids[i][j];
            }

            trainingInfo >> sigmaSq[i];
        }

        trainingInfo >> numRandPxls;
        randPxls = std::vector<std::pair<int, int> >(numRandPxls);

        for(int j = 0; j < numRandPxls; j++)
            trainingInfo >> randPxls[j].first >> randPxls[j].second;

        std::vector<float>                dfCol(numFeatureClusters, 0);
        std::vector<std::vector<float> >  logDistField(numRandPxls, dfCol);

        int iteration;

        while(true)
        {
            trainingInfo >> iteration;

            if(trainingInfo.eof())
                break;

            for(int j = 0; j < numRandPxls; j++)
            {
                for(int i = 0; i < numFeatureClusters; i++)
                    trainingInfo >> logDistField[j][i];
            }

            logDFSeq.push_back(logDistField);
        }
    }
    catch (const std::ifstream::failure& e)
    {
        qCritical(DIGIKAM_FACESENGINE_LOG) << "Error loading Congealing/Funnel training data:" << e.what();
    }
    catch(...)
    {
        qCritical(DIGIKAM_FACESENGINE_LOG) << "Default exception";
    }

    computeGaussian(Gaussian, windowSize);

    isLoaded = true;
}

void FunnelReal::Private::computeGaussian(std::vector<std::vector<float> > &Gaussian, int windowSize) const
{
    for(int i = 0; i < 2*windowSize; i++)
    {
        std::vector<float> grow(2*windowSize);

        for(int j = 0; j < 2*windowSize; j++)
        {
            float ii = i-(windowSize-0.5f), jj = j-(windowSize-0.5f);
            grow[j]  = exp(-(ii*ii+jj*jj)/(2*windowSize*windowSize));
        }

        Gaussian.push_back(grow);
    }
}

static float dist(const std::vector<float> &a, const std::vector<float> &b)
{
    float r=0;

    for(int i = 0; i < (signed)a.size(); i++)
        r+=(a[i]-b[i])*(a[i]-b[i]);

    return r;
}

// Main function, part 1
void FunnelReal::Private::computeOriginalFeatures(std::vector<std::vector<std::vector<float> > > &originalFeatures,
                                                  const cv::Mat& image,
                                                  const int width, const int height) const
{
    std::vector<float>               ofEntry(edgeDescDim, 0);
    std::vector<std::vector<float> > ofCol(width, ofEntry);

    originalFeatures = std::vector<std::vector<std::vector<float> > >(height, ofCol);

    std::vector<float> SiftDesc(edgeDescDim);

    std::vector<float>               mtRow(image.cols);
    std::vector<std::vector<float> > m(image.rows, mtRow);
    std::vector<std::vector<float> > theta(image.rows, mtRow);
    float dx, dy;

    for(int j = 0; j < image.rows; j++)
    {
        const float *greaterRow, *lesserRow, *row;
        row = image.ptr<float>(j);

        if (j == 0)
        {
            greaterRow = image.ptr<float>(j+1);
            lesserRow  = row;
        }
        else if (j == image.rows-1)
        {
            greaterRow = row;
            lesserRow  = image.ptr<float>(j-1);
        }
        else
        {
            greaterRow = image.ptr<float>(j+1);
            lesserRow  = image.ptr<float>(j-1);
        }

        for(int k = 0; k < image.cols; k++)
        {
            dy = greaterRow[k] - lesserRow[k];

            if(k == 0)
            {
                dx = row[k+1] - row[k];
            }
            else if (k == image.cols-1)
            {
                dx = row[k] - row[k-1];
            }
            else
            {
                dx = row[k+1] - row[k-1];
            }

            m[j][k]     = (float)sqrt(dx*dx+dy*dy);
            theta[j][k] = (float)atan2(dy,dx) * 180.0f/M_PI;

            if(theta[j][k] < 0)
            {
                theta[j][k] += 360.0f;
            }
        }
    }

    for(int j = 0; j < height; j++)
    {
        for(int k = 0; k < width; k++)
        {
            getSIFTdescripter(SiftDesc, m, theta, j+windowSize, k+windowSize, windowSize,
                              siftHistDim, siftBucketsDim, Gaussian);
            originalFeatures[j][k] = SiftDesc;
        }
    }

    for(int j = 0; j < height; j++)
    {
        for(int k = 0; k < width; k++)
        {
            std::vector<float> distances(numFeatureClusters);
            float sum = 0;

            for(int ii = 0; ii < numFeatureClusters; ii++)
            {
                distances[ii] = exp(-dist(originalFeatures[j][k], centroids[ii])/(2*sigmaSq[ii]))/sqrt(sigmaSq[ii]);
                sum += distances[ii];
            }

            for(int ii = 0; ii < numFeatureClusters; ii++)
            {
                distances[ii] /= sum;
            }

            originalFeatures[j][k] = distances;
        }
    }
}

// Main function, part 2
std::vector<float> FunnelReal::Private::computeTransform(const std::vector<std::vector<std::vector<float> > > &originalFeatures,
                                                         const int width, const int height) const
{
    std::vector<float> v(numParams);

    std::vector<float>               fidsEntry(numFeatureClusters, 0);
    std::vector<std::vector<float> > featureIDs(numRandPxls, fidsEntry);

    std::vector<float>               nfEntry(numFeatureClusters, 0);
    std::vector<std::vector<float> > newFIDs(numRandPxls, nfEntry);
    float centerX = width/2.0f;
    float centerY = height/2.0f;

    float d[] = {1.0f, 1.0f, (float)M_PI/180.0f, 0.02f};

    getNewFeatsInvT(featureIDs, originalFeatures, v, centerX, centerY);

    for(uint iter=0; iter<logDFSeq.size(); iter++)
    {
        float oldL = computeLogLikelihood(logDFSeq[iter], featureIDs, numFeatureClusters);

        for(int k=0; k<numParams; k++)
        {
            float dn = ((qrand()%160)-80)/100.0f;
            if(k>1)
            {
                dn /= 100.0f;
            }
            v[k] += (d[k] + dn);

            getNewFeatsInvT(newFIDs, originalFeatures, v, centerX, centerY);
            float newL = computeLogLikelihood(logDFSeq[iter], newFIDs, numFeatureClusters);

            if(newL > oldL)
            {
                featureIDs = newFIDs;
                oldL       = newL;
            }
            else
            {
                v[k] -= (2*(d[k] + dn));
                getNewFeatsInvT(newFIDs, originalFeatures, v, centerX, centerY);
                newL = computeLogLikelihood(logDFSeq[iter], newFIDs, numFeatureClusters);

                if(newL > oldL)
                {
                    oldL       = newL;
                    featureIDs = newFIDs;
                }
                else
                {
                    v[k] += (d[k]+dn);
                }
            }
        }
    }

    return v;
}


cv::Mat FunnelReal::Private::applyTransform(const cv::Mat& image,
                                            const std::vector<float> &v, int h, int w) const
{
    float cropT1inv[2][3] = {{1,0,image.cols/2.0f}, {0,1,image.rows/2.0f}};
    float cropS1inv[3][3] = {{image.cols/(float)w,0,0}, {0,image.rows/(float)h,0}, {0,0,1}};
    float cropS2inv[3][3] = {{w/(float)image.cols,0,0}, {0,h/(float)image.rows,0}, {0,0,1}};
    float cropT2inv[3][3] = {{1,0,-image.cols/2.0f}, {0,1,-image.rows/2.0f}, {0,0,1}};

    float postM[3][3]     = {{1,0,w/2.0f}, {0,1,h/2.0f}, {0,0,1}};
    float preM[3][3]      = {{1,0,-w/2.0f}, {0,1,-h/2.0f}, {0,0,1}};

    float tM[3][3]        = {{1, 0, v[0]}, {0, 1, v[1]}, {0,0,1}};
    float rM[3][3]        = {{cosf(v[2]), -sinf(v[2]), 0}, {sinf(v[2]), cosf(v[2]), 0}, {0, 0, 1}};
    float sM[3][3]        = {{expf(v[3]), 0, 0}, {0, expf(v[3]), 0}, {0, 0, 1}};

    cv::Mat tCVM(3, 3, CV_32FC1, tM);
    cv::Mat rCVM(3, 3, CV_32FC1, rM);
    cv::Mat sCVM(3, 3, CV_32FC1, sM);

    cv::Mat postCVM(3, 3, CV_32FC1, postM);
    cv::Mat preCVM(3, 3, CV_32FC1, preM);

    cv::Mat cropT1invCVM(2,3, CV_32FC1, cropT1inv);
    cv::Mat cropS1invCVM(3,3, CV_32FC1, cropS1inv);
    cv::Mat cropS2invCVM(3,3, CV_32FC1, cropS2inv);
    cv::Mat cropT2invCVM(3,3, CV_32FC1, cropT2inv);

    cv::Mat xform(2, 3, CV_32FC1);

    xform = cropT1invCVM * cropS1invCVM;
    xform = xform * tCVM;
    xform = xform * rCVM;
    xform = xform * sCVM;
    xform = xform * cropS2invCVM;
    xform = xform * cropT2invCVM;

    cv::Mat dst;//(image.rows, image.cols, image.type())
    cv::warpAffine(image, dst, xform, image.size(), cv::WARP_INVERSE_MAP /*+ cv::WARP_FILL_OUTLIERS*/ + cv::INTER_CUBIC);
    return dst;
}

void FunnelReal::Private::getSIFTdescripter(std::vector<float> &descripter,
                                             const std::vector<std::vector<float> > &m,
                                             const std::vector<std::vector<float> > &theta,
                                             int x, int y, int windowSize,
                                             int histDim, int bucketsDim,
                                             const std::vector<std::vector<float> > &Gaussian) const
{
    for(int i=0; i<(signed)descripter.size(); i++)
        descripter[i]=0;

    int histDimWidth = 2*windowSize/histDim;
    float degPerBin  = 360.0f/bucketsDim;

    // weight magnitudes by Gaussian with sigma equal to half window
    std::vector<float> mtimesGRow(2*windowSize);
    std::vector<std::vector<float> > mtimesG(2*windowSize, mtimesGRow);

    for(int i=0; i<2*windowSize; i++)
    {
        for(int j=0; j<2*windowSize; j++)
        {
            int xx        = x+i-(windowSize-1);
            int yy        = y+j-(windowSize-1);
            mtimesG[i][j] = m[xx][yy] * Gaussian[i][j];
        }
    }

    // calculate descripter
    // using trilinear interpolation
    int histBin[2], histX[2], histY[2];
    float dX[2], dY[2], dBin[2];

    for(int i=0; i<2*windowSize; i++)
    {
        for(int j=0; j<2*windowSize; j++)
        {
            histX[0] = i/histDim; histX[1] = i/histDim;
            histY[0] = j/histDim; histY[1] = j/histDim;
            dX[1]    = 0;
            dY[1]    = 0;

            int iModHD    = i % histDim;
            int jModHD    = j % histDim;
            int histDimD2 = histDim/2;

            if( iModHD >= histDimD2 && i < 2*windowSize - histDimD2 )
            {
                histX[1] = histX[0] + 1;
                dX[1] = (iModHD + 0.5f - histDimD2) / histDim;
            }

            if( iModHD < histDimD2 && i >= histDimD2 )
            {
                histX[1] = histX[0] - 1;
                dX[1] = (histDimD2 + 0.5f - iModHD) / histDim;
            }

            if( jModHD >= histDimD2 && j < 2*windowSize - histDimD2 )
            {
                histY[1] = histY[0] + 1;
                dY[1] = (jModHD + 0.5f - histDimD2) / histDim;
            }

            if( jModHD < histDimD2 && j >= histDimD2)
            {
                histY[1] = histY[0] - 1;
                dY[1] = (histDimD2 + 0.5f - jModHD) / histDim;
            }

            dX[0] = 1.0f - dX[1];
            dY[0] = 1.0f - dY[1];

            float histAngle = theta[x+i-(windowSize-1)][y+j-(windowSize-1)];

            histBin[0] = (int)(histAngle / degPerBin);
            histBin[1] = (histBin[0]+1) % bucketsDim;
            dBin[1]    = (histAngle - histBin[0]*degPerBin) / degPerBin;
            dBin[0]    = 1.0f-dBin[1];

            for(int histBinIndex=0; histBinIndex<2; histBinIndex++)
            {
                for(int histXIndex=0; histXIndex<2; histXIndex++)
                {
                    for(int histYIndex=0; histYIndex<2; histYIndex++)
                    {
                        int histNum = histX[histXIndex]*histDimWidth + histY[histYIndex];
                        int bin = histBin[histBinIndex];
                        descripter[histNum*bucketsDim + bin] += (mtimesG[i][j] * dX[histXIndex] * dY[histYIndex] * dBin[histBinIndex]);
                    }
                }
            }
        }
    }

    // normalize
    // threshold values at .2, renormalize
    float sum = 0;

    for(int i=0; i<(signed)descripter.size(); i++)
        sum += descripter[i];

    if(sum < .0000001f)
    {
        //float dn = 1.0f / (signed)descripter.size(); // is unused, dont know
        for(int i=0; i<(signed)descripter.size(); i++)
            descripter[i] = 0;

        return;
    }

    for(int i=0; i<(signed)descripter.size(); i++)
    {
        descripter[i] /= sum;

        if(descripter[i] > .2f)
            descripter[i] = .2f;
    }

    sum = 0;

    for(int i=0; i<(signed)descripter.size(); i++)
        sum += descripter[i];

    for(int i=0; i<(signed)descripter.size(); i++)
        descripter[i] /= sum;
}

void FunnelReal::Private::getNewFeatsInvT(std::vector<std::vector<float> > &newFIDs,
                                          const std::vector<std::vector<std::vector<float> > > &originalFeats,
                                          const std::vector<float> &vparams,
                                          float centerX, float centerY) const
{
    int numFeats = newFIDs[0].size();
    std::vector<float> uniformDist(numFeats, 1.0f/numFeats);

    float postM[2][3] = {{1,0,centerX}, {0,1,centerY}};
    float preM[3][3]  = {{1,0,-centerX}, {0,1,-centerY}, {0,0,1}};

    float tM[3][3]    = {{1, 0, vparams[0]}, {0, 1, vparams[1]}, {0,0,1}};
    float rM[3][3]    = {{cosf(vparams[2]), -sinf(vparams[2]), 0}, {sinf(vparams[2]), cosf(vparams[2]), 0}, {0, 0, 1}};
    float sM[3][3]    = {{expf(vparams[3]), 0, 0}, {0, expf(vparams[3]), 0}, {0, 0, 1}};

    cv::Mat tCVM(3, 3, CV_32FC1, tM);
    cv::Mat rCVM(3, 3, CV_32FC1, rM);
    cv::Mat sCVM(3, 3, CV_32FC1, sM);

    cv::Mat postCVM(2, 3, CV_32FC1, postM);
    cv::Mat preCVM(3, 3, CV_32FC1, preM);

    cv::Mat xform(2, 3, CV_32FC1);
    xform = postCVM * tCVM;
    xform = xform * rCVM;
    xform = xform * sCVM;
    xform = xform * preCVM;

    int height = (signed)originalFeats.size();
    int width  = (signed)originalFeats[0].size();

    for(int i=0; i<(signed)newFIDs.size(); i++)
    {
        int j  = randPxls[i].first;
        int k  = randPxls[i].second;
        int nx = (int)(xform.at<float>(0)*k + xform.at<float>(1)*j + xform.at<float>(2) + 0.5f);
        int ny = (int)(xform.at<float>(3)*k + xform.at<float>(4)*j + xform.at<float>(5) + 0.5f);

        if(!(ny >= 0 && ny < height && nx >= 0 && nx < width))
            newFIDs[i] = uniformDist;
        else
            newFIDs[i] = originalFeats[ny][nx];
    }
}

float FunnelReal::Private::computeLogLikelihood(const std::vector<std::vector<float> > &logDistField,
                                                const std::vector<std::vector<float> > &fids,
                                                int numFeatureClusters) const
{
    float l = 0;

    for(int j=0; j<(signed)fids.size(); j++)
    {
        for(int i=0; i<numFeatureClusters; i++)
            l += fids[j][i] * logDistField[j][i];
    }

    return l;
}

} // namespace Digikam
