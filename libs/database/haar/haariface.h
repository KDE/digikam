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

class DImage;

class HaarIfacePriv;
class SignatureData;
class HaarIface
{

public:

    HaarIface();
    ~HaarIface();

    static int preferredSize();

    bool indexImage(const QString& filename);
    bool indexImage(const QString& filename, const QImage &image);
    bool indexImage(const QString& filename, const DImg &image);
    bool indexImage(qlonglong imageid, const QImage &image);
    bool indexImage(qlonglong imageid, const DImg &image);

    enum SketchType
    {
        ScannedSketch = 0,
        PaintedSketch = 1
    };

private:

    HaarIfacePriv *d;

    QImage      loadQImage(const QString &filename);
    void        fillPixelData(const QImage &image);
    void        fillPixelData(const DImg &image);

    bool indexImage(qlonglong imageid);
    QList<qlonglong> bestMatches(SignatureData *data, int numberOfResults, SketchType type);
    QList<qlonglong> bestMatchesForFile(const QString& filename, int numberOfResults, SketchType type);

#if 0
    /*
    long_list   queryImgDataForThres(sigMap* tsigs, Haar::Idx* sig1, Haar::Idx* sig2, Haar::Idx* sig3,
                                     double* avgl, float thresd, int sketch);
    long_list   queryImgDataForThresFast(sigMap* tsigs, double* avgl, float thresd, int sketch);

    long_list_2 clusterSim(float thresd, bool fast=false);
    int         getNumResults();
    long int    getResultID();
    double      getResultScore();

    void        queryImgID(long int id, int numres);
    */
    //double      calcAvglDiff(long int id1, long int id2);
    //double      calcDiff(long int id1, long int id2);

    // TODO: Marcel, these methods can be removed.
    int         loadDB(char* filename);
    int         saveDB(char* filename);
    int         resetDB();
    void        removeIDFromDB(long int id);

    int         getLongListSize(long_list& li);
    long int    popLongList(long_list& li);
    int         getLongList2Size(long_list_2& li);
    long_list   popLong2List(long_list_2& li);
#endif
};

}  // namespace Digikam

#endif // HAARIFACE_H
