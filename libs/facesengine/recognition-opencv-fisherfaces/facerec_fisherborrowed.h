/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date    2017-06-10
 * @brief   <a href="http://docs.opencv.org/2.4/modules/contrib/doc/facerec/facerec_tutorial.html#Fisherfaces">Face Recognition based on Fisherfaces</a>
 *          Turk, Matthew A and Pentland, Alex P. "Face recognition using Fisherfaces." 
 *          Computer Vision and Pattern Recognition, 1991. Proceedings {CVPR'91.},
 *          {IEEE} Computer Society Conference on 1991.
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

#include "libopencv.h"

#if !OPENCV_TEST_VERSION(3,0,0)
#include "face.hpp"
#endif

// C++ includes

#include <vector>

namespace Digikam
{

#if OPENCV_TEST_VERSION(3,0,0)
class FisherFaceRecognizer : public cv::FaceRecognizer
#else
class FisherFaceRecognizer : public cv::face::FaceRecognizer
#endif
{
public:

    // Initializes this Fisherfaces Model.
    FisherFaceRecognizer(double threshold = DBL_MAX):
    m_threshold(threshold), m_num_components(0)
    {
    }

    // Initializes and computes this Fisherfaces Model.
    FisherFaceRecognizer(cv::InputArrayOfArrays src,
                       cv::InputArray labels,
                       double threshold = DBL_MAX) :
        m_threshold(threshold), m_num_components(0)
    {
        train(src, labels);
    }

    ~FisherFaceRecognizer() {}

#if OPENCV_TEST_VERSION(3,0,0)
    using cv::FaceRecognizer::save;
    using cv::FaceRecognizer::load;
#else
    using cv::face::FaceRecognizer::save;
    using cv::face::FaceRecognizer::load;
#endif

    static cv::Ptr<FisherFaceRecognizer> create(double threshold = DBL_MAX);

    /**
     * Computes a Fisherfaces model with images in src and
     * corresponding labels in labels.
     */
#if OPENCV_TEST_VERSION(3,1,0)
    void train(cv::InputArrayOfArrays src, cv::InputArray labels);
#else
    void train(cv::InputArrayOfArrays src, cv::InputArray labels) override;
#endif

    /**
     * Updates this Fisherfaces model with images in src and
     * corresponding labels in labels.
     */
#if OPENCV_TEST_VERSION(3,1,0)
    void update(cv::InputArrayOfArrays src, cv::InputArray labels);
#else
    void update(cv::InputArrayOfArrays src, cv::InputArray labels) override;
#endif


#if OPENCV_TEST_VERSION(3,1,0)
    /**
     * Predicts the label of a query image in src.
     */
    int predict(cv::InputArray src) const;

    /**
     * Predicts the label and confidence for a given sample.
     */
    void predict(cv::InputArray _src, int &label, double &dist) const;
#else
    using cv::face::FaceRecognizer::predict;
    /*
     * Predict
     */
    void predict(cv::InputArray src, cv::Ptr<cv::face::PredictCollector> collector) const override;
#endif

    /**
     * See FaceRecognizer::read().
     */
    void read(const cv::FileStorage&) {}

    /**
     * See FaceRecognizer::write().
     */
    void write(cv::FileStorage&) const {}

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

    /** Computes a Fisherfaces model with images in src and
     *  corresponding labels in labels, possibly preserving
     *  old training data.
     */
    void train(cv::InputArrayOfArrays src, cv::InputArray labels, bool preserveData);

private:

    // NOTE: Do not use a d private internal container, this will crash OpenCV in cv::Algorithm::set()
    double m_threshold;
    int m_num_components;

    std::vector<cv::Mat> m_src;
    std::vector<cv::Mat> m_projections;
    cv::Mat m_labels;
    cv::Mat m_eigenvectors;
    cv::Mat m_mean;
};

} // namespace Digikam
