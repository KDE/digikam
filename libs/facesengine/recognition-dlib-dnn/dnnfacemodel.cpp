/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-05-22
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
    ptr()->setThreshold(0.6);
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
    const DNNFaceRecognizer* const ptr = cv::Ptr<DNNFaceRecognizer>::operator Digikam::DNNFaceRecognizer*();

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

} // namespace Digikam
