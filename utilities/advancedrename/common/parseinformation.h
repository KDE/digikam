/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-12
 * Description : parse information class
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef PARSEINFORMATION_H
#define PARSEINFORMATION_H

// Qt includes

#include <QDateTime>
#include <QFileInfo>
#include <QString>

// Local includes

#include "imageinfo.h"

namespace Digikam
{

class ParseInformation
{
public:

    ParseInformation() : index(1) {};
    ParseInformation(const ImageInfo& info) :
        filePath(info.filePath()),
        cameraName(info.photoInfoContainer().make + ' ' + info.photoInfoContainer().model),
        dateTime(info.dateTime()),
        index(1)
    {};

    bool isValid()
    {
        QFileInfo fi(filePath);
        return fi.isReadable();
    };

public:

    QString   filePath;
    QString   cameraName;
    QDateTime dateTime;
    int       index;
};

} // namespace Digikam

#endif /* PARSEINFORMATION_H */
