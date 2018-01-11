/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-05-22
 * Description : Face Recognition based on Eigenfaces
 *               http://docs.opencv.org/2.4/modules/contrib/doc/facerec/facerec_tutorial.html#eigenfaces
 *               Turk, Matthew A and Pentland, Alex P. "Face recognition using eigenfaces." 
 *               Computer Vision and Pattern Recognition, 1991. Proceedings {CVPR'91.},
 *               {IEEE} Computer Society Conference on 1991.
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

#ifndef FACEREC_EIGENBORROWED_H
#define FACEREC_EIGENBORROWED_H

#include "libopencv.h"
#include "face.hpp"

// C++ includes

#include <vector>

namespace Digikam
{

class EigenFaceRecognizer : public cv::face::FaceRecognizer
{
public:

    // Initializes this Eigenfaces Model.
    EigenFaceRecognizer(double threshold = DBL_MAX)
        : m_threshold(threshold),
          m_num_components(0)
    {
    }

    // Initializes and computes this Eigenfaces Model.
    EigenFaceRecognizer(cv::InputArrayOfArrays src,
                        cv::InputArray labels,
                        double threshold = DBL_MAX)
        : m_threshold(threshold),
          m_num_components(0)
    {
        train(src, labels);
    }

    ~EigenFaceRecognizer() {}

    using cv::face::FaceRecognizer::predict;

#if OPENCV_TEST_VERSION(3,4,0)
    using cv::face::FaceRecognizer::save;
    using cv::face::FaceRecognizer::load;
#else
    using cv::face::FaceRecognizer::write;
    using cv::face::FaceRecognizer::read;
#endif

    static cv::Ptr<EigenFaceRecognizer> create(double threshold = DBL_MAX);

    /**
     * Computes a Eigenfaces model with images in src and
     * corresponding labels in labels.
     */

    void train(cv::InputArrayOfArrays src, cv::InputArray labels) override;

    /**
     * Updates this Eigenfaces model with images in src and
     * corresponding labels in labels.
     */
    void update(cv::InputArrayOfArrays src, cv::InputArray labels) override;

    /*
     * Predict
     */
    void predict(cv::InputArray src, cv::Ptr<cv::face::PredictCollector> collector) const override;

    /**
     * See FaceRecognizer::load().
     */
#if OPENCV_TEST_VERSION(3,4,0)
    void load(const cv::FileStorage&) override {}
#else
    void read(const cv::FileStorage&) override {}
#endif

    /**
     * See FaceRecognizer::save().
     */
#if OPENCV_TEST_VERSION(3,4,0)
    void save(cv::FileStorage&) const override {}
#else
    void write(cv::FileStorage&) const override {}
#endif

    /**
     * Getter functions.
     */

    int getNumComponents() const                           { return m_num_components;             }
    void setNumComponents(int _num_com_ponents)            { m_num_components = _num_com_ponents; }

    double getThreshold() const override                   { return m_threshold;                  }
    void setThreshold(double _threshold)                   { m_threshold = _threshold;            }

    std::vector<cv::Mat> getSrc() const                    { return m_src;                        }
    void setSrc(std::vector<cv::Mat> _src)                 { m_src = _src;                        }

    std::vector<cv::Mat> getProjections() const            { return m_projections;                }
    void setProjections(std::vector<cv::Mat> _projections) { m_projections = _projections;        }

    cv::Mat getLabels() const                              { return m_labels;                     }
    void setLabels(cv::Mat _labels)                        { m_labels = _labels;                  }

    cv::Mat getEigenvectors() const                        { return m_eigenvectors;               }
    void setEigenvectors(cv::Mat _eigenvectors)            { m_eigenvectors = _eigenvectors;      }

    cv::Mat getMean() const                                { return m_mean;                       }
    void setMean(cv::Mat _mean)                            { m_mean = _mean;                      }

private:

    /** Computes a Eigenfaces model with images in src and
     *  corresponding labels in labels, possibly preserving
     *  old training data.
     */
    void train(cv::InputArrayOfArrays src, cv::InputArray labels, bool preserveData);

private:

    // NOTE: Do not use a d private internal container, this will crash OpenCV in cv::Algorithm::set()
    double               m_threshold;
    int                  m_num_components;

    std::vector<cv::Mat> m_src;
    std::vector<cv::Mat> m_projections;
    cv::Mat              m_labels;
    cv::Mat              m_eigenvectors;
    cv::Mat              m_mean;
};

} // namespace Digikam

#endif // FACEREC_EIGENBORROWED_H
