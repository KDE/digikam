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

#include "facerec_dnnborrowed.h"

// C++ includes

#include <set>
#include <limits>
#include <vector>
#include <cmath>

//Qt includes
#include <QFile>
#include <QDataStream>
#include <QStandardPaths>


// Local includes
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

#include "dnn_face.h"
#include "digikam_debug.h"
#include "shapepredictor.h"
#include "fullobjectdetection.h"

using namespace cv;

namespace Digikam
{

void DNNFaceRecognizer::train(std::vector<std::vector<float>> _in_src, InputArray _inm_labels)
{
    this->train(_in_src, _inm_labels, false);
}

void DNNFaceRecognizer::update(std::vector<std::vector<float>> _in_src, InputArray _inm_labels)
{
    // got no data, just return
    if (_in_src.size() == 0)
        return;

    this->train(_in_src, _inm_labels, true);
}

void DNNFaceRecognizer::train(std::vector<std::vector<float>> _in_src, InputArray _inm_labels, bool preserveData)
{
    /*
    This train function is used to store the face vectors, not training
    */

    // get the vector of matrices
    std::vector<std::vector<float>> src = _in_src;

    // get the label matrix
    cv::Mat labels = _inm_labels.getMat();

    // check if data is well- aligned
    if (labels.total() != src.size())
    {
        String error_message = format("The number of samples (src) must equal the number of labels (labels). Was len(samples)=%d, len(labels)=%d.", src.size(), m_labels.total());
        CV_Error(CV_StsBadArg, error_message);
    }

    // if this model should be trained without preserving old data, delete old model data
    if (!preserveData)
    {
        m_labels.release();
        m_src.clear();
    }

    // append labels to m_labels matrix
    for (size_t labelIdx = 0; labelIdx < labels.total(); labelIdx++)
    {
        m_labels.push_back(labels.at<int>((int)labelIdx));
        m_src.push_back(src[(int)labelIdx]);
    }

    return ;
}
/*
void DNNFaceRecognizer::getFaceVector(cv::Mat data, std::vector<float>& vecdata) const
{
    anet_type net;
    frontal_face_detector detector = get_frontal_face_detector();
    qCDebug(DIGIKAM_FACEDB_LOG) << "Start reading model file";
    QString path1 = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              QLatin1String("digikam/facesengine/dlib_face_recognition_resnet_model_v1.dat")); 
    deserialize(path1.toStdString()) >> net;
    qCDebug(DIGIKAM_FACEDB_LOG) << "End reading model file";
    qCDebug(DIGIKAM_FACEDB_LOG) << "Start reading shape file";
    redeye::ShapePredictor sp;
    QString path2 = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                              QLatin1String("digikam/facesengine/shapepredictor.dat"));
    QFile model(path2);
    std::cout << "read file\n";
    if (model.open(QIODevice::ReadOnly))
    {
        redeye::ShapePredictor* const temp = new redeye::ShapePredictor();
        QDataStream dataStream(&model);
        dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        dataStream >> *temp;
        sp = *temp;
    }
    else
    {
        qCDebug(DIGIKAM_FACEDB_LOG) << "Error open file shapepredictor.dat";
        return ;
    }

    cv::Mat tmp_mat = data;
    matrix<rgb_pixel> img;
    std::vector<matrix<rgb_pixel>> faces;
    assign_image(img, cv_image<rgb_pixel>(tmp_mat));
    bool face_flag = false;
    for (auto face : detector(img))
    {
        face_flag = true;
        cv::Mat gray;
        
        int type = tmp_mat.type();
        if(type == CV_8UC3 || type == CV_16UC3)
        {
            cv::cvtColor(tmp_mat, gray, CV_RGB2GRAY);  // 3 channels
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
        matrix<rgb_pixel> face_chip;
        extract_image_chip(img, get_face_chip_details(object,150,0.25), face_chip);
        faces.push_back(move(face_chip));
        break;
    }
    if(!face_flag)
    {
        cv::resize(tmp_mat, tmp_mat, cv::Size(150, 150));
        assign_image(img, cv_image<rgb_pixel>(tmp_mat));
        faces.push_back(img);
    }
    std::vector<matrix<float,0,1>> face_descriptors = net(faces);
    if(face_descriptors.size()!=0)
    {
        vecdata.clear();
        for(int i = 0; i < face_descriptors[0].nr(); i++)
        {
            for(int j = 0; j < face_descriptors[0].nc(); j++)
            {
                vecdata.push_back(face_descriptors[0](i, j));
            }
        }
    }
    else
    {
        qCDebug(DIGIKAM_FACEDB_LOG) << "Error calculate face vector";
    }
}
*/
//#if OPENCV_TEST_VERSION(3,1,0)
void DNNFaceRecognizer::predict(cv::InputArray _src, int &minClass, double &minDist) const
//#else
//void DNNFaceRecognizer::predict(cv::InputArray _src, cv::Ptr<cv::face::PredictCollector> collector) const
//#endif
{
    qCWarning(DIGIKAM_FACESENGINE_LOG) << "Predicting face image";

    cv::Mat src = _src.getMat();//254*254
    std::vector<float> vecdata;
    DNNFaceKernel dnnface_kernel;
    dnnface_kernel.getFaceVector(src, vecdata);

    minDist      = DBL_MAX;
    minClass     = -1;

/*
#if OPENCV_TEST_VERSION(3,1,0)
    minDist      = DBL_MAX;
    minClass     = -1;
#else
    collector->init(0);//here need to confirm
#endif
*/

    //find nearest neighbor
    for (size_t sampleIdx = 0; sampleIdx < m_src.size(); sampleIdx++)
    {
        double dist = 0;
        for(int i = 0; i < m_src[sampleIdx].size(); i++)
        {
            dist += (vecdata[i]-m_src[sampleIdx][i])*(vecdata[i]-m_src[sampleIdx][i]);
        }
        dist = std::sqrt(dist);

//#if OPENCV_TEST_VERSION(3,1,0)
        if ((dist < minDist) && (dist < m_threshold))
        {
            minDist  = dist;
            minClass = m_labels.at<int>((int) sampleIdx);
        }
    }
/*
#else
        int label = m_labels.at<int>((int) sampleIdx);

        if (!collector->collect(label, dist))
        {
            return;
        }
    }
#endif*/
}

//#if OPENCV_TEST_VERSION(3,1,0)
int DNNFaceRecognizer::predict(InputArray _src) const
{
    int    label;
    double dummy;
    predict(_src, label, dummy);

    return label;
}
//#endif

// Static method ----------------------------------------------------

Ptr<DNNFaceRecognizer> DNNFaceRecognizer::create(double threshold)
{
    Ptr<DNNFaceRecognizer> ptr;

    DNNFaceRecognizer* const fr = new DNNFaceRecognizer(threshold);

    if (!fr)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "Cannot create DNNFaceRecognizer instance";
        return ptr;
    }

    ptr = Ptr<DNNFaceRecognizer>(fr);

    if (ptr.empty())
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "DNNFaceRecognizer instance is empty";
    }

    return ptr;
}

} // namespace Digikam
