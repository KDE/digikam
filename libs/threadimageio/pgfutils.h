/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : PGF utils.
 *
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PGFUTIL_H
#define PGFUTIL_H

// Qt includes

#include <QtCore/QString>
#include <QtGui/QImage>
#include <QtCore/QByteArray>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/** PGF image data to QImage.
    NOTE: Only use this method to manage PGF thumbnails stored in database (256x256 pixels image)
 */
bool readPGFImageData(const QByteArray& data, QImage& img);

/** QImage to PGF image data. 'quality' argument set compression ratio:
    0 => lossless compression, as PNG.
    1 => Not loss less compression, wavelets based...
    2 =>
    3 =>
    4 => Same compression ratio near than JPEG quality=85. image quality is valid for thumbnails.
    >= 5 => provide artifacts due to down-sampling. Do not use it...
    NOTE: Only use this method to manage PGF thumbnails stored in database (256x256 pixels image)
 */
bool writePGFImageData(const QImage& img, QByteArray& data, int quality);

/** Load a reduced version of PGF file
 */
bool loadPGFScaled(QImage& img, const QString& path, int maximumSize);

/** Return a libpgf version string
 */
QString libPGFVersion();

/** Return true if libpgf is compiled with OpenMP support
 */
bool libPGFUseOpenMP();

}  // namespace Digikam

#endif /* PGFUTIL_H */
