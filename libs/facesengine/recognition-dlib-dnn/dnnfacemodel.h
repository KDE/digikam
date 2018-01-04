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

#ifndef DNN_FACE_MODEL_H
#define DNN_FACE_MODEL_H

// OpenCV library

#include "libopencv.h"

// Qt include

#include <QList>

// Local includes

#include "opencvmatdata.h"
#include "facerec_dnnborrowed.h"

namespace Digikam
{

class DNNFaceVecMetadata
{
public:

    enum StorageStatus
    {
        Created,
        InDatabase
    };

public:

    explicit DNNFaceVecMetadata();
    ~DNNFaceVecMetadata();

public:

    int           databaseId;
    int           identity;
    QString       context;

    StorageStatus storageStatus;
};

// -------------------------------------------------------------------------------------------------------------------------------------

class DNNFaceModel : public cv::Ptr<DNNFaceRecognizer>
{
public:

    explicit DNNFaceModel();
    ~DNNFaceModel();

    DNNFaceRecognizer*       ptr();
    const DNNFaceRecognizer* ptr() const;

    //Getter function
    std::vector<std::vector<float>> getSrc() const;
    void setSrc(std::vector<std::vector<float>> new_src);

    cv::Mat getLabels() const;
    void setLabels(cv::Mat new_labels);

    QList<DNNFaceVecMetadata>   matMetadata() const;
    //OpenCVMatData               matData(int index) const;

    void setWrittenToDatabase(int index, int databaseId);

    void setMats(const QList<std::vector<float>>& mats, const QList<DNNFaceVecMetadata>& matMetadata);

//public:

    //int databaseId;

protected:

    QList<DNNFaceVecMetadata> m_vecMetadata;
};

} // namespace Digikam

#endif // DNN_FACE_MODEL_H
