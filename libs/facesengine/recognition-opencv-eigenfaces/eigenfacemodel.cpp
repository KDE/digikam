/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date    2017-05-22
 * @brief   <a href="http://docs.opencv.org/2.4/modules/contrib/doc/facerec/facerec_tutorial.html#eigenfaces">Face Recognition based on Eigenfaces</a>
 *          Turk, Matthew A and Pentland, Alex P. "Face recognition using eigenfaces." 
 *          Computer Vision and Pattern Recognition, 1991. Proceedings {CVPR'91.},
 *          {IEEE} Computer Society Conference on 1991.
 *
 * @section DESCRIPTION
 *
 * @author Copyright (C) 2017 by Yingjie Liu
 *         <a href="mailto:yingjiewudi at gmail dot come">yingjiewudi at gmail dot come</a>
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

#include "eigenfacemodel.h"

// Qt includes

#include <QList>

// local includes

#include "digikam_debug.h"

namespace Digikam
{

EigenFaceMatMetadata::EigenFaceMatMetadata()
    : /*databaseId(0),*/
      identity(0),
      storageStatus(Created)
{
}

EigenFaceMatMetadata::~EigenFaceMatMetadata()
{
}

// ------------------------------------------------------------------------------------

EigenFaceModel::EigenFaceModel()
    : cv::Ptr<EigenFaceRecognizer>(EigenFaceRecognizer::create())/*,
      databaseId(0)*/
{
    ptr()->setThreshold(20000.0);
}

EigenFaceModel::~EigenFaceModel()
{
}

EigenFaceRecognizer* EigenFaceModel::ptr()
{
    EigenFaceRecognizer* const ptr = cv::Ptr<EigenFaceRecognizer>::operator Digikam::EigenFaceRecognizer*();

    if (!ptr)
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "EigenFaceRecognizer pointer is null";

    return ptr;
}

const EigenFaceRecognizer* EigenFaceModel::ptr() const
{
#if OPENCV_TEST_VERSION(3,0,0)
    const EigenFaceRecognizer* const ptr = cv::Ptr<EigenFaceRecognizer>::operator const Digikam::EigenFaceRecognizer*();
#else
    const EigenFaceRecognizer* const ptr = cv::Ptr<EigenFaceRecognizer>::operator Digikam::EigenFaceRecognizer*();
#endif

    if (!ptr)
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "EigenFaceRecognizer pointer is null";

    return ptr;
}

std::vector<cv::Mat> EigenFaceModel::getSrc() const
{
    return ptr()->getSrc();
}

void EigenFaceModel::setSrc(std::vector<cv::Mat> new_src)
{
    ptr()->setSrc(new_src);
}

cv::Mat EigenFaceModel::getLabels() const
{
    return ptr()->getLabels();
}

void EigenFaceModel::setLabels(cv::Mat new_labels)
{
    ptr()->setLabels(new_labels);
}


OpenCVMatData EigenFaceModel::matData(int index) const
{
    return OpenCVMatData(ptr()->getSrc().at(index));
}


QList<EigenFaceMatMetadata> EigenFaceModel::matMetadata() const
{
    return m_matMetadata;
}

void EigenFaceModel::setWrittenToDatabase(int index, int id)
{
    m_matMetadata[index].databaseId    = id;
    m_matMetadata[index].storageStatus = EigenFaceMatMetadata::InDatabase;
}

void EigenFaceModel::setMats(const QList<OpenCVMatData>& mats, const QList<EigenFaceMatMetadata>& matMetadata)
{
    /*
     * Does not work with standard OpenCV, as these two params are declared read-only in OpenCV.
     * One reason why we copied the code.
     */
    std::vector<cv::Mat> newSrc;
    cv::Mat newLabels;
    newSrc.reserve(mats.size());
    newLabels.reserve(matMetadata.size());

    foreach (const OpenCVMatData& mat, mats)
    {
        newSrc.push_back(mat.toMat());
    }

    m_matMetadata.clear();

    foreach (const EigenFaceMatMetadata& metadata, matMetadata)
    {
        newLabels.push_back(metadata.identity);
        m_matMetadata << metadata;
    }

    std::vector<cv::Mat> currentSrcs = ptr()->getSrc();
    cv::Mat currentLabels            = ptr()->getLabels();

    currentSrcs.insert(currentSrcs.end(), newSrc.begin(), newSrc.end());
    currentLabels.push_back(newLabels);

    //ptr()->setSrc(currentSrcs);
    //ptr()->setLabels(currentLabels);
    //make sure that there exits traing data
    if(currentSrcs.size()>0)
    {
        ptr()->train(currentSrcs, currentLabels);
    }
}

void EigenFaceModel::update(const std::vector<cv::Mat>& images, const std::vector<int>& labels, const QString& context)
{
    ptr()->update(images, labels);

    // Update local information
    // We assume new labels are simply appended
    cv::Mat currentLabels = ptr()->getLabels();

    for (int i = m_matMetadata.size() ; i < currentLabels.rows ; i++)
    {
        EigenFaceMatMetadata metadata;
        metadata.storageStatus = EigenFaceMatMetadata::Created;
        metadata.identity      = currentLabels.at<int>(i);
        metadata.context       = context;
        m_matMetadata << metadata;
    }
}

} // namespace Digikam
