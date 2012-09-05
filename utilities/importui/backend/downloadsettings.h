/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-07
 * Description : Camera item download settings container.
 *
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DOWNLOADSETTINGS_H
#define DOWNLOADSETTINGS_H

// Qt includes

#include <QString>
#include <QDateTime>

// local includes

#include "globals.h"

namespace Digikam
{

class DownloadSettings
{

public:

    DownloadSettings()
    {
        autoRotate  = true;
        fixDateTime = false;
        convertJpeg = false;
        rating      = NoRating;
        pickLabel   = NoPickLabel;
        colorLabel  = NoColorLabel;
    };

    ~DownloadSettings()
    {
    };

public:

    // -- Settings from AdvancedSettings widget ---------------
    bool       autoRotate;
    bool       fixDateTime;
    bool       convertJpeg;

    QDateTime  newDateTime;

    // New format to convert Jpeg files.
    QString    losslessFormat;

    // Metadata template title.
    QString    templateTitle;

    // -- File path to download ------------------------------

    QString    folder;
    QString    file;
    QString    dest;

    // -- Settings from ScriptingSettings widget ---------------
    QString    script;

    // -- Pre-rating of each camera file.
    int        rating;

    // -- Pre-pickLabel of each camera file.
    int        pickLabel;

    // -- Pre-colorLabel of each camera file.
    int        colorLabel;

    // -- Pre-tags of each camera file.
    QList<int> tagIds;
};

typedef QList<DownloadSettings> DownloadSettingsList;

}  // namespace Digikam

#endif  // DownloadSettings_H
