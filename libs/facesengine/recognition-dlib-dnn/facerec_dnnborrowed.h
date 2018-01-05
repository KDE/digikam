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
#include "face.hpp"

// C++ includes

#include <vector>

namespace Digikam
{

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

    ~DNNFaceRecognizer()
    {
    }

    static cv::Ptr<DNNFaceRecognizer> create(double threshold = DBL_MAX);

    ///void getFaceVector(cv::Mat data, std::vector<float>& vecdata) const;

    /**
     * Computes a DNNFace model with images in src and
     * corresponding labels in labels.
     */
    void train(std::vector<std::vector<float>> src, cv::InputArray labels);

    /**
     * Updates this DNNFace model with images in src and
     * corresponding labels in labels.
     */
    void update(std::vector<std::vector<float>> src, cv::InputArray labels);

    /**
     * Predicts the label of a query image in src.
     */
    int predict(cv::InputArray src) const;

    /**
     * Predicts the label and confidence for a given sample.
     */
    void predict(cv::InputArray _src, int& label, double& dist) const;

    /**
     * Getter functions.
     */

    double getThreshold() const                            { return m_threshold;                  }
    void   setThreshold(double _threshold)                 { m_threshold = _threshold;            }

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
