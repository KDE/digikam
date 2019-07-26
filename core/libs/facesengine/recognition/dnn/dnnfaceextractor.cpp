/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2019-06-01
 * Description : Face recognition using deep learning
 *               The internal DNN library interface
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

#include "dnnfaceextractor.h"

// Qt includes

#include <QString>
#include <QFile>
#include <QDataStream>
#include <QStandardPaths>
#include <QTime>

// OpenCV includes

#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DNNFaceExtractor::DNNFaceExtractor(Preprocessor* p)
  : preprocessor(p)
{
    // Read pretrained neural network for face recognition

    // QString nnproto = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
    //                                           QLatin1String("digikam/facesengine/dnnface/deep-residual-networks/ResNet-50-deploy.prototxt"));
    // QString nnmodel = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
    //                                           QLatin1String("digikam/facesengine/dnnface/deep-residual-networks/ResNet-50-model.caffemodel"));
    // net = cv::dnn::readNetFromCaffe(nnproto, nnmodel);

    QString nnmodel = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                             QLatin1String("digikam/facesengine/openface_nn4.small2.v1.t7"));
    qCDebug(DIGIKAM_FACEDB_LOG) << nnmodel;
    net = cv::dnn::readNetFromTorch(nnmodel.toStdString());

    // As we use OpenFace, we need to set appropriate values for image color space and image size

    imageSize = cv::Size(96,96);
    scaleFactor = 1.0 / 255.0;
    meanValToSubtract = cv::Scalar(0.0, 0.0, 0.0);
}

DNNFaceExtractor::~DNNFaceExtractor()
{
}

void DNNFaceExtractor::getFaceEmbedding(cv::Mat faceImage, std::vector<float>& vecdata)
{
    qCDebug(DIGIKAM_FACEDB_LOG) << "faceImage channels: " << faceImage.channels();
    qCDebug(DIGIKAM_FACEDB_LOG) << "faceImage size: (" << faceImage.rows << ", " << faceImage.cols << ")\n";

    QTime timer;
    timer.start();

    cv::Mat alignedFace = /*faceImage;*/ preprocessor->preprocess(faceImage);

    qCDebug(DIGIKAM_FACEDB_LOG) << "Finish aligning face in " << timer.elapsed() << " ms";

    qCDebug(DIGIKAM_FACEDB_LOG) << "Start neural network";

    timer.start();
    cv::Mat face_descriptors;
    cv::Mat blob = cv::dnn::blobFromImage(alignedFace, scaleFactor, imageSize, meanValToSubtract);
    net.setInput(blob);
    face_descriptors = net.forward();

    qCDebug(DIGIKAM_FACEDB_LOG) << "Finish computing face embedding in " << timer.elapsed() << " ms";

    // cv::Mat blob = cv::dnn::blobFromImage(faceImage, 1.0 / 255, cv::Size(96, 96), cv::Scalar(0,0,0), false, true, CV_32F); // work for openface.nn4
    // cv::Mat blob = cv::dnn::blobFromImage(faceImage, 1.0 / 255, cv::Size(224,224), cv::Scalar(0,0,0), false, true, CV_32F);
    // net.setInput(blob);
    // cv::Mat face_descriptors = net.forward();

    qCDebug(DIGIKAM_FACEDB_LOG) << "Face descriptors size: (" << face_descriptors.rows << ", " << face_descriptors.cols << ")";

    for(int i = 0 ; i < face_descriptors.rows ; ++i)
    {
        for(int j = 0 ; j < face_descriptors.cols ; ++j)
        {
            vecdata.push_back(face_descriptors.at<float>(i,j));
        }
    }
}

}; // namespace Digikam