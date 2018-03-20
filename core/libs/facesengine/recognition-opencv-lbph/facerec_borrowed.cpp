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

#include "facerec_borrowed.h"

// C++ includes

#include <set>
#include <limits>

// Local includes

#include "digikam_debug.h"

using namespace cv;

namespace Digikam
{

//------------------------------------------------------------------------------
// LBPH
//------------------------------------------------------------------------------

template <typename _Tp> static
void olbp_(InputArray _src, OutputArray _dst)
{
    // get matrices
    Mat src = _src.getMat();

    // allocate memory for result
    _dst.create(src.rows-2, src.cols-2, CV_8UC1);
    Mat dst = _dst.getMat();

    // zero the result matrix
    dst.setTo(0);

    // calculate patterns

    for (int i=1 ; i < src.rows-1 ; i++)
    {
        for (int j=1 ; j < src.cols-1 ; j++)
        {
            _Tp center         = src.at<_Tp>(i,j);
            unsigned char code = 0;
            code |= (src.at<_Tp>(i-1,j-1) >= center) << 7;
            code |= (src.at<_Tp>(i-1,j) >= center) << 6;
            code |= (src.at<_Tp>(i-1,j+1) >= center) << 5;
            code |= (src.at<_Tp>(i,j+1) >= center) << 4;
            code |= (src.at<_Tp>(i+1,j+1) >= center) << 3;
            code |= (src.at<_Tp>(i+1,j) >= center) << 2;
            code |= (src.at<_Tp>(i+1,j-1) >= center) << 1;
            code |= (src.at<_Tp>(i,j-1) >= center) << 0;
            dst.at<unsigned char>(i-1,j-1) = code;
        }
    }
}

//------------------------------------------------------------------------------
// cv::elbp
//------------------------------------------------------------------------------

template <typename _Tp> static
inline void elbp_(InputArray _src, OutputArray _dst, int radius, int neighbors)
{
    //get matrices
    Mat src = _src.getMat();
    // allocate memory for result
    _dst.create(src.rows-2*radius, src.cols-2*radius, CV_32SC1);
    Mat dst = _dst.getMat();
    // zero
    dst.setTo(0);

    for (int n=0; n < neighbors; n++)
    {
        // sample points
        float x = static_cast<float>(radius * cos(2.0*CV_PI*n/static_cast<float>(neighbors)));
        float y = static_cast<float>(-radius * sin(2.0*CV_PI*n/static_cast<float>(neighbors)));

        // relative indices
        int fx = static_cast<int>(floor(x));
        int fy = static_cast<int>(floor(y));
        int cx = static_cast<int>(ceil(x));
        int cy = static_cast<int>(ceil(y));

        // fractional part
        float ty = y - fy;
        float tx = x - fx;

        // set interpolation weights
        float w1 = (1 - tx) * (1 - ty);
        float w2 =      tx  * (1 - ty);
        float w3 = (1 - tx) *      ty;
        float w4 =      tx  *      ty;

        // iterate through your data
        for (int i=radius; i < src.rows-radius;i++)
        {
            for (int j=radius;j < src.cols-radius;j++)
            {
                // calculate interpolated value
                float t                         = static_cast<float>(w1*src.at<_Tp>(i+fy,j+fx) + w2*src.at<_Tp>(i+fy,j+cx) + w3*src.at<_Tp>(i+cy,j+fx) + w4*src.at<_Tp>(i+cy,j+cx));
                // floating point precision, so check some machine-dependent epsilon
                dst.at<int>(i-radius,j-radius) += ((t > src.at<_Tp>(i,j)) || (qAbs(t-src.at<_Tp>(i,j)) < std::numeric_limits<float>::epsilon())) << n;
            }
        }
    }
}

static void elbp(InputArray src, OutputArray dst, int radius, int neighbors)
{
    int type = src.type();

    switch (type)
    {
        case CV_8SC1:   elbp_<char>(src,dst, radius, neighbors);
            break;
        case CV_8UC1:   elbp_<unsigned char>(src, dst, radius, neighbors);
            break;
        case CV_16SC1:  elbp_<short>(src,dst, radius, neighbors);
            break;
        case CV_16UC1:  elbp_<unsigned short>(src,dst, radius, neighbors);
            break;
        case CV_32SC1:  elbp_<int>(src,dst, radius, neighbors);
            break;
        case CV_32FC1:  elbp_<float>(src,dst, radius, neighbors);
            break;
        case CV_64FC1:  elbp_<double>(src,dst, radius, neighbors);
            break;
        default:
            String error_msg = format("Using Original Local Binary Patterns for feature extraction only works on single-channel images (given %d). Please pass the image data as a grayscale image!", type);
            CV_Error(CV_StsNotImplemented, error_msg);
            break;
    }
}

static Mat histc_(const Mat& src, int minVal=0, int maxVal=255, bool normed=false)
{
    Mat result;

    // Establish the number of bins.
    int histSize           = maxVal-minVal+1;

    // Set the ranges.
    float range[]          = { static_cast<float>(minVal), static_cast<float>(maxVal+1) };
    const float* histRange = { range };

    // calc histogram
    calcHist(&src, 1, 0, Mat(), result, 1, &histSize, &histRange, true, false);

    // normalize
    if (normed)
    {
        result /= (int)src.total();
    }

    return result.reshape(1,1);
}

static Mat histc(InputArray _src, int minVal, int maxVal, bool normed)
{
    Mat src = _src.getMat();

    switch (src.type())
    {
        case CV_8SC1:
            return histc_(Mat_<float>(src), minVal, maxVal, normed);
            break;
        case CV_8UC1:
            return histc_(src, minVal, maxVal, normed);
            break;
        case CV_16SC1:
            return histc_(Mat_<float>(src), minVal, maxVal, normed);
            break;
        case CV_16UC1:
            return histc_(src, minVal, maxVal, normed);
            break;
        case CV_32SC1:
            return histc_(Mat_<float>(src), minVal, maxVal, normed);
            break;
        case CV_32FC1:
            return histc_(src, minVal, maxVal, normed);
            break;
        default:
            CV_Error(CV_StsUnmatchedFormats, "This type is not implemented yet."); break;
    }

    return Mat();
}


static Mat spatial_histogram(InputArray _src, int numPatterns, int grid_x, int grid_y, bool /*normed*/)
{
    Mat src    = _src.getMat();

    // calculate LBP patch size
    int width  = src.cols/grid_x;
    int height = src.rows/grid_y;

    // allocate memory for the spatial histogram
    Mat result = Mat::zeros(grid_x * grid_y, numPatterns, CV_32FC1);

    // return matrix with zeros if no data was given
    if (src.empty())
        return result.reshape(1,1);

    // initial result_row
    int resultRowIdx = 0;

    // iterate through grid

    for (int i = 0; i < grid_y; i++)
    {
        for (int j = 0; j < grid_x; j++)
        {
            Mat src_cell   = Mat(src, Range(i*height,(i+1)*height), Range(j*width,(j+1)*width));
            Mat cell_hist  = histc(src_cell, 0, (numPatterns-1), true);

            // copy to the result matrix
            Mat result_row = result.row(resultRowIdx);
            cell_hist.reshape(1,1).convertTo(result_row, CV_32FC1);

            // increase row count in result matrix
            resultRowIdx++;
        }
    }

    // return result as reshaped feature vector
    return result.reshape(1,1);
}

//------------------------------------------------------------------------------
// wrapper to cv::elbp (extended local binary patterns)
//------------------------------------------------------------------------------

static Mat elbp(InputArray src, int radius, int neighbors)
{
    Mat dst;
    elbp(src, dst, radius, neighbors);

    return dst;
}

/*
 * Implementation not copied from OpenCV
void LBPHFaceRecognizer::load(const FileStorage& fs)
{
    fs["radius"] >> m_radius;
    fs["neighbors"] >> m_neighbors;
    fs["grid_x"] >> m_grid_x;
    fs["grid_y"] >> m_grid_y;
    //read matrices
    readFileNodeList(fs["histograms"], m_histograms);
    fs["labels"] >> m_labels;
}

// See FaceRecognizer::save.
void LBPHFaceRecognizer::save(FileStorage& fs) const
{
    fs << "radius" << m_radius;
    fs << "neighbors" << m_neighbors;
    fs << "grid_x" << m_grid_x;
    fs << "grid_y" << m_grid_y;
    // write matrices
    writeFileNodeList(fs, "histograms", m_histograms);
    fs << "labels" << m_labels;
}
*/

void LBPHFaceRecognizer::train(InputArrayOfArrays _in_src, InputArray _inm_labels)
{
    this->train(_in_src, _inm_labels, false);
}

void LBPHFaceRecognizer::update(InputArrayOfArrays _in_src, InputArray _inm_labels)
{
    // got no data, just return
    if (_in_src.total() == 0)
        return;

    this->train(_in_src, _inm_labels, true);
}

void LBPHFaceRecognizer::train(InputArrayOfArrays _in_src, InputArray _inm_labels, bool preserveData)
{
    if (_in_src.kind() != _InputArray::STD_VECTOR_MAT && _in_src.kind() != _InputArray::STD_VECTOR_VECTOR)
    {
        String error_message = "The images are expected as InputArray::STD_VECTOR_MAT (a std::vector<Mat>) or _InputArray::STD_VECTOR_VECTOR (a std::vector< std::vector<...> >).";
        CV_Error(CV_StsBadArg, error_message);
    }

    if (_in_src.total() == 0)
    {
        String error_message = format("Empty training data was given. You'll need more than one sample to learn a model.");
        CV_Error(CV_StsUnsupportedFormat, error_message);
    }
    else if (_inm_labels.getMat().type() != CV_32SC1)
    {
        String error_message = format("Labels must be given as integer (CV_32SC1). Expected %d, but was %d.", CV_32SC1, _inm_labels.type());
        CV_Error(CV_StsUnsupportedFormat, error_message);
    }

    // get the vector of matrices
    std::vector<Mat> src;
    _in_src.getMatVector(src);

    // get the label matrix
    Mat labels = _inm_labels.getMat();

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
        m_histograms.clear();
    }

    // append labels to m_labels matrix
    for (size_t labelIdx = 0; labelIdx < labels.total(); labelIdx++)
    {
        m_labels.push_back(labels.at<int>((int)labelIdx));
    }

    // store the spatial histograms of the original data
    for (size_t sampleIdx = 0; sampleIdx < src.size(); sampleIdx++)
    {
        // calculate lbp image
        Mat lbp_image = elbp(src[sampleIdx], m_radius, m_neighbors);
        // get spatial histogram from this lbp image
        Mat p = spatial_histogram(lbp_image,                                                         /* lbp_image                   */
                                  static_cast<int>(std::pow(2.0, static_cast<double>(m_neighbors))), /* number of possible patterns */
                                  m_grid_x,                                                          /* grid size x                 */
                                  m_grid_y,                                                          /* grid size y                 */
                                  true
                                 );
        // add to templates
        m_histograms.push_back(p);
    }
}

void LBPHFaceRecognizer::predict(cv::InputArray _src, cv::Ptr<cv::face::PredictCollector> collector) const
{
    if (m_histograms.empty())
    {
        // throw error if no data (or simply return -1?)
        String error_message = "This LBPH model is not computed yet. Did you call the train method?";
        CV_Error(CV_StsBadArg, error_message);
    }

    Mat src = _src.getMat();

    // get the spatial histogram from input image
    Mat lbp_image = elbp(src, m_radius, m_neighbors);
    Mat query     = spatial_histogram(lbp_image,                                                         /* lbp_image                   */
                                      static_cast<int>(std::pow(2.0, static_cast<double>(m_neighbors))), /* number of possible patterns */
                                      m_grid_x,                                                          /* grid size x                 */
                                      m_grid_y,                                                          /* grid size y                 */
                                      true                                                               /* normed histograms           */
                                     );
    collector->init((int)m_histograms.size());

    // This is the standard method

    if (m_statisticsMode == NearestNeighbor)
    {
        // find 1-nearest neighbor
        for (size_t sampleIdx = 0 ; sampleIdx < m_histograms.size() ; sampleIdx++)
        {
            double dist = compareHist(m_histograms[sampleIdx], query, CV_COMP_CHISQR);

            int label = m_labels.at<int>((int) sampleIdx);

            if (!collector->collect(label, dist))
            {
                return;
            }
        }
    }

    // All other methods are just unvalidated examples.
    // Development can take place only if there is proper alignment available

    else if (m_statisticsMode == NearestMean)
    {
        // Create map "label -> vector of distances to all histograms for this label"
        std::map<int, std::vector<int> > distancesMap;

        for (size_t sampleIdx = 0; sampleIdx < m_histograms.size(); sampleIdx++)
        {
            double dist                 = compareHist(m_histograms[sampleIdx], query, CV_COMP_CHISQR);
            std::vector<int>& distances = distancesMap[m_labels.at<int>((int) sampleIdx)];
            distances.push_back(dist);
        }

        // Compute mean
        QString s = QString::fromLatin1("Mean distances: ");
        std::map<int, std::vector<int> >::const_iterator it;

        for (it = distancesMap.begin(); it != distancesMap.end(); ++it)
        {
            double sum = 0;

            for (std::vector<int>::const_iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2)
            {
                sum += *it2;
            }

            double mean = sum / it->second.size();
            s          += QString::fromLatin1("%1: %2 - ").arg(it->first).arg(mean);

            if (!collector->collect(it->first, mean))
            {
                return;
            }
        }

        qCDebug(DIGIKAM_FACESENGINE_LOG) << s;
    }
    else if (m_statisticsMode == MostNearestNeighbors)
    {
        // Create map "distance -> label"
        std::multimap<double, int> distancesMap;

        // map "label -> number of histograms"
        std::map<int, int> countMap;

        for (size_t sampleIdx = 0; sampleIdx < m_histograms.size(); sampleIdx++)
        {
            int label   = m_labels.at<int>((int) sampleIdx);
            double dist = compareHist(m_histograms[sampleIdx], query, CV_COMP_CHISQR);
            distancesMap.insert(std::pair<double, int>(dist, label));
            countMap[label]++;
        }

        int nearestElementCount = cv::min(100, int(distancesMap.size()/3+1));

        // map "label -> number of nearest neighbors"
        std::map<int, int> scoreMap;

        for (std::multimap<double, int>::iterator it = distancesMap.begin(); it != distancesMap.end() && nearestElementCount != 0; ++it, nearestElementCount--)
        {
            scoreMap[it->second]++;
        }

        QString s = QString::fromLatin1("Nearest Neighbor score: ");

        for (std::map<int,int>::iterator it = scoreMap.begin(); it != scoreMap.end(); ++it)
        {
            double score = double(it->second) / countMap.at(it->first);
            s           += QString::fromLatin1("%1/%2 %3  ").arg(it->second).arg(countMap.at(it->first)).arg(score);

            // large is better thus it is -score.
            if (!collector->collect(it->first, -score))
            {
                return;
            }
        }

        qCDebug(DIGIKAM_FACESENGINE_LOG) << s;
    }
}

// Static method ----------------------------------------------------

Ptr<LBPHFaceRecognizer> LBPHFaceRecognizer::create(int radius, int neighbors, int grid_x, int grid_y, double threshold, PredictionStatistics statistics)
{
    Ptr<LBPHFaceRecognizer> ptr;

    LBPHFaceRecognizer* const fr = new LBPHFaceRecognizer(radius, neighbors, grid_x, grid_y, threshold, statistics);

    if (!fr)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "Cannot create LBPHFaceRecognizer instance";
        return ptr;
    }

    ptr = Ptr<LBPHFaceRecognizer>(fr);

    if (ptr.empty())
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "LBPHFaceRecognizer instance is empty";
    }

    return ptr;
}

} // namespace Digikam
