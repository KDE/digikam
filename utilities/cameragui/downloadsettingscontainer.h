/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date  : 2006-21-07
 * Description : Camera item download settings container.
 * 
 * Copyright 2006 by Gilles Caulier
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

#ifndef DOWNLOADSETTINGSCONTAINER_H
#define DOWNLOADSETTINGSCONTAINER_H

// Qt includes.

#include <qstring.h>
#include <qdatetime.h>

namespace Digikam
{

class DownloadSettingsContainer
{

public:
    
    DownloadSettingsContainer()
    {
        autoRotate        = true;
        fixDateTime       = false;
        setPhotographerId = false;
        setCredits        = false;
        convertJpeg       = false;
    };
    
    ~DownloadSettingsContainer(){};

public:

    bool      autoRotate;
    bool      fixDateTime;
    bool      setPhotographerId;
    bool      setCredits;
    bool      convertJpeg;

    QDateTime newDateTime;

    // File path to download.
    QString   folder;
    QString   file;
    QString   dest;

    // New format to convert Jpeg files.
    QString   losslessFormat;

    // IPTC settings
    QString   author;
    QString   authorTitle;
    QString   credit;
    QString   source;
    QString   copyright;
};

}  // namespace Digikam

#endif  // DOWNLOADSETTINGSCONTAINER_H
