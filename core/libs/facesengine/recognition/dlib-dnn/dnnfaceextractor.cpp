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

// DNN includes

#include "tensor.h"
#include "input.h"
#include "layers.h"
#include "loss.h"
#include "core.h"
#include "solvers.h"
#include "cpu_dlib.h"
#include "tensor_tools.h"
#include "utilities.h"
#include "validation.h"
#include "serialize.h"
#include "matrix.h"
#include "matrix_utilities.h"
#include "matrix_subexp.h"
#include "matrix_math_functions.h"
#include "matrix_generic_image.h"
#include "cv_image.h"
#include "assign_image.h"
#include "interpolation.h"
#include "frontal_face_detector.h"

// OpenCV includes

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// Local includes

#include "shapepredictor.h"
#include "fullobjectdetection.h"
#include "digikam_debug.h"

namespace Digikam
{

DNNFaceExtractor::DNNFaceExtractor()
{
}

void DNNFaceExtractor::getFaceEmbedding(cv::Mat faceImage, std::vector<float>& vecdata)
{
    std::vector<int> OUTER_EYES_AND_NOSE = {36,45,33};
    cv::Mat FACE_TEMPLATE(3, 2, CV_32F);
    FACE_TEMPLATE.at<float>(0,0) = 18.639072;
    FACE_TEMPLATE.at<float>(0,1) = 16.249624;
    FACE_TEMPLATE.at<float>(1,0) = 75.73048;
    FACE_TEMPLATE.at<float>(1,1) = 15.18443;
    FACE_TEMPLATE.at<float>(2,0) = 47.515285;
    FACE_TEMPLATE.at<float>(2,1) = 49.38637;

    qCDebug(DIGIKAM_FACEDB_LOG) << "faceImage channels: " << faceImage.channels();
    qCDebug(DIGIKAM_FACEDB_LOG) << "faceImage size: (" << faceImage.rows << ", " << faceImage.cols << ")\n";

    // cv::imshow("face", faceImage);
    // cv::waitKey(0);


    // Detection, shapepredictor and extract face landmark (using for DNN dlib)
    // Codes of shape predictor are for extracting 68 points of face landmark

    redeye::ShapePredictor sp;
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

    matrix<rgb_pixel> img;
    std::vector<cv::Mat> faces;

    assign_image(img, cv_image<rgb_pixel>(faceImage));
    bool face_flag = false;
    frontal_face_detector detector = get_frontal_face_detector();

    for (auto face : detector(img))
    {
        qCDebug(DIGIKAM_FACEDB_LOG) << "Detected face";

        face_flag = true;
        cv::Mat gray;

        int type = faceImage.type();
        qCDebug(DIGIKAM_FACEDB_LOG) << "type: " << type;

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

        cv::Rect new_rect(face.left(), face.top(), face.right()-face.left(), face.bottom()-face.top());
        FullObjectDetection object = sp(gray, new_rect);
        qCDebug(DIGIKAM_FACEDB_LOG) << "Full object detection finished";

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
        
        faces.push_back(move(alignedFace));
        
        break;
    }

    if (!face_flag)
    {
        faces.push_back(faceImage);
    }

    std::cout << "faces size: " << faces.size() << "\n";

    // Read pretrained neural network
    
    // cv::dnn::Net net = cv::dnn::readNetFromCaffe("/home/trungdinh/Devel/digikam/core/libs/facesengine/recognition/dlib-dnn/models/deep-residual-networks/ResNet-50-deploy.prototxt",
    //                                              "/home/trungdinh/Devel/digikam/core/libs/facesengine/recognition/dlib-dnn/models/deep-residual-networks/ResNet-50-model.caffemodel");
    cv::dnn::Net net = cv::dnn::readNetFromTorch("/home/trungdinh/Devel/digikam/core/libs/facesengine/recognition/dlib-dnn/models/openface_nn4.small2.v1.t7");

    qCDebug(DIGIKAM_FACEDB_LOG) << "Start neural network";

    cv::Mat face_descriptors;
    for(auto face : faces)
    {
        cv::Mat blob = cv::dnn::blobFromImage(face, 1.0 / 255, cv::Size(96, 96), cv::Scalar(0,0,0), false, true, CV_32F);
        net.setInput(blob);
        face_descriptors = net.forward();
    }

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