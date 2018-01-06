/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 17-8-2016
 * Description : A Red-Eye automatic detection and correction filter.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// C++ includes

#include <iterator>

// Qt includes

#include <QFile>
#include <QDataStream>
#include <QListIterator>
#include <QStandardPaths>

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

    FaceDetector      facedetector;
    static redeye::ShapePredictor* sp;

    RedEyeCorrectionContainer      settings;
};

redeye::ShapePredictor* RedEyeCorrectionFilter::Private::sp = 0;

RedEyeCorrectionFilter::RedEyeCorrectionFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

RedEyeCorrectionFilter::RedEyeCorrectionFilter(DImg* const orgImage, QObject* const parent,
                                               const RedEyeCorrectionContainer& settings)
    : DImgThreadedFilter(orgImage, parent,
                         QLatin1String("RedEyeCorrection")),
      d(new Private)
{
    d->settings = settings;
    initFilter();
}

RedEyeCorrectionFilter::RedEyeCorrectionFilter(const RedEyeCorrectionContainer& settings,
                                               DImgThreadedFilter* const parentFilter,
                                               const DImg& orgImage, const DImg& destImage,
                                               int progressBegin, int progressEnd)
    : DImgThreadedFilter(parentFilter, orgImage, destImage,
                         progressBegin, progressEnd,
                         parentFilter->filterName() + QLatin1String(": RedEyeCorrection")),
      d(new Private)
{
    d->settings = settings;
    filterImage();
}

RedEyeCorrectionFilter::~RedEyeCorrectionFilter()
{
    cancelFilter();
    delete d;
}

void RedEyeCorrectionFilter::filterImage()
{
    if (!d->sp)
    {
        // Loading the shape predictor model

        QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              QLatin1String("digikam/facesengine/shapepredictor.dat"));

        QFile model(path);

        if (model.open(QIODevice::ReadOnly))
        {
            redeye::ShapePredictor* const temp = new redeye::ShapePredictor();
            QDataStream dataStream(&model);
            dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
            dataStream >> *temp;
            d->sp = temp;
        }
        else
        {
            qCDebug(DIGIKAM_DIMG_LOG) << "Error open file shapepredictor.dat";
            return;
        }
    }

    cv::Mat intermediateImage;
    // Deep copy
    DImg temp         = m_orgImage.copy();
    int type          = m_orgImage.sixteenBit() ? CV_16UC3 : CV_8UC3;
    type              = m_orgImage.hasAlpha()   ? type     : type + 8;

    intermediateImage = cv::Mat(m_orgImage.height(), m_orgImage.width(),
                                type, m_orgImage.bits());

    cv::Mat gray;

    if (type == CV_8UC3 || type == CV_16UC3)
    {
        cv::cvtColor(intermediateImage, gray, CV_RGB2GRAY);  // 3 channels
    }
    else
    {
        cv::cvtColor(intermediateImage, gray, CV_RGBA2GRAY); // 4 channels
    }

    if (type == CV_16UC3 || type == CV_16UC4)
    {
        gray.convertTo(gray, CV_8UC1, 1 / 255.0);
    }

    QList<QRectF> qrectfdets   = d->facedetector.detectFaces(temp);
    redeye::ShapePredictor& sp = *(d->sp);

    if (runningFlag() && (qrectfdets.size() != 0))
    {
        std::vector<cv::Rect> dets;
        QList<QRect> qrectdets = FaceDetector::toAbsoluteRects(qrectfdets, temp.size());
        QRectFtocvRect(qrectdets, dets);

        // Eye Detection
        for (unsigned int i = 0 ; runningFlag() && (i < dets.size()) ; i++)
        {
            FullObjectDetection object = sp(gray,dets[i]);
            std::vector<cv::Rect> eyes = geteyes(object);

            for (unsigned int j = 0 ; runningFlag() && (j < eyes.size()) ; j++)
            {
                correctRedEye(intermediateImage.data,
                              intermediateImage.type(),
                              eyes[j],
                              cv::Rect(0, 0, intermediateImage.size().width ,
                                             intermediateImage.size().height));
            }
        }
    }

    if (runningFlag())
    {
        m_destImage.putImageData(m_orgImage.width(), m_orgImage.height(), temp.sixteenBit(),
                                 !temp.hasAlpha(), intermediateImage.data, true);
    }
}

void RedEyeCorrectionFilter::correctRedEye(uchar* data, int type,
                                           cv::Rect eyerect, cv::Rect imgRect)
{
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
        qCDebug(DIGIKAM_DIMG_LOG) << "Insupported Type in redeye correction filter";
    }

    bool sixteendepth = (type == CV_8UC3) || (type == CV_8UC4) ? false : true;
    double redratio   = d->settings.m_redToAvgRatio;

    for (int i = eyerect.y ; i < eyerect.y + eyerect.height ; i++)
    {
        for (int j = eyerect.x ; j < eyerect.x + eyerect.width ; j++)
        {
            int pixelindex = (i*imgRect.width + j) * pixeldepth;
            onebytedata    = &(reinterpret_cast<uchar*> (data)[pixelindex]);
            twobytedata    = &(reinterpret_cast<ushort*>(data)[pixelindex]);

            if (sixteendepth)
            {
                float redIntensity = ((float)twobytedata[2] / (( (unsigned int)twobytedata[1]
                                                               + (unsigned int)twobytedata[0]) / 2));
                if (redIntensity > redratio)
                {
                    // reduce red to the average of blue and green
                    twobytedata[2] = ((int)twobytedata[1] + (int)twobytedata[0]) / 2;
                }
            }
            else
            {
                float redIntensity = ((float)onebytedata[2] / (( (unsigned int)onebytedata[1]
                                                               + (unsigned int)onebytedata[0]) / 2));
                if (redIntensity > redratio)
                {
                    // reduce red to the average of blue and green
                    onebytedata[2] = ((int)onebytedata[1] + (int)onebytedata[0]) / 2;
                }
            }
        }
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

FilterAction RedEyeCorrectionFilter::filterAction()
{
    DefaultFilterAction<RedEyeCorrectionFilter> action;
    d->settings.writeToFilterAction(action);
    return action;
}

void RedEyeCorrectionFilter::readParameters(const FilterAction& action)
{
    d->settings = RedEyeCorrectionContainer::fromFilterAction(action);
}

}  // namespace Digikam
