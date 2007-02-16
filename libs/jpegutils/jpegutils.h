/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at gmail dot com>
 * Date   : 2004-09-29
 * Description : perform lossless rotation/flip to JPEG file
 * 
 * Copyright 2004-2005 by Renchi Raju
 * Copyright      2006 by Gilles Caulier
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

#ifndef JPEGUTILS_H
#define JPEGUTILS_H

// Qt includes.

#include <qstring.h>
#include <qimage.h>

namespace Digikam
{

bool loadJPEGScaled(QImage& image, const QString& path, int maximumSize);
bool exifRotate(const QString& file, const QString& documentName);
bool jpegConvert(const QString& src, const QString& dest, const QString& documentName, 
                 const QString& format=QString("PNG"));
bool isJpegImage(const QString& file);

}

#endif /* JPEGUTILS_H */
