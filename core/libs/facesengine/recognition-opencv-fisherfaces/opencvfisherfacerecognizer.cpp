/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-06-10
 * Description : Face Recognition based on Fisherfaces
 *               http://docs.opencv.org/2.4/modules/contrib/doc/facerec/facerec_tutorial.html#Fisherfaces
 *               Turk, Matthew A and Pentland, Alex P. "Face recognition using Fisherfaces." 
 *               Computer Vision and Pattern Recognition, 1991. Proceedings {CVPR'91.},
 *               {IEEE} Computer Society Conference on 1991.
 *
 * Copyright (C) 2017      by Yingjie Liu <yingjiewudi at gmail dot com>
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "opencvfisherfacerecognizer.h"

// local includes

#include "libopencv.h"
#include "facedbaccess.h"
#include "facedb.h"
#include "fisherfacemodel.h"
#include "digikam_debug.h"

namespace Digikam
{

class OpenCVFISHERFaceRecognizer::Private
{
public:

    Private()
        : threshold(25000.0),
          loaded(false)
    {
    }

public:

    FisherFaceModel& fisher()
    {
        if (!loaded)
        {
            m_fisher = FaceDbAccess().db()->fisherFaceModel();
            loaded = true;
        }

        return m_fisher;
    }

public:

    float             threshold;

private:

    FisherFaceModel   m_fisher;
    bool              loaded;
};

OpenCVFISHERFaceRecognizer::OpenCVFISHERFaceRecognizer()
    : d(new Private)
{
    setThreshold(25000.0);
}

OpenCVFISHERFaceRecognizer::~OpenCVFISHERFaceRecognizer()
{
    delete d;
}

void OpenCVFISHERFaceRecognizer::setThreshold(float threshold) const
{
    d->threshold = threshold;
}

namespace
{
    enum
    {
        TargetInputSize = 256
    };
}

cv::Mat OpenCVFISHERFaceRecognizer::prepareForRecognition(const QImage& inputImage)
{
    QImage image(inputImage);

    if (inputImage.width() > TargetInputSize || inputImage.height() > TargetInputSize)
    {
        image = inputImage.scaled(TargetInputSize, TargetInputSize, Qt::IgnoreAspectRatio);
    }

    cv::Mat cvImage = cv::Mat(image.height(), image.width(), CV_8UC1);
    cv::Mat cvImageWrapper;

    switch (image.format())
    {
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
            // I think we can ignore premultiplication when converting to grayscale
            cvImageWrapper = cv::Mat(image.height(), image.width(), CV_8UC4, image.scanLine(0), image.bytesPerLine());
            cvtColor(cvImageWrapper, cvImage, CV_RGBA2GRAY);
            break;
        default:
            image          = image.convertToFormat(QImage::Format_RGB888);
            cvImageWrapper = cv::Mat(image.height(), image.width(), CV_8UC3, image.scanLine(0), image.bytesPerLine());
            cvtColor(cvImageWrapper, cvImage, CV_RGB2GRAY);
            break;
    }

    //resize(cvImage, cvImage, Size(256, 256), (0, 0), (0, 0), INTER_LINEAR);
    equalizeHist(cvImage, cvImage);
    return cvImage;
}

int OpenCVFISHERFaceRecognizer::recognize(const cv::Mat& inputImage)
{
    int predictedLabel = -1;
    double confidence  = 0;
    d->fisher()->predict(inputImage, predictedLabel, confidence);
    qCDebug(DIGIKAM_FACESENGINE_LOG) << predictedLabel << confidence;

    if (confidence > d->threshold)
    {
        return -1;
    }

    return predictedLabel;
}

void OpenCVFISHERFaceRecognizer::train(const std::vector<cv::Mat>& images, const std::vector<int>& labels, const QString& context)
{
    if (images.empty() || labels.size() != images.size())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "Fisherfaces Train: nothing to train...";
        return;
    }

    d->fisher().update(images, labels, context);
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Fisherfaces Train: Adding model to Facedb";
    // add to database waiting
}

} // namespace Digikam
