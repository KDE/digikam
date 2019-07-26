/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2019-07-22
 * Description : Class to perform faces detection using OpenCV DNN module
 *
 * Copyright (C) 2019 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#include "opencvdnnfacedetector.h"

// Qt includes

#include <QtGlobal>
#include <QStandardPaths>
#include <qmath.h>

// Local includes

#include "digikam_debug.h"

// OpenCV includes

#include <opencv2/imgproc.hpp>
//#include <opencv2/highgui.hpp>

#include <cassert>

namespace Digikam
{

OpenCVDNNFaceDetector::OpenCVDNNFaceDetector()
  : confidenceThreshold(0.8)
{

    QString nnproto = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                             QLatin1String("digikam/facesengine/deploy.prototxt"));
    QString nnmodel = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                             QLatin1String("digikam/facesengine/res10_300x300_ssd_iter_140000_fp16.caffemodel"));
    
    qCDebug(DIGIKAM_FACEDB_LOG) << "nnproto " << nnproto << ", nnmodel " << nnmodel;

    net = cv::dnn::readNetFromCaffe(nnproto.toStdString(), nnmodel.toStdString());

    // As we use SSD, we need to set appropriate values for image color space and image size

    scaleFactor = 1.0;
    meanValToSubtract = cv::Scalar(104.0, 177.0, 123.0);
}

OpenCVDNNFaceDetector::~OpenCVDNNFaceDetector()
{
}

int OpenCVDNNFaceDetector::recommendedImageSizeForDetection()
{
    return 800;
}

cv::Mat OpenCVDNNFaceDetector::prepareForDetection(const QImage& inputImage) const
{
    if (inputImage.isNull() || !inputImage.size().isValid())
    {
        return cv::Mat();
    }

    QImage image(inputImage);
    int inputArea                    = image.width() * image.height();
    const int maxAcceptableInputArea = 1024*768;

    if (inputArea > maxAcceptableInputArea)
    {
        // Resize to 1024 * 768 (or comparable area for different aspect ratio)
        // Looking for scale factor z where A = w*z * h*z => z = sqrt(A/(w*h))
        qreal z          = qSqrt(qreal(maxAcceptableInputArea) / image.width() / image.height());
        QSize scaledSize = image.size() * z;
        image            = image.scaled(scaledSize, Qt::KeepAspectRatio);
    }

    //TODO: move to common utils, opentldrecognition
    cv::Mat cvImageWrapper, cvImage;

    // switch (image.format())
    // {
    //     case QImage::Format_RGB32:
    //     case QImage::Format_ARGB32:
    //     case QImage::Format_ARGB32_Premultiplied:
    //         // I think we can ignore premultiplication when converting to grayscale
    //         cvImageWrapper = cv::Mat(image.height(), image.width(), CV_8UC4, image.scanLine(0), image.bytesPerLine());
    //         cv::cvtColor(cvImageWrapper, cvImage, colorConversion);
    //         break;
    //     default:
            // image          = image.convertToFormat(QImage::Format_RGB888);
            // cvImageWrapper = cv::Mat(image.height(), image.width(), CV_8UC3, image.scanLine(0), image.bytesPerLine());
            // cv::cvtColor(cvImageWrapper, cvImage, colorConversion);
    //         break;
    // }

    image = image.convertToFormat(QImage::Format_RGB888);
    cvImageWrapper = cv::Mat(image.height(), image.width(), CV_8UC3, image.scanLine(0), image.bytesPerLine());
    
    cv::cvtColor(cvImageWrapper, cvImage, cv::COLOR_RGB2GRAY);

    cv::equalizeHist(cvImage, cvImage);

    cv::cvtColor(cvImage, cvImage, cv::COLOR_GRAY2BGR);

    return cvImage;
}

cv::Mat OpenCVDNNFaceDetector::prepareForDetection(const Digikam::DImg& inputImage) const
{
    if (inputImage.isNull() || !inputImage.size().isValid())
    {
        return cv::Mat();
    }

    Digikam::DImg image(inputImage);
    int inputArea                    = image.width() * image.height();
    const int maxAcceptableInputArea = 1024*768;

    if (inputArea > maxAcceptableInputArea)
    {
        // Resize to 1024 * 768 (or comparable area for different aspect ratio)
        // Looking for scale factor z where A = w*z * h*z => z = sqrt(A/(w*h))
        qreal z          = qSqrt(qreal(maxAcceptableInputArea) / image.width() / image.height());
        QSize scaledSize = image.size() * z;
        image            = image.smoothScale(scaledSize, Qt::KeepAspectRatio);
    }

    //TODO: move to common utils, opentldrecognition
    cv::Mat cvImageWrapper, cvImage;
    int type = image.sixteenBit() ? CV_16UC3 : CV_8UC3;
    type     = image.hasAlpha()   ? type     : type+8;

    switch (type)
    {
        case CV_8UC4:
        case CV_16UC4:
            // cvImageWrapper = cv::Mat(image.height(), image.width(), type, image.bits());
            // cv::cvtColor(cvImageWrapper, cvImage, colorConversion);
            // cvImage = cv::Mat(image.height(), image.width(), CV_8UC4, image.scanLine(0), image.bytesPerLine());
            // break;
            assert(0);
        case CV_8UC3:
        case CV_16UC3:
            // cvImageWrapper = cv::Mat(image.height(), image.width(), type, image.bits());
            // cv::cvtColor(cvImageWrapper, cvImage, colorConversion);
            cvImage = cv::Mat(image.height(), image.width(), type, image.bits());
            break;
    }

    // if (type == CV_16UC4 || type == CV_16UC3)
    // {
    //     cvImage.convertTo(cvImage, CV_8UC1, 1/255.0);
    // }

    // cv::equalizeHist(cvImage, cvImage);

    // cv::cvtColor(cvImage, cvImage, cv::COLOR_GRAY2BGR);

    return cvImage;
}


QList<QRect> OpenCVDNNFaceDetector::detectFaces(const cv::Mat& inputImage, const cv::Size& originalSize)
{
    if (inputImage.empty())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Invalid image given, not detecting faces.";
        return QList<QRect>();
    }

    // qDebug() << "original size " << inputImage.size().width << ", " << inputImage.size().height;

    // cv::imshow("image", inputImg);
    // cv::waitKey(0);

    cv::Mat inputBlob = cv::dnn::blobFromImage(inputImage, scaleFactor, cv::Size(originalSize.width, originalSize.height), meanValToSubtract);
    net.setInput(inputBlob);
    cv::Mat detection = net.forward();

    QList<QRect> results; 
    cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

    for(int i = 0; i < detectionMat.rows; i++)
    {
        float confidence = detectionMat.at<float>(i, 2);

        if(confidence > confidenceThreshold)
        {
            int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * originalSize.width);
            int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * originalSize.height);
            int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * originalSize.width);
            int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * originalSize.height);

            // qDebug() << detectionMat.at<float>(detectedPos, 3) << ", " << detectionMat.at<float>(detectedPos, 4) << ", " << detectionMat.at<float>(detectedPos, 5) << ", " << detectionMat.at<float>(detectedPos, 6);

            results << QRect(x1, y1, x2-x1, y2-y1);
        }
    }

    // qDebug() << detectedConfidence << ", " << detectedPos;

    // qDebug() << results.first();

    return results;
}

} // namespace Digikam
