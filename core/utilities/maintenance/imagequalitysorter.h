/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : image quality sorter
 *
 * Copyright (C) 2013-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_IMAGE_QUALITY_SORTER_H
#define DIGIKAM_IMAGE_QUALITY_SORTER_H

// Qt includes

#include <QObject>

// Local includes

#include "album.h"
#include "maintenancetool.h"
#include "imagequalitycontainer.h"

class QImage;

namespace Digikam
{

class ImageQualitySorter : public MaintenanceTool
{
    Q_OBJECT

public:

    enum QualityScanMode
    {
        AllItems = 0,        // Clean all Pick Labels assignements and re-scan all items.
        NonAssignedItems     // Scan only items with no Pick Labels assigned.
    };

public:

    /** Constructor using AlbumList as argument. If list is empty, whole Albums collection is processed.
     */
    explicit ImageQualitySorter(QualityScanMode mode,
                                const AlbumList& list=AlbumList(),
                                const ImageQualityContainer& quality=ImageQualityContainer(),
                                ProgressItem* const parent = nullptr);
    ~ImageQualitySorter();

    void setUseMultiCoreCPU(bool b) override;

private:

    void processOne();

private Q_SLOTS:

    void slotStart() override;
    void slotCancel() override;
    void slotAdvance(const QImage&);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_IMAGE_QUALITY_SORTER_H
