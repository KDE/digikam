/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : TBD
 * Description : A Red-Eye automatic detection and correction filter.
 *
 * Copyright (C) 2005-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2016      by Omar Amin <Omar dot moh dot amin at gmail dot com>
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

#include "redeyecorrectionfilter.h"

// OpenCV includes
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "facedetector.h"

// Qt includes

#include <QtConcurrent>
#include <QtMath>
#include <QMutex>
#include <iterator>
#include <QListIterator>


// Local includes
#include "digikam_debug.h"
//#include "opencv"

namespace Digikam
{

class RedEyeCorrectionFilter::Private
{
public:

    Private()
    {

    }

    FacesEngine::FaceDetector facedetector;

};

RedEyeCorrectionFilter::RedEyeCorrectionFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

RedEyeCorrectionFilter::RedEyeCorrectionFilter(DImg* const orgImage, QObject* const parent)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("RedEyeCorrection")),
      d(new Private)
{
    initFilter();
}

RedEyeCorrectionFilter::RedEyeCorrectionFilter(DImgThreadedFilter* const parentFilter,
                       const DImg& orgImage, const DImg& destImage,
                       int progressBegin, int progressEnd)
    : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                         parentFilter->filterName() + QLatin1String(": RedEyeCorrection")),
      d(new Private)
{
    filterImage();
}

RedEyeCorrectionFilter::~RedEyeCorrectionFilter()
{
      cancelFilter();
      delete d;
}


cv::Mat RedEyeCorrectionFilter::QImageToCvMat( const QImage &inImage, bool inCloneImageData )
{
    // Todo : Handle QImage 16 bit depth images or convert from DImg to cv::Mat directly
    switch ( inImage.format() )
    {
        // 8-bit, 4 channel
        case QImage::Format_RGB32:
        {
            cv::Mat  mat( inImage.height(), inImage.width(), CV_8UC4, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine() );

            return (inCloneImageData ? mat.clone() : mat);
        }

        // 8-bit, 3 channel
        case QImage::Format_RGB888:
        {
            if ( !inCloneImageData )
               qWarning() << "ASM::QImageToCvMat() - Conversion requires cloning since we use a temporary QImage";

            QImage   swapped = inImage.rgbSwapped();

            return cv::Mat( swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine() ).clone();
        }

         // 8-bit, 1 channel
        case QImage::Format_Indexed8:
        {
            cv::Mat  mat( inImage.height(), inImage.width(), CV_8UC1, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine() );

            return (inCloneImageData ? mat.clone() : mat);
        }

        default:
            qWarning() << "ASM::QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
            break;
    }

    return cv::Mat();
}


void RedEyeCorrectionFilter::filterImage()
{
    // Deep copy
    QImage temp = m_orgImage.copyQImage();


    int type = m_orgImage.sixteenBit()?CV_16UC3:CV_8UC3;
    type = m_orgImage.hasAlpha()?type:type+8;
    //cv::Mat intermediateImage = cv::Mat(cv::Size(m_orgImage.width(),m_orgImage.height()),
    //                                    type,m_orgImage.bits());
    // intermediateImage.data = m_orgImage.bits();


    QList<QRectF> faces = d->facedetector.detectFaces(temp,temp.size());
    QList<cv::Rect> cvfaces;
    QRectFtocvRect(faces, cvfaces);

    // Shallow Copy
    cv::Mat outputimage = QImageToCvMat(temp);

    // Todo : replace this code with eye detection and correction
    drawRects(outputimage, cvfaces);

    // Shallow Copy
    m_destImage.putImageData(m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit(),
                             m_orgImage.hasAlpha(), outputimage.data, false);


}

void RedEyeCorrectionFilter::drawRects(cv::Mat &image, const QList<cv::Rect> & rects)
{
    QListIterator<cv::Rect> listit(rects);

    while(listit.hasNext())
    {
        cv::Rect temp = listit.next();
        cv::rectangle(image, temp, cv::Scalar(0,0,255));
    }
}

void RedEyeCorrectionFilter::QRectFtocvRect(const QList<QRectF> & faces, QList<cv::Rect> & result)
{
    QListIterator<QRectF> listit(faces);

    while(listit.hasNext())
    {
        QRectF  temp = listit.next();
        result.append(cv::Rect(temp.topLeft().rx(), temp.topLeft().ry(),
                               temp.width()       , temp.height()) );
    }
}

void RedEyeCorrectionFilter::correctRedEye(cv::Mat & eye)
{
    // Todo : handle different images depth
    for(int i = 0 ; i<eye.rows; i++)
        for(int j = 0; j<eye.cols; j++)
        {
            cv::Vec3b intensity = eye.at<cv::Vec3b>(i, j);
            float redIntensity = ((float)intensity.val[2] / ((intensity.val[1] + intensity.val[0]) / 2));
            if (redIntensity > 2.1f)
            {
                // reduce red to the average of blue and green
                intensity.val[2] = (intensity.val[1] + intensity.val[0]) / 2;
                eye.at<cv::Vec3b>(i, j) = intensity;
            }
        }

}


FilterAction RedEyeCorrectionFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    return action;
}

void RedEyeCorrectionFilter::readParameters(const FilterAction& action)
{
    // if there'll be parameters
}

}  // namespace Digikam
