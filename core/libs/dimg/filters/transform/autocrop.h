/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-10-18
 * Description : Auto Crop analyzer
 *
 * Copyright (C) 2013 by Sayantan Datta <sayantan dot knz at gmail dot com>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef AUTOCROP_H
#define AUTOCROP_H

// Qt includes

#include <QObject>
#include <QRect>
#include <QImage>

// Local includes

#include "digikam_export.h"
#include "nrfilter.h"
#include "dimg.h"
#include "dimgthreadedanalyser.h"

namespace Digikam
{

class DIGIKAM_EXPORT AutoCrop : public DImgThreadedAnalyser
{
public:

    /** Standard constructor with image container to parse
     */
    explicit AutoCrop(DImg* const orgImage, QObject* const parent = 0);
    ~AutoCrop();

    /** Perform auto-crop analyze to find best inner crop. Use autoInnerCrop()
     *  to get computed area.
     */
    void startAnalyse();

    /** Return inner crop area detected by startAnalyse().
     */
    QRect autoInnerCrop() const;

private :

    /** Takes in a binary image and crops it on the basis of black point
     *  detection, spirally moving outwards.
     *  topCrop can be set to explicitly crop a upper portion of the image
     *  bottomCrop can be set to explicitly crop a bottom portion of the image
     */
    QRect spiralClockwiseTraversal(const QImage& source, int topCrop=-1, int bottomCrop=-1);

private :

    class Private;
    Private* d;
};

}   //  namespace Digikam

#endif /*AUTOCROP_H*/
