/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date    2017-07-13
 * @brief   Face recognition using deep learning
 *
 * @section DESCRIPTION
 *
 * @author Copyright (C) 2017 by Yingjie Liu
 *         <a href="mailto:yingjiewudi at gmail dot com">yingjiewudi at gmail dot com</a>
 *
 * @section LICENSE
 *
 * Released to public domain under terms of the BSD Simplified license.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the organization nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *   See <http://www.opensource.org/licenses/bsd-license>
 *
 * ============================================================ */

#include "opencvdnnfacerecognizer.h"

// local includes

#include "libopencv.h"
#include "facedbaccess.h"
#include "facedb.h"
#include "dnnfacemodel.h"
#include "digikam_debug.h"

namespace Digikam
{

class OpenCVDNNFaceRecognizer::Private
{
public:

    Private()
        : threshold(15000.0),
          loaded(false)
    {
    }

public:

    DNNFaceModel& dnn()
    {
        if (!loaded)
        {
            m_dnn = FaceDbAccess().db()->dnnFaceModel();
            loaded = true;
        }

        return m_dnn;
    }

public:

    float             threshold;

private:

    DNNFaceModel      m_dnn;
    bool              loaded;
};

OpenCVDNNFaceRecognizer::OpenCVDNNFaceRecognizer()
    : d(new Private)
{
    setThreshold(0.8);
}

OpenCVDNNFaceRecognizer::~OpenCVDNNFaceRecognizer()
{
    delete d;
}

void OpenCVDNNFaceRecognizer::setThreshold(float threshold) const
{
    d->threshold    = threshold;
}

namespace
{
    enum
    {
        TargetInputSize = 256
    };
}

cv::Mat OpenCVDNNFaceRecognizer::prepareForRecognition(const QImage& inputImage)
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

int OpenCVDNNFaceRecognizer::recognize(const cv::Mat& inputImage)
{
    int predictedLabel = -1;
    double confidence  = 0;
    d->dnn()->predict(inputImage, predictedLabel, confidence);
    qCDebug(DIGIKAM_FACESENGINE_LOG) << predictedLabel << confidence;

    if (confidence > d->threshold)
    {
        return -1;
    }

    return predictedLabel;
}
//we can delete this function later
/*
void OpenCVDNNFaceRecognizer::train(const std::vector<cv::Mat>& images, const std::vector<int>& labels, const QString& context)
{
    if (images.empty() || labels.size() != images.size())
    {
        return;
    }

    d->dnn().update(images, labels, context);
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "dnnfaces Train: Adding model to Facedb, should not be invoked";
    // add to database waiting
    //FaceDbAccess().db()->updateDNNFaceModel(d->dnn());
}
*/
} // namespace Digikam
