/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-05-22
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

#include "dnnfacemodel.h"

// Qt includes

#include <QList>

// local includes

#include "digikam_debug.h"
#include "facedbaccess.h"
#include "facedb.h"
#include "opencvdnnfacerecognizer.h"

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
    ptr()->setThreshold(OpenCVDNNFaceRecognizer::m_threshold);
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

std::vector<std::vector<float> > DNNFaceModel::getSrc() const
{
    return ptr()->getSrc();
}

void DNNFaceModel::setSrc(std::vector<std::vector<float> > new_src)
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

std::vector<float> DNNFaceModel::vecData(int index) const
{
    return ptr()->getSrc().at(index);
}

QList<DNNFaceVecMetadata> DNNFaceModel::vecMetadata() const
{
    return m_vecMetadata;
}

void DNNFaceModel::setWrittenToDatabase(int index, int id)
{
    m_vecMetadata[index].databaseId    = id;
    m_vecMetadata[index].storageStatus = DNNFaceVecMetadata::InDatabase;
}

void DNNFaceModel::setMats(const QList<std::vector<float> >& mats, const QList<DNNFaceVecMetadata>& matMetadata)
{
    /*
     * Does not work with standard OpenCV, as these two params are declared read-only in OpenCV.
     * One reason why we copied the code.
     */
    std::vector<std::vector<float> > newSrc;
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

    std::vector<std::vector<float> > currentSrcs = ptr()->getSrc();
    cv::Mat currentLabels                        = ptr()->getLabels();

    currentSrcs.insert(currentSrcs.end(), newSrc.begin(), newSrc.end());
    currentLabels.push_back(newLabels);

    //ptr()->setSrc(currentSrcs);
    //ptr()->setLabels(currentLabels);

    //make sure that there exits training data
    if (currentSrcs.size() > 0)
    {
        ptr()->train(currentSrcs, currentLabels);
    }
}

void DNNFaceModel::update(const std::vector<cv::Mat>& images, const std::vector<int>& labels, const QString& context,
                          DNNFaceExtractor* const extractor)
{
    std::vector<std::vector<float> > src;

    foreach (const cv::Mat& mat, images)
    {
        std::vector<float> vecdata;
        // FaceDbAccess().db()->getFaceVector(mat, vecdata);
        extractor->getFaceEmbedding(mat, vecdata);
        qCDebug(DIGIKAM_FACEDB_LOG) << "vecdata: " << vecdata[vecdata.size()-2]
                                                   << vecdata[vecdata.size()-1];
        src.push_back(vecdata);
    }

    ptr()->update(src, labels);

    // Update local information
    // We assume new labels are simply appended
    cv::Mat currentLabels = ptr()->getLabels();

    for (int i = m_vecMetadata.size() ; i < currentLabels.rows ; ++i)
    {
        DNNFaceVecMetadata metadata;
        metadata.storageStatus = DNNFaceVecMetadata::Created;
        metadata.identity      = currentLabels.at<int>(i);
        metadata.context       = context;
        m_vecMetadata << metadata;
    }
}

} // namespace Digikam
