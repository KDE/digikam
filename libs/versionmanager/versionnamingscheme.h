/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-20
 * Description : Interface for version file naming
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

#ifndef VERSIONNAMINGSCHEME_H
#define VERSIONNAMINGSCHEME_H

// Qt includes

#include <QString>
#include <QVariant>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT VersionNamingScheme
{
public:

    /**
     * Creates and analyzes file names of versioned files.
     */
    virtual ~VersionNamingScheme()
    {
    }

    /**
     * Analyzes the given file name.
     * Returns the basename in the sense of stripping the file name of all added
     * version information: A scheme that appends a number, like "MyFile-1.jpg", shall return "MyFile".
     * Path is the directory, filename the file name, so path + filename is the file path.
     * If counter is given, and the given file name has a version number, write it to counter.
     * If intermediateCounter is given, and the given file name has an intermediate counter number,
     * write it to counter. If not available, do not touch the given counters.
     * See initialCounter() for the valid counter formats.
     */
    virtual QString baseName(const QString& path, const QString& filename,
                             QVariant* counter = 0, QVariant* intermediateCounter = 0) = 0;

    /**
     * Creates a version file name for a file in given directory, as previously returned by directory(),
     * given baseName, as previously returned by baseName, and version counter.
     * Do not append a file suffix.
     * You do not need to check if the file exists.
     */
    virtual QString versionFileName(const QString& path, const QString& baseName,
                                    const QVariant& counter) = 0;

    /**
     * Creates a version file name for an intermediate file in given directory,
     * as previously returned by directory(), given baseName, as previously
     * returned by baseName, version and intermediate number counter.
     * Do not append a file suffix.
     * You do not need to check if the file exists.
     */
    virtual QString intermediateFileName(const QString& path, const QString& filename,
                                         const QVariant& version, const QVariant& counter) = 0;

    /**
     * For a loaded file in directory path and with file name filename,
     * returns the directory in which a new version (a new intermediate version, resp.) shall be stored.
     */
    virtual QString directory(const QString& path, const QString& filename) = 0;
    virtual QString intermediateDirectory(const QString& currentPath, const QString& fileName) = 0;

    /**
     * Returns an initial counter value for version and intermediate number counters.
     * There are two places where you shall generate counters
     * You will receive the given QVariant in incrementedCounter(),
     * versionFileName() and baseName(), and you shall read a counter value from a
     * generated file name in baseName().
     */
    virtual QVariant initialCounter() = 0;

    /**
     * Returns the given counter "incremented", that is, changed in a steady, repeatable fashion.
     * You shall never return the given counter.
     */
    virtual QVariant incrementedCounter(const QVariant& counter) = 0;
};

} // namespace Digikam

#endif // VERSIONNAMINGSCHEME_H
