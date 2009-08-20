/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : central place for ICC settings
 *
 * Copyright (C) 2005-2006 by F.J. Cruz <fj.cruz@supercable.es>
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ICCSETTINGS_H
#define ICCSETTINGS_H

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"
#include "iccsettingscontainer.h"

namespace Digikam
{

class IccSettingsPriv;

class DIGIKAM_EXPORT IccSettings : public QObject
{
    Q_OBJECT

public:

    /** Global container for ICC settings. All accessor methods are thread-safe. */

    static IccSettings *instance();

    /// Returns the current ICC settings.
    ICCSettingsContainer settings();

    /**
     * Sets the current ICC settings and writes them to config.
     */
    void setSettings(const ICCSettingsContainer& settings);

    /// Set single parts of the settings
    void setUseManagedView(bool useManagedView);

Q_SIGNALS:

    void settingsChanged();

private:

    void readFromConfig();

    friend class IccSettingsCreator;
    IccSettings();
    ~IccSettings();

    IccSettingsPriv* const d;
};

}  // namespace Digikam

#endif   // ICCSETTINGS_H
