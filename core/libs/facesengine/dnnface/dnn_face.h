/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-07-13
 * Description : Face recognition using deep learning
 *               The internal DNN library interface
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

#ifndef _DNN_FACE_H_
#define _DNN_FACE_H_

// C++ includes

#include <vector>

// Qt includes

#include <QString>
#include <QFile>
#include <QDataStream>
#include <QStandardPaths>

// Local includes

#include "shapepredictor.h"
#include "fullobjectdetection.h"
#include "digikam_debug.h"

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

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;
template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
                                alevel0<
                                alevel1<
                                alevel2<
                                alevel3<
                                alevel4<
                                max_pool<3,3,2,2,relu<affine<con<32,7,7,2,2,
                                input_rgb_image_sized<150>
                                >>>>>>>>>>>>;

using namespace Digikam;
using namespace Digikam::redeye;

class DNNFaceKernel
{
public:

    DNNFaceKernel()
    {
    };

    void getFaceVector(cv::Mat tmp_mat, std::vector<float>& vecdata)
    {
        anet_type net;
        frontal_face_detector detector = get_frontal_face_detector();

        QString path1 = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                               QLatin1String("digikam/facesengine/dlib_face_recognition_resnet_model_v1.dat")); 
        deserialize(path1.toStdString()) >> net;

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
        }
        else
        {
            qCDebug(DIGIKAM_FACEDB_LOG) << "Error open file shapepredictor.dat\n";
            return;
        }

        delete temp;

        matrix<rgb_pixel> img;
        std::vector<matrix<rgb_pixel>> faces;

        std::cout << "tmp_mat channels: " << tmp_mat.channels() << std::endl;

        assign_image(img, cv_image<rgb_pixel>(tmp_mat));
        bool face_flag = false;

        for (auto face : detector(img))
        {
            qCDebug(DIGIKAM_FACEDB_LOG) << "Detected face";

            face_flag = true;
            cv::Mat gray;

            int type = tmp_mat.type();
            qCDebug(DIGIKAM_FACEDB_LOG) << "type: " << type;

            if (type == CV_8UC3 || type == CV_16UC3)
            {
                cv::cvtColor(tmp_mat, gray, CV_RGB2GRAY);   // 3 channels
            }
            else
            {
                cv::cvtColor(tmp_mat, gray, CV_RGBA2GRAY);  // 4 channels
            }

            if (type == CV_16UC3 || type == CV_16UC4)
            {
                gray.convertTo(gray, CV_8UC1, 1 / 255.0);
            }

            cv::Rect new_rect(face.left(), face.top(), face.right()-face.left(), face.bottom()-face.top());
            FullObjectDetection object = sp(gray,new_rect);
            qCDebug(DIGIKAM_FACEDB_LOG) << "Full object detection finished";
            matrix<rgb_pixel> face_chip;
            extract_image_chip(img, get_face_chip_details(object,150,0.25), face_chip);
            qCDebug(DIGIKAM_FACEDB_LOG) << "Extract image chip finished";
            faces.push_back(move(face_chip));
            break;
        }

        if (!face_flag)
        {
            cv::resize(tmp_mat, tmp_mat, cv::Size(150, 150));
            assign_image(img, cv_image<rgb_pixel>(tmp_mat));
            faces.push_back(img);
        }

        qCDebug(DIGIKAM_FACEDB_LOG) << "Start neural network";
        std::vector<matrix<float,0,1>> face_descriptors = net(faces);
        qCDebug(DIGIKAM_FACEDB_LOG) << "Face descriptors size:" << face_descriptors.size();

        if (face_descriptors.size() != 0)
        {
            vecdata.clear();

            for(int i = 0 ; i < face_descriptors[0].nr() ; i++)
            {
                for(int j = 0 ; j < face_descriptors[0].nc() ; j++)
                {
                    vecdata.push_back(face_descriptors[0](i, j));
                }
            }
        }
        /*else
        {
            qCDebug(DIGIKAM_FACEDB_LOG) << "Error calculate face vector";
        }*/
    };
};

#endif // _DNN_FACE_H_
