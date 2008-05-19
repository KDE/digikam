/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-17
 * Description : Haar Database interface
 *
 * Copyright (C) 2003 by Ricardo Niederberger Cabral <nieder at mail dot ru>
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef HAARIFACE_H
#define HAARIFACE_H

// C++ Includes

#include <map>
#include <list>

// Qt includes.

#include <QString>

// Local includes.

#include "haar.h"

class QImage;

namespace Digikam
{

class DImg;

namespace Haar
{
    class SignatureData; 
}

class HaarIfacePriv;
class HaarIface
{

public:

    enum SketchType
    {
        ScannedSketch   = 0,
        HanddrawnSketch = 1
    };

public:

    HaarIface();
    ~HaarIface();

    static int preferredSize();

    bool indexImage(const QString& filename);
    bool indexImage(const QString& filename, const QImage &image);
    bool indexImage(const QString& filename, const DImg &image);
    bool indexImage(qlonglong imageid, const QImage &image);
    bool indexImage(qlonglong imageid, const DImg &image);

    QList<qlonglong> bestMatchesForImage(qlonglong imageid, int numberOfResults=20, SketchType type=ScannedSketch);
    QList<qlonglong> bestMatchesForImage(const QImage& image, int numberOfResults=20, SketchType type=ScannedSketch);
    QList<qlonglong> bestMatchesForFile(const QString& filename, int numberOfResults=20, SketchType type=ScannedSketch);

private:

    QImage loadQImage(const QString &filename);

    bool   indexImage(qlonglong imageid);

    QList<qlonglong> bestMatches(Haar::SignatureData *data, int numberOfResults, SketchType type);

private:

    HaarIfacePriv *d;
};

}  // namespace Digikam

#endif // HAARIFACE_H
