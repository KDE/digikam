/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-20
 * Description : settings container for versioning
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef VERSIONMANAGERSETTINGS_H
#define VERSIONMANAGERSETTINGS_H

// Qt includes

#include <QFlags>
#include <QString>

// Local includes

#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT VersionManagerSettings
{
public:

    enum IntermediateSavepoint
    {
        NoIntermediates     = 0,
        AfterEachSession    = 1 << 0,
        AfterRawConversion  = 1 << 1,
        WhenNotReproducible = 1 << 2
    };
    Q_DECLARE_FLAGS(IntermediateBehavior, IntermediateSavepoint)

    enum ShowInViewFlag
    {
        OnlyShowCurrent     = 0,
        ShowOriginal        = 1 << 0,
        ShowIntermediates   = 1 << 1
    };
    Q_DECLARE_FLAGS(ShowInViewFlags, ShowInViewFlag)

    enum EditorClosingMode
    {
        AlwaysAsk,
        AutoSave
    };

public:

    VersionManagerSettings();

    void readFromConfig(KConfigGroup& group);
    void writeToConfig(KConfigGroup& group) const;

public:

    bool                 enabled;

    IntermediateBehavior saveIntermediateVersions;
    ShowInViewFlags      showInViewFlags;
    EditorClosingMode    editorClosingMode;

    /// Image format string as defined for database, in upper case
    QString              format;
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::VersionManagerSettings::IntermediateBehavior)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::VersionManagerSettings::ShowInViewFlags)

#endif // VERSIONMANAGERSETTINGS_H
