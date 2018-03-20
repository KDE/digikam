/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-18
 * Description : class for determining new file name in terms of version management
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef VERSIONMANAGER_H
#define VERSIONMANAGER_H

// Qt includes

#include <QFlags>
#include <QMap>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "dimagehistory.h"
#include "versionfileoperation.h"
#include "versionmanagersettings.h"
#include "versionnamingscheme.h"

namespace Digikam
{

class DIGIKAM_EXPORT VersionManager
{
public:

    enum FileNameType
    {
        CurrentVersionName,
        NewVersionName
    };

public:

    VersionManager();
    virtual ~VersionManager();

    void setSettings(const VersionManagerSettings& settings);
    VersionManagerSettings settings() const;

    void setNamingScheme(VersionNamingScheme* scheme);
    VersionNamingScheme* namingScheme() const;

    VersionFileOperation operation(FileNameType request, const VersionFileInfo& loadedFile,
                                   const DImageHistory& initialResolvedHistory,
                                   const DImageHistory& currentHistory);

    VersionFileOperation operationNewVersionInFormat(const VersionFileInfo& loadedFile,
                                   const QString& format,
                                   const DImageHistory& initialResolvedHistory,
                                   const DImageHistory& currentHistory);

    VersionFileOperation operationNewVersionAs(const VersionFileInfo& loadedFile,
                                               const VersionFileInfo& saveLocation,
                                               const DImageHistory& initialResolvedHistory,
                                               const DImageHistory& currentHistory);

    virtual QString toplevelDirectory(const QString& path);

    virtual QStringList workspaceFileFormats() const;

private:

    class VersionManagerPriv;
    VersionManagerPriv* const d;
};

} // namespace Digikam

#endif // VERSIONMANAGER_H
