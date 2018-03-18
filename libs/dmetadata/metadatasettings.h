/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-20
 * Description : central place for Metadata settings
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

#ifndef METADATA_SETTINGS_H
#define METADATA_SETTINGS_H

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"
#include "metadatasettingscontainer.h"

namespace Digikam
{

class DIGIKAM_EXPORT MetadataSettings : public QObject
{
    Q_OBJECT

public:

    /**
     * Global container for Metadata settings. All accessor methods are thread-safe.
     */
    static MetadataSettings* instance();

    /**
     * Returns the current Metadata settings.
     */
    MetadataSettingsContainer settings() const;

    /**
     * Sets the current Metadata settings and writes them to config.
     */
    void setSettings(const MetadataSettingsContainer& settings);

    /**
     * Shortcut to get exif rotation settings from container.
     */
    bool exifRotate() const;

Q_SIGNALS:

    void settingsChanged();
    void settingsChanged(const MetadataSettingsContainer& current, const MetadataSettingsContainer& previous);

private:

    MetadataSettings();
    ~MetadataSettings();

    void readFromConfig();

private:

    class Private;
    Private* const d;

    friend class MetadataSettingsCreator;
};

}  // namespace Digikam

#endif // METADATA_SETTINGS_H
