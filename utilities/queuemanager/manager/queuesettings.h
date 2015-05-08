/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-21
 * Description : Queue common settings container.
 *
 * Copyright (C) 2009-2013 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef QUEUE_SETTINGS_H
#define QUEUE_SETTINGS_H

// KDE includes

#include <kurl.h>

// LibKDcraw includes

#include <libkdcraw/rawdecodingsettings.h>

// Local includes

#include "iofilesettings.h"

using namespace KDcrawIface;

namespace Digikam
{

/** This container host all common settings used by a queue, not including assigned batch tools
 */
class QueueSettings
{
public:

    enum ConflictRule
    {
        OVERWRITE = 0,
        DIFFNAME
    };

    enum RenamingRule
    {
        USEORIGINAL = 0,
        CUSTOMIZE
    };


    enum RawLoadingRule
    {
        USEEMBEDEDJPEG = 0,
        DEMOSAICING
    };

public:

    QueueSettings()
    {
        useMultiCoreCPU    = false;
        exifSetOrientation = true;
        useOrgAlbum        = true;
        conflictRule       = DIFFNAME;
        renamingRule       = USEORIGINAL;
        rawLoadingRule     = DEMOSAICING;
    };

public:

    bool                useMultiCoreCPU;
    
    /// Setting managed through Metadata control panel.
    bool                exifSetOrientation;

    /// If true, original file dir will be used to process queue items.
    bool                useOrgAlbum;

    QString             renamingParser;

    KUrl                workingUrl;

    ConflictRule        conflictRule;
    RenamingRule        renamingRule;
    RawLoadingRule      rawLoadingRule;

    RawDecodingSettings rawDecodingSettings;

    IOFileSettings      ioFileSettings;
};

}  // namespace Digikam

#endif /* QUEUE_SETTINGS_H */
