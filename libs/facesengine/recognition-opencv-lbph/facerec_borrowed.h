/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2012-01-03
 * Description : http://docs.opencv.org/modules/contrib/doc/facerec/facerec_tutorial.html#local-binary-patterns-histograms
 *               Ahonen T, Hadid A. and Pietikäinen M. "Face description with local binary
 *               patterns: Application to face recognition." IEEE Transactions on Pattern
 *               Analysis and Machine Intelligence, 28(12):2037-2041.
 *
 * Copyright (C) 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2011-2012 by Philipp Wagner <bytefish at gmx dot de>
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

#ifndef FACEREC_BORROWED_H
#define FACEREC_BORROWED_H

#include "libopencv.h"
#include "face.hpp"

// C++ includes

#include <vector>

namespace Digikam
{

class LBPHFaceRecognizer : public cv::face::FaceRecognizer
{
public:

    enum PredictionStatistics
    {
        NearestNeighbor,
        NearestMean,
        MostNearestNeighbors // makes only sense if there is a threshold!
    };

public:

    // Initializes this LBPH Model. The current implementation is rather fixed
    // as it uses the Extended Local Binary Patterns per default.
    //
    // radius, neighbors are used in the local binary patterns creation.
    // grid_x, grid_y control the grid size of the spatial histograms.
    LBPHFaceRecognizer(int radius_=1,
                       int neighbors_=8,
                       int gridx=8,
                       int gridy=8,
                       double threshold = DBL_MAX,
                       PredictionStatistics statistics = NearestNeighbor) :
        m_grid_x(gridx),
        m_grid_y(gridy),
        m_radius(radius_),
        m_neighbors(neighbors_),
        m_threshold(threshold),
        m_statisticsMode(statistics)
    {
    }

    // Initializes and computes this LBPH Model. The current implementation is
    // rather fixed as it uses the Extended Local Binary Patterns per default.
    //
    // (radius=1), (neighbors=8) are used in the local binary patterns creation.
    // (grid_x=8), (grid_y=8) controls the grid size of the spatial histograms.
    LBPHFaceRecognizer(cv::InputArrayOfArrays src,
                       cv::InputArray labels,
                       int radius_=1, int neighbors_=8,
                       int gridx=8, int gridy=8,
                       double threshold = DBL_MAX,
                       PredictionStatistics statistics = NearestNeighbor) :
        m_grid_x(gridx),
        m_grid_y(gridy),
        m_radius(radius_),
        m_neighbors(neighbors_),
        m_threshold(threshold),
        m_statisticsMode(statistics)
    {
        train(src, labels);
    }

    ~LBPHFaceRecognizer() {}

    using cv::face::FaceRecognizer::predict;

#if OPENCV_TEST_VERSION(3,4,0)
    using cv::face::FaceRecognizer::save;
    using cv::face::FaceRecognizer::load;
#else
    using cv::face::FaceRecognizer::write;
    using cv::face::FaceRecognizer::read;
#endif

    static cv::Ptr<LBPHFaceRecognizer> create(int radius=1,
                                              int neighbors=8,
                                              int grid_x=8,
                                              int grid_y=8,
                                              double threshold = DBL_MAX,
                                              PredictionStatistics statistics = NearestNeighbor);

    /**
     * Computes a LBPH model with images in src and
     * corresponding labels in labels.
     */
    void train(cv::InputArrayOfArrays src, cv::InputArray labels) override;

    /**
     * Updates this LBPH model with images in src and
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

    int getNeighbors() const                             { return m_neighbors;            }
    void setNeighbors(int _neighbors)                    { m_neighbors = _neighbors;      }

    int getRadius()    const                             { return m_radius;               }
    void setRadius(int radius)                           { m_radius = radius;             }

    int getGrid_x()    const                             { return m_grid_x;               }
    void setGrid_x(int _grid_x)                          { m_grid_x = _grid_x;            }

    int getGrid_y()    const                             { return m_grid_y;               }
    void setGrid_y(int _grid_y)                          { m_grid_y = _grid_y;            }

    double getThreshold() const override                 { return m_threshold;            }
    void setThreshold(double _threshold)                 { m_threshold = _threshold;      }

    void setHistograms(std::vector<cv::Mat> _histograms) { m_histograms = _histograms;    }
    std::vector<cv::Mat> getHistograms() const           { return m_histograms;           }

    void setLabels(cv::Mat _labels)                      { m_labels = _labels;            }
    cv::Mat getLabels() const                            { return m_labels;               }

    void setStatistic(int _statistic)                    { m_statisticsMode = _statistic; }
    int getStatistic() const                             { return m_statisticsMode;       }

private:

    /** Computes a LBPH model with images in src and
     *  corresponding labels in labels, possibly preserving
     *  old model data.
     */
    void train(cv::InputArrayOfArrays src, cv::InputArray labels, bool preserveData);

private:

    // NOTE: Do not use a d private internal container, this will crash OpenCV in cv::Algorithm::set()
    int                  m_grid_x;
    int                  m_grid_y;
    int                  m_radius;
    int                  m_neighbors;
    double               m_threshold;
    int                  m_statisticsMode;

    std::vector<cv::Mat> m_histograms;
    cv::Mat              m_labels;
};

} // namespace Digikam

#endif // FACEREC_BORROWED_H
