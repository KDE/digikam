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

#include "fisherfacemodel.h"

// Qt includes

#include <QList>

// local includes

#include "digikam_debug.h"

namespace Digikam
{

FisherFaceMatMetadata::FisherFaceMatMetadata()
    : databaseId(-1),
      identity(0),
      storageStatus(Created)
{
}

FisherFaceMatMetadata::~FisherFaceMatMetadata()
{
}

// ------------------------------------------------------------------------------------

FisherFaceModel::FisherFaceModel()
    : cv::Ptr<FisherFaceRecognizer>(FisherFaceRecognizer::create())/*,
      databaseId(0)*/
{
    ptr()->setThreshold(25000.0);
}

FisherFaceModel::~FisherFaceModel()
{
}

FisherFaceRecognizer* FisherFaceModel::ptr()
{
    FisherFaceRecognizer* const ptr = cv::Ptr<FisherFaceRecognizer>::operator Digikam::FisherFaceRecognizer*();

    if (!ptr)
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "FisherFaceRecognizer pointer is null";

    return ptr;
}

const FisherFaceRecognizer* FisherFaceModel::ptr() const
{
    const FisherFaceRecognizer* const ptr = cv::Ptr<FisherFaceRecognizer>::operator Digikam::FisherFaceRecognizer*();

    if (!ptr)
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "FisherFaceRecognizer pointer is null";

    return ptr;
}

std::vector<cv::Mat> FisherFaceModel::getSrc() const
{
    return ptr()->getSrc();
}

void FisherFaceModel::setSrc(std::vector<cv::Mat> new_src)
{
    ptr()->setSrc(new_src);
}

cv::Mat FisherFaceModel::getLabels() const
{
    return ptr()->getLabels();
}

void FisherFaceModel::setLabels(cv::Mat new_labels)
{
    ptr()->setLabels(new_labels);
}


OpenCVMatData FisherFaceModel::matData(int index) const
{
    return OpenCVMatData(ptr()->getSrc().at(index));
}


QList<FisherFaceMatMetadata> FisherFaceModel::matMetadata() const
{
    return m_matMetadata;
}

void FisherFaceModel::setWrittenToDatabase(int index, int id)
{
    m_matMetadata[index].databaseId    = id;
    m_matMetadata[index].storageStatus = FisherFaceMatMetadata::InDatabase;
}

void FisherFaceModel::setMats(const QList<OpenCVMatData>& mats, const QList<FisherFaceMatMetadata>& matMetadata)
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

    foreach (const FisherFaceMatMetadata& metadata, matMetadata)
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

void FisherFaceModel::update(const std::vector<cv::Mat>& images, const std::vector<int>& labels, const QString& context)
{
    ptr()->update(images, labels);

    // Update local information
    // We assume new labels are simply appended
    cv::Mat currentLabels = ptr()->getLabels();

    for (int i = m_matMetadata.size() ; i < currentLabels.rows ; i++)
    {
        FisherFaceMatMetadata metadata;
        metadata.storageStatus = FisherFaceMatMetadata::Created;
        metadata.identity      = currentLabels.at<int>(i);
        metadata.context       = context;
        m_matMetadata << metadata;
    }
}

} // namespace Digikam
