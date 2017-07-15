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

#include "dnnfacemodel.h"

// Qt includes

#include <QList>

// local includes

#include "digikam_debug.h"

namespace Digikam
{

DNNFaceVecMetadata::DNNFaceVecMetadata()
    : databaseId(-1),
      identity(0),
      storageStatus(Created)
{
}

DNNFaceVecMetadata::~DNNFaceVecMetadata()
{
}

// ------------------------------------------------------------------------------------

DNNFaceModel::DNNFaceModel()
    : cv::Ptr<DNNFaceRecognizer>(DNNFaceRecognizer::create())/*,
      databaseId(0)*/
{
    ptr()->setThreshold(0.8);
}

DNNFaceModel::~DNNFaceModel()
{
}

DNNFaceRecognizer* DNNFaceModel::ptr()
{
    DNNFaceRecognizer* const ptr = cv::Ptr<DNNFaceRecognizer>::operator Digikam::DNNFaceRecognizer*();

    if (!ptr)
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "DNNFaceRecognizer pointer is null";

    return ptr;
}

const DNNFaceRecognizer* DNNFaceModel::ptr() const
{
#if OPENCV_TEST_VERSION(3,0,0)
    const DNNFaceRecognizer* const ptr = cv::Ptr<DNNFaceRecognizer>::operator const Digikam::DNNFaceRecognizer*();
#else
    const DNNFaceRecognizer* const ptr = cv::Ptr<DNNFaceRecognizer>::operator Digikam::DNNFaceRecognizer*();
#endif

    if (!ptr)
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "DNNFaceRecognizer pointer is null";

    return ptr;
}

std::vector<std::vector<float>> DNNFaceModel::getSrc() const
{
    return ptr()->getSrc();
}

void DNNFaceModel::setSrc(std::vector<std::vector<float>> new_src)
{
    ptr()->setSrc(new_src);
}

cv::Mat DNNFaceModel::getLabels() const
{
    return ptr()->getLabels();
}

void DNNFaceModel::setLabels(cv::Mat new_labels)
{
    ptr()->setLabels(new_labels);
}

/*
OpenCVMatData DNNFaceModel::matData(int index) const
{
    return OpenCVMatData(ptr()->getSrc().at(index));
}
*/

QList<DNNFaceVecMetadata> DNNFaceModel::matMetadata() const
{
    return m_vecMetadata;
}

void DNNFaceModel::setWrittenToDatabase(int index, int id)
{
    m_vecMetadata[index].databaseId    = id;
    m_vecMetadata[index].storageStatus = DNNFaceVecMetadata::InDatabase;
}

void DNNFaceModel::setMats(const QList<std::vector<float>>& mats, const QList<DNNFaceVecMetadata>& matMetadata)
{
    /*
     * Does not work with standard OpenCV, as these two params are declared read-only in OpenCV.
     * One reason why we copied the code.
     */
    std::vector<std::vector<float>> newSrc;
    cv::Mat newLabels;
    newSrc.reserve(mats.size());
    newLabels.reserve(matMetadata.size());

    foreach (const std::vector<float>& mat, mats)
    {
        newSrc.push_back(mat);
    }

    m_vecMetadata.clear();

    foreach (const DNNFaceVecMetadata& metadata, matMetadata)
    {
        newLabels.push_back(metadata.identity);
        m_vecMetadata << metadata;
    }

    std::vector<std::vector<float>> currentSrcs = ptr()->getSrc();
    cv::Mat currentLabels                       = ptr()->getLabels();

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

void DNNFaceModel::update(const std::vector<cv::Mat>& images, const std::vector<int>& labels, const QString& context)
{
    //this function will be deleted later
    return ;
    /*
    ptr()->update(images, labels);

    // Update local information
    // We assume new labels are simply appended
    cv::Mat currentLabels = ptr()->getLabels();

    for (int i = m_vecMetadata.size() ; i < currentLabels.rows ; i++)
    {
        DNNFaceVecMetadata metadata;
        metadata.storageStatus = DNNFaceVecMetadata::Created;
        metadata.identity      = currentLabels.at<int>(i);
        metadata.context       = context;
        m_vecMetadata << metadata;
    }
    */
}

} // namespace Digikam
