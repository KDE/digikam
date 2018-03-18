/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-20
 * Description : Metadata Settings Container.
 *
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

#ifndef METADATA_SETTINGS_CONTAINER_H
#define METADATA_SETTINGS_CONTAINER_H

// Qt includes

#include <QFlags>

// Local includes

#include "digikam_export.h"
#include "metaengine.h"

class QStringList;

class KConfigGroup;

namespace Digikam
{

/**
    The class MetadataSettingsContainer encapsulates all metadata related settings.
    NOTE: this allows supply changed arguments to MetadataHub without changing the global settings.
*/
class DIGIKAM_EXPORT MetadataSettingsContainer
{
public:

    explicit MetadataSettingsContainer();
    ~MetadataSettingsContainer()
    {
    };

    /**
     * Describes the allowed and desired operation when rotating a picture.
     * The modes are in escalating order and describe if an operation is allowed.
     * What is actually done will be goverend by what is possible:
     * 1) RAW files cannot by rotated by content, setting the metadata may be problematic
     * 2) Read-Only files cannot edited, neither content nor metadata
     * 3) Writable files wil have lossy compression
     * 4) Only JPEG and PGF offer lossless rotation
     * Using a contents-based rotation always implies resetting the flag.
     */
    enum RotationBehaviorFlag
    {
        NoRotation               = 0,
        RotateByInternalFlag     = 1 << 0,
        RotateByMetadataFlag     = 1 << 1,
        RotateByLosslessRotation = 1 << 2,
        RotateByLossyRotation    = 1 << 3,

        RotatingFlags            = RotateByInternalFlag     | RotateByMetadataFlag,
        RotatingPixels           = RotateByLosslessRotation | RotateByLossyRotation
    };
    Q_DECLARE_FLAGS(RotationBehaviorFlags, RotationBehaviorFlag)

public:

    void readFromConfig(KConfigGroup& group);
    void writeToConfig(KConfigGroup& group) const;

public:

    bool                            exifRotate;
    bool                            exifSetOrientation;

    bool                            saveComments;
    bool                            saveDateTime;
    bool                            savePickLabel;
    bool                            saveColorLabel;
    bool                            saveRating;

    bool                            saveTemplate;
    bool                            saveTags;
    bool                            saveFaceTags;

    bool                            writeRawFiles;
    bool                            updateFileTimeStamp;
    bool                            rescanImageIfModified;
    bool                            useXMPSidecar4Reading;
    bool                            useLazySync;

    MetaEngine::MetadataWritingMode metadataWritingMode;

    RotationBehaviorFlags           rotationBehavior;

    QStringList                     sidecarExtensions;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::MetadataSettingsContainer::RotationBehaviorFlags)

#endif // METADATA_SETTINGS_CONTAINER_H
