/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-18
 * Description : batch face detection
 *
 * Copyright (C) 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#ifndef BATCH_FACES_DETECTOR_H
#define BATCH_FACES_DETECTOR_H

// Qt includes

#include <QObject>

// Local includes

#include "maintenancetool.h"

namespace Digikam
{

class DImg;
class FacePipelinePackage;
class FaceScanSettings;
class ImageInfo;
class ImageInfoList;

class FacesDetector : public MaintenanceTool
{
    Q_OBJECT

public:

    explicit FacesDetector(const FaceScanSettings& settings, ProgressItem* const parent = 0);
    ~FacesDetector();

private Q_SLOTS:

    void slotStart();
    void slotContinueAlbumListing();
    void slotItemsInfo(const ImageInfoList&);
    void slotImagesSkipped(const QList<ImageInfo>&);
    void slotShowOneDetected(const FacePipelinePackage&);
    void slotDone();
    void slotCancel();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // BATCH_FACES_DETECTOR_H
