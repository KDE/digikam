/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net> 
 * Date   : 2006-19-06
 * Description : digiKam KIO slave to extract image preview.
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

#ifndef DIGIKAM_PREVIEW_H
#define DIGIKAM_PREVIEW_H

// KDE includes.

#include <kio/slavebase.h>

class QString;
class QImage;

class KURL;

class kio_digikampreviewProtocol : public KIO::SlaveBase
{

public:

    kio_digikampreviewProtocol(int argc, char** argv);
    virtual ~kio_digikampreviewProtocol();
    virtual void get(const KURL& url);

private:

    void exifRotate(const QString& filePath, QImage& thumb);
    bool loadImagePreview(QImage& image, const QString& path);

};

#endif  // DIGIKAM_PREVIEW_H
