/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 2013-05-26
 *
 * Copyright (C) 2013 by Marcel Wiesweg <marcel dot wiesweg at uk-essen dot de>
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

#ifndef DATABASEFACECONTAINERS_H
#define DATABASEFACECONTAINERS_H

// OpenCV includes

#include "libopencv.h"

// Qt includes

#include <QByteArray>

namespace FacesEngine
{

class OpenCVMatData
{
public:

    /// Facilitates cv::Mat storage in DB (where Qt interface requires Qt types)

    OpenCVMatData();
    OpenCVMatData(const cv::Mat& mat);

    /// Creates a deep copy of the QByteArray
    cv::Mat toMat() const;
    /// Creates a deep copy of the cv::Mat
    void setMat(const cv::Mat& mat);

    void clearData();

public:

    int        type;
    int        rows;
    int        cols;
    QByteArray data;
};

} // namespace FacesEngine

#endif // DATABASEFACECONTAINERS_H
