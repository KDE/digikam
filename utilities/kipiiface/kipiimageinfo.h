/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : class to get/set image information/properties
 *               in a digiKam album.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well.com>
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef KIPIIMAGEINFO_H
#define KIPIIMAGEINFO_H

// Qt includes

#include <QVariant>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QMap>

// KDE includes

#include <kurl.h>

// LibKIPI includes

#include <libkipi/interface.h>
#include <libkipi/imageinfo.h>
#include <libkipi/imageinfoshared.h>

// Local includes

#include "imageinfo.h"

namespace Digikam
{

class PAlbum;

class KipiImageInfo : public KIPI::ImageInfoShared
{
public:

    KipiImageInfo(KIPI::Interface* interface, const KUrl& url);
    ~KipiImageInfo();

    virtual QString title();
    virtual void setTitle(const QString&);

    virtual QString description();
    virtual void setDescription(const QString&);

    virtual void cloneData(ImageInfoShared* other);

    virtual QDateTime time(KIPI::TimeSpec spec);
    virtual void setTime(const QDateTime& time, KIPI::TimeSpec spec = KIPI::FromInfo );

    virtual QMap<QString, QVariant> attributes();
    virtual void addAttributes(const QMap<QString, QVariant>& res);
    virtual void delAttributes(const QStringList& res);
    virtual void clearAttributes();

    virtual int  angle();
    virtual void setAngle(int angle);

private:

    PAlbum* parentAlbum();

private:

    ImageInfo m_info;
};

}  // namespace Digikam

#endif  // KIPIIMAGEINFO_H
