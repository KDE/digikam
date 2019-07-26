/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-07-13
 * Description : Face recognition using deep learning
 *
 * Copyright (C) 2017      by Yingjie Liu <yingjiewudi at gmail dot com>
 * Copyright (C) 2017-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2019 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#include "facerec_dnnborrowed.h"

// C++ includes

#include <set>
#include <limits>
#include <vector>
#include <cmath>
#include <cassert>
#include <numeric>

// Qt includes

#include <QFile>
#include <QDataStream>
#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"

using namespace cv;

/** This compute cosine distance between 2 vectors with formula:
 *      cos(a) = (v1*v2) / (||v1||*||v2||)
 */
static double cosineDistance(std::vector<float> v1, std::vector<float> v2)
{
    assert(v1.size() == v2.size());

    double scalarProduct = std::inner_product(v1.begin(), v1.end(), v2.begin(), 0.0);
    double normV1 = sqrt(std::inner_product(v1.begin(), v1.end(), v1.begin(), 0.0));
    double normV2 = sqrt(std::inner_product(v2.begin(), v2.end(), v2.begin(), 0.0));

    return scalarProduct / normV1*normV2;
}

namespace Digikam
{

void DNNFaceRecognizer::train(std::vector<std::vector<float> > _in_src, InputArray _inm_labels)
{
    this->train(_in_src, _inm_labels, false);
}

void DNNFaceRecognizer::update(std::vector<std::vector<float> > _in_src, InputArray _inm_labels)
{
    // got no data, just return
    if (_in_src.size() == 0)
    {
        return;
    }

    this->train(_in_src, _inm_labels, true);
}

/** This train function is used to store the face vectors, not training
 */
void DNNFaceRecognizer::train(std::vector<std::vector<float> > _in_src, InputArray _inm_labels, bool preserveData)
{
    // get the vector of matrices
    std::vector<std::vector<float> > src = _in_src;

    // get the label matrix
    cv::Mat labels                       = _inm_labels.getMat();

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
    for (size_t labelIdx = 0 ; labelIdx < labels.total() ; ++labelIdx)
    {
        m_labels.push_back(labels.at<int>((int)labelIdx));
        m_src.push_back(src[(int)labelIdx]);
    }

    return ;
}

void DNNFaceRecognizer::predict(cv::InputArray _src, int& label, double& dist,
                                DNNFaceExtractor* const extractor) const
{
    qCWarning(DIGIKAM_FACESENGINE_LOG) << "Predicting face image";

    cv::Mat src              = _src.getMat();//254*254
    std::vector<float> vecdata;
    extractor->getFaceEmbedding(src, vecdata);
    qCWarning(DIGIKAM_FACESENGINE_LOG) << "m_threshold " << m_threshold;
    qCWarning(DIGIKAM_FACESENGINE_LOG) << "vecdata: " << vecdata[vecdata.size()-2] << " " << vecdata[vecdata.size()-1];

    dist  = -1;
    label = -1;

    // find nearest neighbor

    for (size_t sampleIdx = 0 ; sampleIdx < m_src.size() ; ++sampleIdx)
    {
/*        
        double dist = 0;

        for (size_t i = 0 ; i < m_src[sampleIdx].size() ; ++i)
        {
            dist += (vecdata[i]-m_src[sampleIdx][i])*(vecdata[i]-m_src[sampleIdx][i]);
        }

        dist = std::sqrt(dist);

        if ((dist < minDist) && (dist < m_threshold))
        {
            minDist  = dist;
            minClass = m_labels.at<int>((int) sampleIdx);
        }
*/

        double newDist = cosineDistance(vecdata, m_src[sampleIdx]);

        if(newDist > dist)
        {
            dist = newDist;
            label = m_labels.at<int>((int) sampleIdx);
        }
    }
}

int DNNFaceRecognizer::predict(InputArray _src) const
{
    int    label;
    double dummy;
    //predict(_src, label, dummy);

    return label;
}

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
