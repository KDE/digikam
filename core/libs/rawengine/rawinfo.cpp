/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-05-02
 * Description : RAW file identification information container
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local includes

#include "rawinfo.h"

namespace Digikam
{

RawInfo::RawInfo()
{
    sensitivity       = -1.0;
    exposureTime      = -1.0;
    aperture          = -1.0;
    focalLength       = -1.0;
    pixelAspectRatio  = 1.0;    // Default value. This can be unavailable (depending of camera model).
    rawColors         = -1;
    rawImages         = -1;
    hasIccProfile     = false;
    isDecodable       = false;
    daylightMult[0]   = 0.0;
    daylightMult[1]   = 0.0;
    daylightMult[2]   = 0.0;
    cameraMult[0]     = 0.0;
    cameraMult[1]     = 0.0;
    cameraMult[2]     = 0.0;
    cameraMult[3]     = 0.0;
    blackPoint        = 0;

    for (int ch = 0 ; ch < 4 ; ch++)
    {
        blackPointCh[ch] = 0;
    }

    whitePoint        = 0;
    topMargin         = 0;
    leftMargin        = 0;
    orientation       = ORIENTATION_NONE;

    for (int x = 0 ; x < 3 ; x++)
    {
        for (int y = 0 ; y < 4 ; y++)
        {
            cameraColorMatrix1[x][y] = 0.0;
            cameraColorMatrix2[x][y] = 0.0;
            cameraXYZMatrix[y][x]    = 0.0;       // NOTE: see B.K.O # 253911 : [y][x] not [x][y]
        }
    }
}

RawInfo::~RawInfo()
{
}

bool RawInfo::isEmpty()
{
    if (make.isEmpty()                  &&
        model.isEmpty()                 &&
        filterPattern.isEmpty()         &&
        colorKeys.isEmpty()             &&
        DNGVersion.isEmpty()            &&
        exposureTime     == -1.0        &&
        aperture         == -1.0        &&
        focalLength      == -1.0        &&
        pixelAspectRatio == 1.0         &&
        sensitivity      == -1.0        &&
        rawColors        == -1          &&
        rawImages        == -1          &&
        blackPoint       == 0           &&
        blackPointCh[0]  == 0           &&
        blackPointCh[1]  == 0           &&
        blackPointCh[2]  == 0           &&
        blackPointCh[3]  == 0           &&
        whitePoint       == 0           &&
        topMargin        == 0           &&
        leftMargin       == 0           &&
        !dateTime.isValid()             &&
        !imageSize.isValid()            &&
        !fullSize.isValid()             &&
        !outputSize.isValid()           &&
        !thumbSize.isValid()            &&
        cameraColorMatrix1[0][0] == 0.0 &&
        cameraColorMatrix1[0][1] == 0.0 &&
        cameraColorMatrix1[0][2] == 0.0 &&
        cameraColorMatrix1[0][3] == 0.0 &&
        cameraColorMatrix1[1][0] == 0.0 &&
        cameraColorMatrix1[1][1] == 0.0 &&
        cameraColorMatrix1[1][2] == 0.0 &&
        cameraColorMatrix1[1][3] == 0.0 &&
        cameraColorMatrix1[2][0] == 0.0 &&
        cameraColorMatrix1[2][1] == 0.0 &&
        cameraColorMatrix1[2][2] == 0.0 &&
        cameraColorMatrix1[2][3] == 0.0 &&
        cameraColorMatrix2[0][0] == 0.0 &&
        cameraColorMatrix2[0][1] == 0.0 &&
        cameraColorMatrix2[0][2] == 0.0 &&
        cameraColorMatrix2[0][3] == 0.0 &&
        cameraColorMatrix2[1][0] == 0.0 &&
        cameraColorMatrix2[1][1] == 0.0 &&
        cameraColorMatrix2[1][2] == 0.0 &&
        cameraColorMatrix2[1][3] == 0.0 &&
        cameraColorMatrix2[2][0] == 0.0 &&
        cameraColorMatrix2[2][1] == 0.0 &&
        cameraColorMatrix2[2][2] == 0.0 &&
        cameraColorMatrix2[2][3] == 0.0 &&
        cameraXYZMatrix[0][0]    == 0.0 &&
        cameraXYZMatrix[0][1]    == 0.0 &&
        cameraXYZMatrix[0][2]    == 0.0 &&
        cameraXYZMatrix[1][0]    == 0.0 &&
        cameraXYZMatrix[1][1]    == 0.0 &&
        cameraXYZMatrix[1][2]    == 0.0 &&
        cameraXYZMatrix[2][0]    == 0.0 &&
        cameraXYZMatrix[2][1]    == 0.0 &&
        cameraXYZMatrix[2][2]    == 0.0 &&
        cameraXYZMatrix[3][0]    == 0.0 &&
        cameraXYZMatrix[3][1]    == 0.0 &&
        cameraXYZMatrix[3][2]    == 0.0 &&
        orientation              == ORIENTATION_NONE
       )
    {
        return true;
    }
    else
    {
        return false;
    }
}

QDebug operator<<(QDebug dbg, const RawInfo& c)
{
    dbg.nospace() << "RawInfo::sensitivity: "      << c.sensitivity      << ", ";
    dbg.nospace() << "RawInfo::exposureTime: "     << c.exposureTime     << ", ";
    dbg.nospace() << "RawInfo::aperture: "         << c.aperture         << ", ";
    dbg.nospace() << "RawInfo::focalLength: "      << c.focalLength      << ", ";
    dbg.nospace() << "RawInfo::pixelAspectRatio: " << c.pixelAspectRatio << ", ";
    dbg.nospace() << "RawInfo::rawColors: "        << c.rawColors        << ", ";
    dbg.nospace() << "RawInfo::rawImages: "        << c.rawImages        << ", ";
    dbg.nospace() << "RawInfo::hasIccProfile: "    << c.hasIccProfile    << ", ";
    dbg.nospace() << "RawInfo::isDecodable: "      << c.isDecodable      << ", ";
    dbg.nospace() << "RawInfo::daylightMult: "     << c.daylightMult     << ", ";
    dbg.nospace() << "RawInfo::cameraMult: "       << c.cameraMult       << ", ";
    dbg.nospace() << "RawInfo::blackPoint: "       << c.blackPoint       << ", ";
    dbg.nospace() << "RawInfo::whitePoint: "       << c.whitePoint       << ", ";
    dbg.nospace() << "RawInfo::topMargin: "        << c.topMargin        << ", ";
    dbg.nospace() << "RawInfo::leftMargin: "       << c.leftMargin       << ", ";
    dbg.nospace() << "RawInfo::orientation: "      << c.orientation;
    return dbg.space();
}

} // namespace Digikam
