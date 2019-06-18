/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2019-06-01
 * Description : Face recognition using deep learning
 *               The internal DNN library interface
 *
 * Copyright (C) 2019      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// C++ includes

#include <array>

// Local includes

#include "shapepredictor.h"
#include "fullobjectdetection.h"
#include "digikam_debug.h"

namespace Digikam
{

// --------------------------------------- Static global variables -----------------------------------

static float data[3][2] =   { {18.639072, 16.249624}, {75.73048,  15.18443}, {47.515285, 49.38637} };
static cv::Mat FACE_TEMPLATE = cv::Mat(3, 2, CV_32F, &data);

static std::array<int, 3> OUTER_EYES_AND_NOSE{36,45,33};

static redeye::ShapePredictor sp;

static cv::dnn::Net net;

// --------------------------------------- Static class members --------------------------------------

bool DNNFaceExtractor::initialized = false;

// --------------------------------------- Class member functions-------------------------------------

DNNFaceExtractor::DNNFaceExtractor()
{
}

void DNNFaceExtractor::init()
{
    if(!initialized)
    {
        // Load shapepredictor model for face alignment with 68 points of face landmark extraction

        QString path2 = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                               QLatin1String("digikam/facesengine/shapepredictor.dat"));
        QFile model(path2);
        redeye::ShapePredictor* const temp = new redeye::ShapePredictor();

        qCDebug(DIGIKAM_FACEDB_LOG) << "Start reading shape predictor file";

        if (model.open(QIODevice::ReadOnly))
        {
            QDataStream dataStream(&model);
            dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
            dataStream >> *temp;
            sp = *temp;
            model.close();
        }
        else
        {
            qCDebug(DIGIKAM_FACEDB_LOG) << "Error open file shapepredictor.dat\n";
            return;
        }

        delete temp;

        // Read pretrained neural network for face recognition

        // net = cv::dnn::readNetFromCaffe("/home/trungdinh/Devel/digikam/core/libs/facesengine/recognition/dlib-dnn/models/deep-residual-networks/ResNet-50-deploy.prototxt",
        //                                 "/home/trungdinh/Devel/digikam/core/libs/facesengine/recognition/dlib-dnn/models/deep-residual-networks/ResNet-50-model.caffemodel");
        net = cv::dnn::readNetFromTorch("/home/trungdinh/Devel/digikam/core/libs/facesengine/recognition/dlib-dnn/models/openface_nn4.small2.v1.t7");
    }

    initialized = true;
}

void DNNFaceExtractor::getFaceEmbedding(cv::Mat faceImage, std::vector<float>& vecdata)
{
    qCDebug(DIGIKAM_FACEDB_LOG) << "faceImage channels: " << faceImage.channels();
    qCDebug(DIGIKAM_FACEDB_LOG) << "faceImage size: (" << faceImage.rows << ", " << faceImage.cols << ")\n";

    QTime timer;
    timer.start();

    int type = faceImage.type();
    qCDebug(DIGIKAM_FACEDB_LOG) << "type: " << type;

    cv::Mat gray;

    if (type == CV_8UC3 || type == CV_16UC3)
    {
        cv::cvtColor(faceImage, gray, CV_RGB2GRAY);   // 3 channels
    }
    else
    {
        cv::cvtColor(faceImage, gray, CV_RGBA2GRAY);  // 4 channels
    }

    if (type == CV_16UC3 || type == CV_16UC4)
    {
        gray.convertTo(gray, CV_8UC1, 1 / 255.0);
    }

    cv::Rect new_rect(0, 0, faceImage.cols, faceImage.rows);
    FullObjectDetection object = sp(gray, new_rect);
    qCDebug(DIGIKAM_FACEDB_LOG) << "Full object detection finished";

    qCDebug(DIGIKAM_FACEDB_LOG) << "Finish computing landmark in " << timer.restart() << " ms";

    cv::Mat landmarks(3,2,CV_32F);
    int row = 0;
    for(auto indx : OUTER_EYES_AND_NOSE)
    {
        landmarks.at<float>(row,0) = object.part(indx)[0];
        landmarks.at<float>(row,1) = object.part(indx)[1];
        row++;
    }
    cv::Mat affineTransformMatrix = cv::getAffineTransform(landmarks, FACE_TEMPLATE);
    cv::Mat alignedFace;
    cv::warpAffine(faceImage, alignedFace, affineTransformMatrix, cv::Size(96, 96));
    qCDebug(DIGIKAM_FACEDB_LOG) << "Align face finished";

    qCDebug(DIGIKAM_FACEDB_LOG) << "Finish aligning face in " << timer.restart() << " ms";

    qCDebug(DIGIKAM_FACEDB_LOG) << "Start neural network";

    cv::Mat face_descriptors;
    cv::Mat blob = cv::dnn::blobFromImage(alignedFace, 1.0 / 255, cv::Size(96, 96), cv::Scalar(0,0,0), false, true, CV_32F);
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