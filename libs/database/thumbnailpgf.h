/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : PGF thumbnail interface.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef THUMBNAILPGF_H
#define THUMBNAILPGF_H

// Qt includes

#include <QtCore/QString>
#include <QtGui/QImage>
#include <QtCore/QByteArray>

// KDE includes

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ThumbnailPGF
{
public:

    /** PGF image data to QImage */
    static bool readPGFImageData(const QByteArray& data, QImage& img);
    /** QImage to PGF image data */
    static bool writePGFImageData(const QImage& img, QByteArray& data);
};

}  // namespace Digikam

#endif /* THUMBNAILPGF_H */
