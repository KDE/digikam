/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-08
 * Description : Low level copy files and directories
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DFILE_OPERATIONS_H
#define DFILE_OPERATIONS_H

// Qt includes

#include <QString>
#include <QStringList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DFileOperations
{
public:

    static bool copyFolderRecursively(const QString& srcPath, const QString& dstPath);
    static bool copyFiles(const QStringList& srcPaths, const QString& dstPath);
};

} // namespace Digikam

#endif // DFILE_OPERATIONS_H
