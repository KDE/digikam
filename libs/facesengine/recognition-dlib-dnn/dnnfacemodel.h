/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date    2017-05-22
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

    DNNFaceVecMetadata();
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

    DNNFaceModel();
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

    /// Make sure to call this instead of FaceRecognizer::update directly!
    void update(const std::vector<cv::Mat>& images, const std::vector<int>& labels, const QString& context);

//public:

    //int databaseId;

protected:

    QList<DNNFaceVecMetadata> m_vecMetadata;
};

} // namespace Digikam

#endif // DNN_FACE_MODEL_H
