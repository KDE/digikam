/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-20
 * Description : central place for MetaEngine settings
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

#ifndef DIGIKAM_META_ENGINE_SETTINGS_H
#define DIGIKAM_META_ENGINE_SETTINGS_H

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"
#include "metaenginesettingscontainer.h"

namespace Digikam
{

class DIGIKAM_EXPORT MetaEngineSettings : public QObject
{
    Q_OBJECT

public:

    /**
     * Global container for Metadata settings. All accessor methods are thread-safe.
     */
    static MetaEngineSettings* instance();

    /**
     * Returns the current Metadata settings.
     */
    MetaEngineSettingsContainer settings() const;

    /**
     * Sets the current Metadata settings and writes them to config.
     */
    void setSettings(const MetaEngineSettingsContainer& settings);

    /**
     * Shortcut to get exif rotation settings from container.
     */
    bool exifRotate() const;

Q_SIGNALS:

    void settingsChanged();
    void settingsChanged(const MetaEngineSettingsContainer& current, const MetaEngineSettingsContainer& previous);

private:

    explicit MetaEngineSettings();
    ~MetaEngineSettings();

    void readFromConfig();

private:

    class Private;
    Private* const d;

    friend class MetaEngineSettingsCreator;
};

} // namespace Digikam

#endif // DIGIKAM_META_ENGINE_SETTINGS_H
