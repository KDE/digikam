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
