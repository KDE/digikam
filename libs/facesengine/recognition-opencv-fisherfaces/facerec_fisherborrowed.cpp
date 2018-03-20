/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-06-10
 * Description : Face Recognition based on Fisherfaces
 *               http://docs.opencv.org/2.4/modules/contrib/doc/facerec/facerec_tutorial.html#Fisherfaces
 *               Turk, Matthew A and Pentland, Alex P. "Face recognition using Fisherfaces." 
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

#include "facerec_fisherborrowed.h"

// C++ includes

#include <set>
#include <limits>
#include <vector>

// Local includes

#include "digikam_debug.h"

using namespace cv;

namespace Digikam
{

//------------------------------------------------------------------------------
// Fisherfaces
//------------------------------------------------------------------------------

inline Mat asRowMatrix(std::vector<Mat> src, int rtype, double alpha=1, double beta=0)
{
    // number of samples
    size_t n = src.size();

    // return empty matrix if no matrices given
    if (n == 0)
        return Mat();

    // dimensionality of (reshaped) samples
    size_t d = src[0].total();

    // create data matrix
    Mat data((int)n, (int)d, rtype);

    // now copy data
    for (unsigned int i = 0 ; i < n ; i++)
    {
        Mat xi = data.row(i);

        // make reshape happy by cloning for non-continuous matrices
        if (src[i].isContinuous())
        {
            src[i].reshape(1, 1).convertTo(xi, rtype, alpha, beta);
        }
        else
        {
            src[i].clone().reshape(1, 1).convertTo(xi, rtype, alpha, beta);
        }
    }

    return data;
}

// Removes duplicate elements in a given vector.
template<typename _Tp>
inline std::vector<_Tp> remove_dups(const std::vector<_Tp>& src) 
{
    typedef typename std::set<_Tp>::const_iterator constSetIterator;
    typedef typename std::vector<_Tp>::const_iterator constVecIterator;
    std::set<_Tp> set_elems;

    for (constVecIterator it = src.begin() ; it != src.end() ; ++it)
        set_elems.insert(*it);

    std::vector<_Tp> elems;

    for (constSetIterator it = set_elems.begin() ; it != set_elems.end() ; ++it)
        elems.push_back(*it);

    return elems;
}

void FisherFaceRecognizer::train(InputArrayOfArrays _in_src, InputArray _inm_labels)
{
    this->train(_in_src, _inm_labels, false);
}

void FisherFaceRecognizer::update(InputArrayOfArrays _in_src, InputArray _inm_labels)
{
    // got no data, just return
    if (_in_src.total() == 0)
        return;

    this->train(_in_src, _inm_labels, true);
}

void FisherFaceRecognizer::train(InputArrayOfArrays _in_src, InputArray _inm_labels, bool preserveData)
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
        m_src.clear();
    }

    // append labels to m_labels matrix
    for (size_t labelIdx = 0; labelIdx < labels.total(); labelIdx++)
    {
        m_labels.push_back(labels.at<int>((int)labelIdx));
        m_src.push_back(src[(int)labelIdx]);
    }

    // observations in row
    Mat data = asRowMatrix(m_src, CV_64FC1);

    // number of samples
    int n = data.rows;

    /*
     LDA needs more than one class
     We have to check the labels first
    */
    bool label_flag = false;

    for (int i = 1 ; i < m_labels.rows ; i++)
    {
        if (m_labels.at<int>(i, 0)!=m_labels.at<int>(i-1, 0))
        {
            label_flag = true;
            break;
        }
    }

    if (!label_flag)
    {
        String error_message = format("The labels should contain more than one types.");
        CV_Error(CV_StsBadArg, error_message);
    }

    // clear existing model data
    m_projections.clear();

    std::vector<int> ll;

    for (unsigned int i = 0 ; i < m_labels.total() ; i++)
    {
        ll.push_back(m_labels.at<int>(i));
    }

    // get the number of unique classes
    int C = (int) remove_dups(ll).size();

    // clip number of components to be valid
    m_num_components = (C-1);

    // perform the PCA
    PCA pca(data, Mat(), PCA::DATA_AS_ROW, (n-C));
    LDA lda(pca.project(data),m_labels, m_num_components);

    // Now calculate the projection matrix as pca.eigenvectors * lda.eigenvectors.
    // Note: OpenCV stores the eigenvectors by row, so we need to transpose it!
    gemm(pca.eigenvectors, lda.eigenvectors(), 1.0, Mat(), 0.0, m_eigenvectors, GEMM_1_T);

    // store the projections of the original data
    for (int sampleIdx = 0 ; sampleIdx < data.rows ; sampleIdx++)
    {
        Mat p = LDA::subspaceProject(m_eigenvectors, m_mean, data.row(sampleIdx));
        m_projections.push_back(p);
    }
}

void FisherFaceRecognizer::predict(cv::InputArray _src, cv::Ptr<cv::face::PredictCollector> collector) const
{
    qCWarning(DIGIKAM_FACESENGINE_LOG) << "Predicting face image using fisherfaces";

    if (m_projections.empty())
    {
        // throw error if no data (or simply return -1?)
        String error_message = "This Fisherfaces model is not computed yet. Did you call the train method?";
        CV_Error(CV_StsBadArg, error_message);
    }

    Mat src = _src.getMat();//254*254

    //make sure the size of input image is the same as traing image
    if (m_src.size() >= 1 && (src.rows != m_src[0].rows || src.cols != m_src[0].cols))
    {
        //resize(src, src, Size(m_src[0].rows, m_src[0].cols), (0, 0), (0, 0), INTER_LINEAR);
        resize(src, src, Size(m_src[0].rows, m_src[0].cols));
    }

    collector->init(0);//here need to confirm

    Mat q = LDA::subspaceProject(m_eigenvectors, m_mean, src.reshape(1, 1));

    //find nearest neighbor
    for (size_t sampleIdx = 0 ; sampleIdx < m_projections.size() ; sampleIdx++)
    {
        double dist = norm(m_projections[sampleIdx], q, NORM_L2);

        int label = m_labels.at<int>((int) sampleIdx);

        if (!collector->collect(label, dist))
        {
            return;
        }
    }
}

// Static method ----------------------------------------------------

Ptr<FisherFaceRecognizer> FisherFaceRecognizer::create(double threshold)
{
    Ptr<FisherFaceRecognizer> ptr;

    FisherFaceRecognizer* const fr = new FisherFaceRecognizer(threshold);

    if (!fr)
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "Cannot create FisherFaceRecognizer instance";
        return ptr;
    }

    ptr = Ptr<FisherFaceRecognizer>(fr);

    if (ptr.empty())
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "FisherFaceRecognizer instance is empty";
    }

    return ptr;
}

} // namespace Digikam
