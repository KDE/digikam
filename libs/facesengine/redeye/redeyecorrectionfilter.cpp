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
#include <QImage>
#include <QDebug>
#include <QDataStream>


// Local includes
#include "digikam_debug.h"
#include "shapepredictor.h"
//#include "opencv"
#include "facedetector.h"


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
    // Todo:move the deserialization into a single place
    // preparing shape predictor
    redeye::shapepredictor sp;


    // Loading the shape predictor model
    QList<QString> path = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                             QString::fromLatin1("digikam/facesengine"),
                                             QStandardPaths::LocateDirectory);
    QFile model(*path.begin()+QString("/shape-predictor.dat"));
    cv::Mat intermediateImage;
    model.open(QIODevice::ReadOnly);
    QDataStream dataStream(&model);
    dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    dataStream>>sp;

    bool visualize = false;



    // Todo: convert dImg to Opencv::Mat directly
    // Deep copy
    QImage temp = m_orgImage.copyQImage();

    //
    int type = m_orgImage.sixteenBit()?CV_16UC3:CV_8UC3;
    // Todo : converting to Qimage including adding an alpha channel
    // to be handled
    type = type+8;//m_orgImage.hasAlpha()?type:type+8;
    intermediateImage = cv::Mat(cv::Size(temp.width(),temp.height()),
                                        type,temp.bits());
    cv::Mat gray;
    cv::cvtColor(intermediateImage,gray,CV_RGBA2GRAY);

    QList<QRectF> qrectfdets = d->facedetector.detectFaces(temp);
    if(qrectfdets.size() != 0)
    {
        std::vector<cv::Rect> dets;
        QList<QRect> qrectdets =
                FacesEngine::FaceDetector::toAbsoluteRects(qrectfdets,temp.size());
        QRectFtocvRect(qrectdets,dets);

        drawRects(intermediateImage,dets);

        // Eye Detection
        for(unsigned int i = 0;i<dets.size();i++)
        {
            fullobjectdetection object = sp(gray,dets[i]);
            std::vector<cv::Rect> eyes = geteyes(object);
            drawRects(intermediateImage,eyes);
            for(unsigned int j = 0;j<eyes.size();j++)
            {
                correctRedEye(intermediateImage.data,
                              intermediateImage.type(),
                              eyes[j],
                              cv::Rect(0,0,intermediateImage.size().width ,
                                           intermediateImage.size().height));
//                correctRedEye(intermediateImage[eyes[j]],
//                              intermediateImage.type(),
//                              cv::Rect(0,0,intermediateImage.size().width,
//                                       intermediateImage.size().height));
            }


        }
    }
    m_destImage.putImageData(m_orgImage.width(), m_orgImage.height(), m_orgImage.sixteenBit(),
                             true/*m_orgImage.hasAlpha()*/, intermediateImage.data, true);


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

void RedEyeCorrectionFilter::drawRects(cv::Mat &image, const std::vector<cv::Rect> & rects)
{
    for(unsigned int i =0;i<rects.size();i++)
    {
        cv::Rect temp = rects[i];
        cv::rectangle(image, temp, cv::Scalar(0,0,255));
    }
}


void RedEyeCorrectionFilter::QRectFtocvRect(const QList<QRect> & faces, std::vector<cv::Rect> & result)
{
    QListIterator<QRect> listit(faces);

    while(listit.hasNext())
    {
        QRect  temp = listit.next();
        result.push_back(cv::Rect(temp.topLeft().rx(), temp.topLeft().ry(),
                                  temp.width()       , temp.height()) );
    }
}

void RedEyeCorrectionFilter::correctRedEye(cv::Mat & eye, int type, cv::Rect imgRect)
{
    // Todo : handle different images depth
    uchar  * onebytedata  = eye.data;
    //ushort * twobytedata = (ushort*)eye.data;
    int pixeldepth;
    if(type == CV_8UC3 || type == CV_16UC3 )
    {
        pixeldepth = 3;
    }
    else if(type == CV_8UC4 || type == CV_16UC4)
    {
        pixeldepth = 4;
    }
    else
    {
        qDebug()<<"\nInsupported Type in redeye correction function";
    }

    //bool sixteendepth = type == CV_8UC3 || type == CV_8UC4 ? false:true;
    uchar * globalindex = eye.data;
    for(int i = 0 ; i<eye.rows; i++)
    {
        for(int j = 0; j<eye.cols; j++)
        {
            int pixelindex = j*pixeldepth;
            onebytedata = &(((uchar*)  globalindex)[pixelindex]);
            //twobytedata = &(((ushort*) globalindex)[pixelindex]);
            onebytedata[0] = 0;   // R
            onebytedata[1] = 255; // G
            onebytedata[2] = 0;   // B
//            if(sixteendepth)
//            {
//                float redIntensity = ((float)twobytedata[0] / ((twobytedata[1] + twobytedata[2]) / 2));
//                if (redIntensity > 2.1f)
//                {
//                    // reduce red to the average of blue and green
//                    twobytedata[0] = (twobytedata[1] + twobytedata[2]) / 2;
//                }
//            }
//            else
//            {
//                float redIntensity = ((float)onebytedata[0] / ((onebytedata[1] + onebytedata[2]) / 2));
//                if (redIntensity > 2.1f)
//                {
//                    // reduce red to the average of blue and green
//                    onebytedata[0] = (onebytedata[1] + onebytedata[2]) / 2;
//                }
//            }

        }
        globalindex = globalindex + imgRect.width*pixeldepth;
    }

}

void RedEyeCorrectionFilter::correctRedEye(uchar * data, int type,
                                           cv::Rect eyerect, cv::Rect imgRect)
{
    // Todo : handle different images depth
    uchar  * onebytedata  = data;
    ushort * twobytedata = (ushort*)data;
    int pixeldepth;
    if(type == CV_8UC3 || type == CV_16UC3 )
    {
        pixeldepth = 3;
    }
    else if(type == CV_8UC4 || type == CV_16UC4)
    {
        pixeldepth = 4;
    }
    else
    {
        qDebug()<<"\nInsupported Type in redeye correction function";
    }

    bool sixteendepth = type == CV_8UC3 || type == CV_8UC4 ? false:true;
    //uchar * globalindex = eye.data;

    for(int i = eyerect.y ; i<eyerect.y + eyerect.height; i++)
    {
        for(int j = eyerect.x; j<eyerect.x + eyerect.width; j++)
        {
            int pixelindex = (i*imgRect.width + j) * pixeldepth;
            onebytedata = &(((uchar*)  data)[pixelindex]);
//            twobytedata = &(((ushort*) data)[pixelindex]);
//            if(i>imgRect.height || j > imgRect.width)
//            {
//                qDebug()<<"row or column wrong\n";
//            }
//            if(pixelindex/4 > imgRect.width * imgRect.height)
//            {
//                qDebug()<<"pixel index out of context\n";
//            }
            if(sixteendepth)
            {
                float redIntensity = ((float)twobytedata[0] / ((twobytedata[1] + twobytedata[2]) / 2));
                if (redIntensity > 2.1f)
                {
                    // reduce red to the average of blue and green
                    twobytedata[0] = (twobytedata[1] + twobytedata[2]) / 2;
                }
            }
            else
            {

                float redIntensity = ((float)onebytedata[2] / (((int)onebytedata[1] + (int)onebytedata[0]) / 2));
                if (redIntensity > 2.1f)
                {
                    // reduce red to the average of blue and green
                    onebytedata[2] = ((int)onebytedata[1] + (int)onebytedata[0]) / 2;

                }
            }

        }
        //globalindex = globalindex + imgRect.width*pixeldepth;
    }

}


FilterAction RedEyeCorrectionFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    return action;
}

void RedEyeCorrectionFilter::readParameters(const FilterAction&)
{

}

}  // namespace Digikam
