/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2013-26-05
 * Description : Facilitates cv::Mat storage in Face DB
 *               (where Qt interface requires Qt types).
 *
 * Copyright (C) 2013      by Marcel Wiesweg <marcel dot wiesweg at uk-essen dot de>
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "opencvmatdata.h"
#include "digikam_debug.h"

namespace Digikam
{

OpenCVMatData::OpenCVMatData()
    : type(-1),
      rows(0),
      cols(0)
{
}

OpenCVMatData::OpenCVMatData(const cv::Mat& mat)
    : type(-1),
      rows(0),
      cols(0)
{
    setMat(mat);
}

void OpenCVMatData::setMat(const cv::Mat& mat)
{
    type                   = mat.type();
    rows                   = mat.rows;
    cols                   = mat.cols;
    const size_t data_size = cols * rows * mat.elemSize();
    data                   = QByteArray::fromRawData((const char*)mat.ptr(), data_size);
}

cv::Mat OpenCVMatData::toMat() const
{
    // shallow copy (only creates header)

    if (data.isEmpty())
    {
        qCWarning(DIGIKAM_FACESENGINE_LOG) << "Array data to clone is empty.";
    }

    cv::Mat mat(rows, cols, type, (void*)data.constData());

    qCDebug(DIGIKAM_FACESENGINE_LOG) << "Clone Array size [" << rows << ", " << cols << "] of type " << type;

    return mat.clone();
}

void OpenCVMatData::clearData()
{
    type = -1;
    rows = 0;
    cols = 0;
    data.clear();
}

} // namespace Digikam
