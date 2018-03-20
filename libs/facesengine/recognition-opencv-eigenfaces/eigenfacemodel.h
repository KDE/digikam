/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2017-05-22
 * Description : Face Recognition based on Eigenfaces
 *               http://docs.opencv.org/2.4/modules/contrib/doc/facerec/facerec_tutorial.html#eigenfaces
 *               Turk, Matthew A and Pentland, Alex P. "Face recognition using eigenfaces." 
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

#ifndef FACESENGINE_EIGENFACESMODEL_H
#define FACESENGINE_EIGENFACESMODEL_H

// OpenCV library

#include "libopencv.h"

// Qt include

#include <QList>

// Local includes

#include "opencvmatdata.h"
#include "facerec_eigenborrowed.h"

namespace Digikam
{

class EigenFaceMatMetadata
{
public:

    enum StorageStatus
    {
        Created,
        InDatabase
    };

public:

    explicit EigenFaceMatMetadata();
    ~EigenFaceMatMetadata();

public:

    int           databaseId;
    int           identity;
    QString       context;

    StorageStatus storageStatus;
};

// -------------------------------------------------------------------------------------------------------------------------------------

class EigenFaceModel : public cv::Ptr<EigenFaceRecognizer>
{
public:

    explicit EigenFaceModel();
    ~EigenFaceModel();

    EigenFaceRecognizer*       ptr();
    const EigenFaceRecognizer* ptr() const;

    //Getter function
    std::vector<cv::Mat> getSrc() const;
    void setSrc(std::vector<cv::Mat> new_src);

    cv::Mat getLabels() const;
    void setLabels(cv::Mat new_labels);

    QList<EigenFaceMatMetadata> matMetadata() const;
    OpenCVMatData               matData(int index) const;

    void setWrittenToDatabase(int index, int databaseId);

    void setMats(const QList<OpenCVMatData>& mats, const QList<EigenFaceMatMetadata>& matMetadata);

    /// Make sure to call this instead of FaceRecognizer::update directly!
    void update(const std::vector<cv::Mat>& images, const std::vector<int>& labels, const QString& context);

//public:

    //int databaseId;

protected:

    QList<EigenFaceMatMetadata> m_matMetadata;
};

} // namespace Digikam

#endif // FACESENGINE_EIGENFACESMODEL_H
