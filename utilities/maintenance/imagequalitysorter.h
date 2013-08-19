/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : image quality sorter
 *
 * Copyright (C) 2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEQUALITYSORTER_H
#define IMAGEQUALITYSORTER_H

// Local includes

#include "album.h"
#include "maintenancetool.h"
#include "imagequalitysettings.h"

class QImage;

namespace Digikam
{

class ImageQualitySorter : public MaintenanceTool
{
    Q_OBJECT

public:

    /** Constructor using AlbumList as argument. If list is empty, whole Albums collection is processed.
     *  FIXME : set right quality default value.
     */
    explicit ImageQualitySorter(const bool rebuildAll, const AlbumList& list=AlbumList(),
                                const ImageQualitySettings& quality=ImageQualitySettings(),
                                ProgressItem* const parent = 0);
    ~ImageQualitySorter();

    void setUseMultiCoreCPU(bool b);

private:

    void processOne();

private Q_SLOTS:

    void slotStart();
    void slotCancel();
    void slotAdvance(const QImage&);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* IMAGEQUALITYSORTER_H */
