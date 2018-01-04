/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-07-13
 * Description : Face recognition using deep learning
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

#ifndef FACE_REC_DNN_BORROWED_H
#define FACE_REC_DNN_BORROWED_H

#include "libopencv.h"
#include "facedb.h"

#if !OPENCV_TEST_VERSION(3,0,0)
#include "face.hpp"
#endif

// C++ includes

#include <vector>

namespace Digikam
{
/*
#ifndef DNN_NETWORK
#define DNN_NETWORK
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
#endif
*/
/*
#if OPENCV_TEST_VERSION(3,0,0)
class DNNFaceRecognizer : public cv::FaceRecognizer
#else
class DNNFaceRecognizer : public cv::face::FaceRecognizer
#endif
*/
class DNNFaceRecognizer
{
public:

    // Initializes this DNNFace Model.
    DNNFaceRecognizer(double threshold = DBL_MAX)
        : m_threshold(threshold)
    {
    }

    // Initializes and computes this DNNFace Model.
    DNNFaceRecognizer(std::vector<std::vector<float>> src,
                      cv::InputArray labels,
                      double threshold = DBL_MAX)
        : m_threshold(threshold)
    {
        train(src, labels);
    }

    ~DNNFaceRecognizer() {}

    static cv::Ptr<DNNFaceRecognizer> create(double threshold = DBL_MAX);

    ///void getFaceVector(cv::Mat data, std::vector<float>& vecdata) const;

    /**
     * Computes a DNNFace model with images in src and
     * corresponding labels in labels.
     */
    void train(std::vector<std::vector<float>> src, cv::InputArray labels);
/*
#if OPENCV_TEST_VERSION(3,1,0)
    void train(std::vector<std::vector<float>> src, cv::InputArray labels);
#else
    void train(std::vector<std::vector<float>> src, cv::InputArray labels) override;
#endif
*/
    /**
     * Updates this DNNFace model with images in src and
     * corresponding labels in labels.
     */
    void update(std::vector<std::vector<float>> src, cv::InputArray labels);
/*
#if OPENCV_TEST_VERSION(3,1,0)
    void update(std::vector<std::vector<float>> src, cv::InputArray labels);
#else
    void update(std::vector<std::vector<float>> src, cv::InputArray labels) override;
#endif
*/

//#if OPENCV_TEST_VERSION(3,1,0)
    /**
     * Predicts the label of a query image in src.
     */
    int predict(cv::InputArray src) const;

    /**
     * Predicts the label and confidence for a given sample.
     */
    void predict(cv::InputArray _src, int &label, double &dist) const;
//#else
    //using cv::face::FaceRecognizer::predict;
    /*
     * Predict
     */
    //void predict(cv::InputArray src, cv::Ptr<cv::face::PredictCollector> collector) const;// override;
//#endif

    /**
     * Getter functions.
     */

    double getThreshold() const                            { return m_threshold;                  }
    void setThreshold(double _threshold)                   { m_threshold = _threshold;            }

    std::vector<std::vector<float>> getSrc() const         { return m_src;                        }
    void setSrc(std::vector<std::vector<float>> _src)      { m_src = _src;                        }

    cv::Mat getLabels() const                              { return m_labels;                     }
    void setLabels(cv::Mat _labels)                        { m_labels = _labels;                  }


private:

    /** Computes a DNNFace model with images in src and
     *  corresponding labels in labels, possibly preserving
     *  old training data.
     */
    void train(std::vector<std::vector<float>> src, cv::InputArray labels, bool preserveData);

private:

    // NOTE: Do not use a d private internal container, this will crash OpenCV in cv::Algorithm::set()
    double                          m_threshold;

    std::vector<std::vector<float>> m_src;
    cv::Mat                         m_labels;
};

} // namespace Digikam

#endif // FACE_REC_DNN_BORROWED_H
