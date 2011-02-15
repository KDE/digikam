/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-20
 * Description : Metadata Settings Container.
 *
 * Copyright (C) 2010-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef METADATASETTINGSCONTAINER_H
#define METADATASETTINGSCONTAINER_H

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

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

    MetadataSettingsContainer();
    ~MetadataSettingsContainer() {};

public:

    void readFromConfig(KConfigGroup& group);
    void writeToConfig(KConfigGroup& group) const;

public:

    bool exifRotate;
    bool exifSetOrientation;

    bool saveComments;
    bool saveDateTime;
    bool savePickLabel;
    bool saveColorLabel;
    bool saveRating;

    bool saveTemplate;
    bool saveTags;

    bool writeRawFiles;
    bool updateFileTimeStamp;
    bool useXMPSidecar4Reading;

    int  metadataWritingMode;
};

}  // namespace Digikam

#endif  // METADATASETTINGSCONTAINER_H
