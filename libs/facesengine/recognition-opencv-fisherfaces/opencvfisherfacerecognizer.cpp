/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date    2017-06-10
 * @brief   <a href="http://docs.opencv.org/2.4/modules/contrib/doc/facerec/facerec_tutorial.html#fisherfaces">Face Recognition based on Fisherfaces</a>
 *          Turk, Matthew A and Pentland, Alex P. "Face recognition using fisherfaces." 
 *          Computer Vision and Pattern Recognition, 1991. Proceedings {CVPR'91.},
 *          {IEEE} Computer Society Conference on 1991.
 *
 * @section DESCRIPTION
 *
 * @author Copyright (C) 2017 by Yingjie Liu
 *         <a href="mailto:yingjiewudi at gmail dot come">yingjiewudi at gmail dot come</a>
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

    FisherFaceModel     m_fisher;
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
    d->threshold    = threshold;
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
        return;
    }

    d->fisher().update(images, labels, context);
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Fisherfaces Train: Adding model to Facedb";
    // add to database waiting
    FaceDbAccess().db()->updateFISHERFaceModel(d->fisher());
}

} // namespace Digikam
