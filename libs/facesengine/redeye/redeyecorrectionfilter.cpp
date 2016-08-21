/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 17-8-2016
 * Description : A Red-Eye automatic detection and correction filter.
 *
 * Copyright (C) 2005-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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



// Qt includes

#include <QtConcurrent>
#include <QtMath>
#include <QMutex>
#include <iterator>
#include <QListIterator>
#include <QImage>
#include <QDataStream>

// Local includes
#include "digikam_debug.h"
#include "facedetector.h"
#include "redeyecorrectionfilter.h"
#include "shapepredictor.h"


namespace Digikam
{

class RedEyeCorrectionFilter::Private
{
public:

    Private()
    {
    }

    FacesEngine::FaceDetector      facedetector;
    static redeye::shapepredictor* sp;
};

redeye::shapepredictor * RedEyeCorrectionFilter::Private::sp = 0;

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

cv::Mat RedEyeCorrectionFilter::QImageToCvMat(const QImage& inImage, bool inCloneImageData)
{
    // TODO : Handle QImage 16 bit depth images or convert from DImg to cv::Mat directly

    switch (inImage.format())
    {
        // 8-bit, 4 channel
        case QImage::Format_RGB32:
        {
            cv::Mat mat(inImage.height(), inImage.width(), CV_8UC4, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine());

            return (inCloneImageData ? mat.clone() : mat);
        }

        // 8-bit, 3 channel
        case QImage::Format_RGB888:
        {
            if ( !inCloneImageData )
            {
               qCWarning(DIGIKAM_FACESENGINE_LOG) << "ASM::QImageToCvMat() - Conversion requires cloning since we use a temporary QImage";
            }

            QImage swapped = inImage.rgbSwapped();

            return cv::Mat(swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine()).clone();
        }

         // 8-bit, 1 channel
        case QImage::Format_Indexed8:
        {
            cv::Mat mat(inImage.height(), inImage.width(), CV_8UC1, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine());

            return (inCloneImageData ? mat.clone() : mat);
        }

        default:
            qCWarning(DIGIKAM_FACESENGINE_LOG) << "ASM::QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
            break;
    }

    return cv::Mat();
}

void RedEyeCorrectionFilter::filterImage()
{

    if (d->sp == 0)
    {
        // Loading the shape predictor model
        redeye::shapepredictor* const temp = new redeye::shapepredictor();

        QList<QString> path = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                        QString::fromLatin1("digikam/facesengine"),
                                                        QStandardPaths::LocateDirectory);
        QFile model(*path.begin()+QLatin1String("/shapepredictor.dat"));

        model.open(QIODevice::ReadOnly);
        if(model.isOpen())
        {
            QDataStream dataStream(&model);
            dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
            dataStream>>*temp;
            d->sp = temp;
        }
    }


    cv::Mat intermediateImage;

    // Todo: convert dImg to Opencv::Mat directly
    // Deep copy
    QImage temp = m_orgImage.copyQImage();

//    int type = m_orgImage.sixteenBit()?CV_8UC3:CV_8UC3;
//    // TODO : converting to Qimage including adding an alpha channel
//    // to be handled
//    type = type+8;//m_orgImage.hasAlpha()?type:type+8;
    intermediateImage = cv::Mat(cv::Size(temp.width(),temp.height()), CV_8UC4,temp.bits());
    cv::Mat gray;
    cv::cvtColor(intermediateImage,gray,CV_RGBA2GRAY);

    QList<QRectF> qrectfdets   = d->facedetector.detectFaces(temp);
    redeye::shapepredictor& sp = *(d->sp);

    if (runningFlag() && (qrectfdets.size() != 0))
    {
        std::vector<cv::Rect> dets;
        QList<QRect> qrectdets = FacesEngine::FaceDetector::toAbsoluteRects(qrectfdets,temp.size());
        QRectFtocvRect(qrectdets,dets);

        //drawRects(intermediateImage,dets);

        // Eye Detection
        for (unsigned int i = 0 ; runningFlag() && (i < dets.size()) ; i++)
        {
            fullobjectdetection object = sp(gray,dets[i]);
            std::vector<cv::Rect> eyes = geteyes(object);
            //drawRects(intermediateImage,eyes);

            for (unsigned int j = 0 ; runningFlag() && (j < eyes.size()) ; j++)
            {
                correctRedEye(intermediateImage.data,
                              intermediateImage.type(),
                              eyes[j],
                              cv::Rect(0,0,intermediateImage.size().width ,
                                           intermediateImage.size().height));
            }
        }
    }

    if (runningFlag())
    {
        m_destImage.putImageData(m_orgImage.width(), m_orgImage.height(), false, //m_orgImage.sixteenBit(),
                                 true/*m_orgImage.hasAlpha()*/, intermediateImage.data, true);

        if (m_orgImage.sixteenBit())
            m_destImage.convertDepth(64);

        //if(!m_orgImage.hasAlpha())  m_destImage.removeAlphaChannel();
    }
}

void RedEyeCorrectionFilter::drawRects(cv::Mat& image, const QList<cv::Rect>& rects)
{
    QListIterator<cv::Rect> listit(rects);

    while (listit.hasNext())
    {
        cv::Rect temp = listit.next();
        cv::rectangle(image, temp, cv::Scalar(0,0,255));
    }
}

void RedEyeCorrectionFilter::drawRects(cv::Mat& image, const std::vector<cv::Rect>& rects)
{
    for (unsigned int i = 0 ; i < rects.size() ; i++)
    {
        cv::Rect temp = rects[i];
        cv::rectangle(image, temp, cv::Scalar(0, 0, 255));
    }
}

void RedEyeCorrectionFilter::QRectFtocvRect(const QList<QRect>& faces, std::vector<cv::Rect>& result)
{
    QListIterator<QRect> listit(faces);

    while (listit.hasNext())
    {
        QRect  temp = listit.next();
        result.push_back(cv::Rect(temp.topLeft().rx(), temp.topLeft().ry(),
                                  temp.width()       , temp.height()) );
    }
}

void RedEyeCorrectionFilter::correctRedEye(cv::Mat& eye, int type, cv::Rect imgRect)
{
    // TODO : handle different images depth
    uchar*  onebytedata = eye.data;
    //ushort* twobytedata = (ushort*)eye.data;
    int     pixeldepth  = 0;

    if (type == CV_8UC3 || type == CV_16UC3 )
    {
        pixeldepth = 3;
    }
    else if(type == CV_8UC4 || type == CV_16UC4)
    {
        pixeldepth = 4;
    }
    else
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "\nInsupported Type in redeye correction function";
        return;
    }

    //bool sixteendepth = type == CV_8UC3 || type == CV_8UC4 ? false:true;
    uchar* globalindex = eye.data;

    for (int i = 0 ; i < eye.rows ; i++)
    {
        for (int j = 0 ; j < eye.cols ; j++)
        {
            int pixelindex = j * pixeldepth;
            onebytedata    = &(((uchar*)globalindex)[pixelindex]);
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

void RedEyeCorrectionFilter::correctRedEye(uchar* data, int type,
                                           cv::Rect eyerect, cv::Rect imgRect)
{
    // TODO : handle different images depth
    uchar*  onebytedata = data;
    ushort* twobytedata = reinterpret_cast<ushort*>(data);
    int     pixeldepth  = 0;

    if (type == CV_8UC3 || type == CV_16UC3 )
    {
        pixeldepth = 3;
    }
    else if (type == CV_8UC4 || type == CV_16UC4)
    {
        pixeldepth = 4;
    }
    else
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "\nInsupported Type in redeye correction function";
    }

    bool sixteendepth = type == CV_8UC3 || type == CV_8UC4 ? false:true;
    //uchar* globalindex = eye.data;

    for (int i = eyerect.y ; i < eyerect.y + eyerect.height ; i++)
    {
        for (int j = eyerect.x ; j < eyerect.x + eyerect.width ; j++)
        {
            int pixelindex = (i*imgRect.width + j) * pixeldepth;
            onebytedata    = &(reinterpret_cast<uchar*> (data)[pixelindex]);
            twobytedata    = &(reinterpret_cast<ushort*>(data)[pixelindex]);

            if (sixteendepth)
            {
                float redIntensity = ((float)twobytedata[0] / (((unsigned int)twobytedata[1]
                                                               +(unsigned int)twobytedata[2]) / 2));
                if (redIntensity > 2.1F)
                {
                    // reduce red to the average of blue and green
                    twobytedata[2] = (twobytedata[1] + twobytedata[2]) / 2;
                }
            }
            else
            {

                float redIntensity = ((float)onebytedata[2] / (( (unsigned int)onebytedata[1]
                                                               + (unsigned int)onebytedata[0]) / 2));
                if (redIntensity > 2.1F)
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
