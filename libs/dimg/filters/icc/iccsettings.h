/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : central place for ICC settings
 *
 * Copyright (C) 2005-2006 by F.J. Cruz <fj dot cruz at supercable dot es>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

class IccProfile;

class DIGIKAM_EXPORT IccSettings : public QObject
{
    Q_OBJECT

public:

    /** Global container for ICC settings. All accessor methods are thread-safe.
     */
    static IccSettings* instance();

    /// Returns the current ICC settings.
    ICCSettingsContainer settings();

    /// Returns if color management is enabled.
    bool isEnabled() const;

    /// Returns if color management for previews is enabled.
    bool useManagedPreviews() const;

    /**
     * Returns the monitor profile (for color managed view).
     * If there are multiple screens, a system-wide settings specifies the monitor profile,
     * and the widget parameter is passed, the returned profile is for the widget's screen.
     * If no settings is specified, the default sRGB profile is returned.
     */
    IccProfile monitorProfile(QWidget* const widget = 0);

    /**
     * Returns if the monitor profile (as returned by monitorProfile())
     * is set system-wide, so that the monitorProfile field of the current settings
     * need not be set and will not be used by monitorProfile().
     */
    bool monitorProfileFromSystem() const;

    /**
     * Sets the current ICC settings and writes them to config.
     */
    void setSettings(const ICCSettingsContainer& settings);

    /// Set single parts of the settings
    void setUseManagedView(bool useManagedView);
    void setUseManagedPreviews(bool useManagedPreviews);
    void setIccPath(const QString& path);

    QList<IccProfile> allProfiles();

    /// Get available profiles suitable as workspace profile
    QList<IccProfile> workspaceProfiles();

    /// Get available profiles suitable as monitor/display profile
    QList<IccProfile> displayProfiles();

    /// Get available profiles suitable as input profile
    QList<IccProfile> inputProfiles();

    /// Get available profiles suitable as proof/output profiles
    QList<IccProfile> outputProfiles();

    /// Returns a list of profiles with the given description()
    QList<IccProfile> profilesForDescription(const QString& description);

    /**
     * IccProfile caches some of its properties (description, type)
     * when it was read once. Subsequently, to read these values no
     * opening is needed. This ensures that all profiles have these
     * values read. May imply scanning and opening all profiles.
     */
    void loadAllProfilesProperties();

Q_SIGNALS:

    void settingsChanged();
    void settingsChanged(const ICCSettingsContainer& current, const ICCSettingsContainer& previous);

private:

    IccSettings();
    ~IccSettings();

    void readFromConfig();

private:

    class Private;
    Private* const d;

    friend class Private;
    friend class IccSettingsCreator;
};

}  // namespace Digikam

#endif   // ICCSETTINGS_H
