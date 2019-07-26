/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-07-13
 * Description : Face recognition using deep learning
 *
 * Copyright (C) 2017      by Yingjie Liu <yingjiewudi at gmail dot com>
 * Copyright (C) 2017-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "opencvdnnfacerecognizer.h"

// Local includes

#include "digikam_opencv.h"
#include "facedbaccess.h"
#include "facedb.h"
#include "dnnfacemodel.h"
#include "digikam_debug.h"
#include "dnnfaceextractor.h"
#include "recognitionpreprocessor.h"

namespace Digikam
{

float OpenCVDNNFaceRecognizer::m_threshold = 15000.0;

class Q_DECL_HIDDEN OpenCVDNNFaceRecognizer::Private
{
public:

    explicit Private()
        : loaded(false),
          m_preprocessor(0)
    {
    }

    ~Private()
    {
        delete m_preprocessor;
        delete m_extractor;
    }

public:

    DNNFaceModel& dnn()
    {
        if (!loaded)
        {
            m_dnn  = FaceDbAccess().db()->dnnFaceModel();

            m_preprocessor = new RecognitionPreprocessor;
            m_preprocessor->init(PreprocessorSelection::OPENFACE);

            m_extractor = new DNNFaceExtractor(m_preprocessor);

            loaded = true;
        }

        return m_dnn;
    }

public:

    RecognitionPreprocessor* m_preprocessor;
    DNNFaceExtractor* m_extractor;

private:

    DNNFaceModel m_dnn;
    bool         loaded;
};

OpenCVDNNFaceRecognizer::OpenCVDNNFaceRecognizer()
    : d(new Private)
{
}

OpenCVDNNFaceRecognizer::~OpenCVDNNFaceRecognizer()
{
    delete d;
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

    cv::Mat cvImage;// = cv::Mat(image.height(), image.width(), CV_8UC3);
    cv::Mat cvImageWrapper;

    switch (image.format())
    {
        case QImage::Format_RGB32:
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
            // I think we can ignore premultiplication when converting to grayscale
            cvImageWrapper = cv::Mat(image.height(), image.width(), CV_8UC4, image.scanLine(0), image.bytesPerLine());
            cvtColor(cvImageWrapper, cvImage, CV_RGBA2RGB);
            break;
        default:
            image          = image.convertToFormat(QImage::Format_RGB888);
            cvImage        = cv::Mat(image.height(), image.width(), CV_8UC3, image.scanLine(0), image.bytesPerLine());
            //cvtColor(cvImageWrapper, cvImage, CV_RGB2GRAY);
            break;
    }

    //resize(cvImage, cvImage, Size(256, 256), (0, 0), (0, 0), INTER_LINEAR);
    //equalizeHist(cvImage, cvImage);
    return cvImage;
}

int OpenCVDNNFaceRecognizer::recognize(const cv::Mat& inputImage)
{
    int predictedLabel = -1;
    double confidence  = 0;
    d->dnn()->predict(inputImage, predictedLabel, confidence, d->m_extractor);
    qCDebug(DIGIKAM_FACESENGINE_LOG) << predictedLabel << confidence;

    /** confidence must be greater than threshold, because distance used is cosine distance
     * in case that we use euclidean distance, confidence must be less than threshold
     */
    if (confidence < m_threshold)
    {
        return -1;
    }

    return predictedLabel;
}

void OpenCVDNNFaceRecognizer::train(const std::vector<cv::Mat>& images,
                                    const std::vector<int>& labels,
                                    const QString& context,
                                    const std::vector<cv::Mat>& images_rgb)
{
    if (images.empty() || labels.size() != images.size())
    {
        qCDebug(DIGIKAM_FACESENGINE_LOG) << "DNN Train: nothing to train...";
        return;
    }

    d->dnn().update(images_rgb, labels, context, d->m_extractor);
    qCDebug(DIGIKAM_FACESENGINE_LOG) << "DNN Train: Adding model to Facedb";
    // add to database waiting
    FaceDbAccess().db()->updateDNNFaceModel(d->dnn());
}

} // namespace Digikam
