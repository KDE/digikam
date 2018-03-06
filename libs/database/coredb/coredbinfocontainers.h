/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-04
 * Description : Container classes holding user presentable information
 *
 * Copyright (C) 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef COREDATABASEINFOCONTAINERS_H
#define COREDATABASEINFOCONTAINERS_H

// Qt includes

#include <QString>
#include <QDateTime>

namespace Digikam
{

class ImageCommonContainer
{
public:

    ImageCommonContainer()
    {
        fileSize   = 0;
        rating     = -1;
        width      = 0;
        height     = 0;
        colorDepth = 0;
    }

    QString   fileName;
    QDateTime fileModificationDate;
    qint64    fileSize;

    int       rating;
    QDateTime creationDate;
    QDateTime digitizationDate;
    QString   orientation;
    int       width;
    int       height;
    QString   format;
    int       colorDepth; // bits per channel, 8/16
    QString   colorModel;
};

// ------------------------------------------------------------

class ImageMetadataContainer
{
public:

    ImageMetadataContainer()
    {
        allFieldsNull = true;
    }

    bool allFieldsNull;

    QString make;
    QString model;
    QString lens;
    QString aperture;
    QString focalLength;
    QString focalLength35;
    QString exposureTime;
    QString exposureProgram;
    QString exposureMode;
    QString sensitivity;
    QString flashMode;
    QString whiteBalance;
    QString whiteBalanceColorTemperature;
    QString meteringMode;
    QString subjectDistance;
    QString subjectDistanceCategory;
};

// ------------------------------------------------------------

class VideoMetadataContainer
{
public:

    VideoMetadataContainer()
    {
        allFieldsNull = true;
    }

    bool    allFieldsNull;

    QString aspectRatio;
    QString audioBitRate;
    QString audioChannelType;
    QString audioCodec;
    QString duration;
    QString frameRate;
    QString videoCodec;
};

} // namespace Digikam

#endif // COREDATABASEINFOCONTAINERS_H
