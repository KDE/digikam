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

static float nmsThreshold = 0.4;

OpenCVDNNFaceDetector::OpenCVDNNFaceDetector()
  : confidenceThreshold(0.5)
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
    inputImageSize = cv::Size(300,300);
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

cv::Mat OpenCVDNNFaceDetector::prepareForDetection(const QString& inputImagePath, cv::Size& paddedSize) const
{
    cv::Mat image = cv::imread(inputImagePath.toStdString()), imagePadded;

    float k = qMin(inputImageSize.width*1.0/image.cols, inputImageSize.height*1.0/image.rows);

    int newWidth = (int)(k*image.cols);
    int newHeight = (int)(k*image.rows);
    cv::resize(image, image, cv::Size(newWidth, newHeight));

    // Pad with black pixels
    int padX = (inputImageSize.width - newWidth) / 2;
    int padY = (inputImageSize.height - newHeight) / 2;
    cv::copyMakeBorder(image, imagePadded,
                       padY, padY,
                       padX, padX,
                       cv::BORDER_CONSTANT,
                       cv::Scalar(0,0,0));

    paddedSize = cv::Size(padX, padY);

    return imagePadded;
}

QList<QRect> OpenCVDNNFaceDetector::detectFaces(const cv::Mat& inputImage, const cv::Size& paddedSize)
{
    if (inputImage.empty())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Invalid image given, not detecting faces.";
        return QList<QRect>();
    }

    cv::Mat inputBlob = cv::dnn::blobFromImage(inputImage, scaleFactor, inputImageSize, meanValToSubtract, true, false);
    net.setInput(inputBlob);
    cv::Mat detection = net.forward();

    QList<QRect> results; 
    cv::Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());

    std::vector<float> goodConfidences, doubtConfidences, confidences;
    std::vector<cv::Rect> goodBoxes, doubtBoxes, boxes;

    for(int i = 0; i < detectionMat.rows; i++)
    {
        float confidence = detectionMat.at<float>(i, 2);

        if(confidence > confidenceThreshold)
        {
            float leftOffset = detectionMat.at<float>(i, 3);
            float topOffset = detectionMat.at<float>(i, 4);
            float rightOffset = detectionMat.at<float>(i, 5);
            float bottomOffset = detectionMat.at<float>(i, 6);

            int left = (int)(leftOffset*inputImageSize.width);
            int right = (int)(rightOffset*inputImageSize.width);
            int top = (int)(topOffset*inputImageSize.height);
            int bottom = (int)(bottomOffset*inputImageSize.height);

            cv::Rect bbox(left, top, right - left + 1, bottom - top + 1);

            if(left >= paddedSize.width && right <= inputImageSize.width - paddedSize.width - 1
               && top >= paddedSize.height && bottom <= inputImageSize.height - paddedSize.height - 1)
            {
                goodBoxes.push_back(bbox);
                goodConfidences.push_back(confidence);
                qDebug() << "Good rect = " << QRect(bbox.x, bbox.y, bbox.width, bbox.height) << ", conf = " << confidence;
            }
            else if(right > left && right > paddedSize.width && left < inputImageSize.width - paddedSize.width - 1
                    && bottom > top && bottom > paddedSize.height && top < inputImageSize.height - paddedSize.height - 1)
            {
                doubtBoxes.push_back(bbox);
                doubtConfidences.push_back(confidence);
                qDebug() << "Doubt rect = " << QRect(bbox.x, bbox.y, bbox.width, bbox.height) << ", conf = " << confidence;
            }
        }
    }

    if(goodBoxes.empty())
    {
        boxes = doubtBoxes;
        confidences = doubtConfidences;
    }
    else
    {
        boxes = goodBoxes;
        confidences = goodConfidences;
    }

    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, confidenceThreshold, nmsThreshold, indices);
    for (size_t i = 0; i < indices.size(); ++i)
    {
        int idx = indices[i];
        cv::Rect bbox = boxes[idx];
        results << QRect(bbox.x - paddedSize.width, bbox.y - paddedSize.height, bbox.width, bbox.height);
        cv::rectangle(inputImage, bbox, cv::Scalar(0,128,0));
    }

    // qDebug() << detectedConfidence << ", " << detectedPos;

    qDebug() << results;

    // cv::imshow("image", inputImage);
    // cv::waitKey(0);

    return results;
}

} // namespace Digikam
