/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : class to get/set image information/properties
 *               in a digiKam album.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well dot com>
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_KIPIIMAGEINFO_H
#define DIGIKAM_KIPIIMAGEINFO_H

// Qt includes

#include <QVariant>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QUrl>

// LibKipi includes

#include <KIPI/ImageInfoShared>

namespace KIPI
{
    class Interface;
}

using namespace KIPI;

namespace Digikam
{

class KipiImageInfo : public ImageInfoShared
{
public:

    KipiImageInfo(Interface* const interface, const QUrl& url);
    ~KipiImageInfo();

    void cloneData(ImageInfoShared* const other);

    QMap<QString, QVariant> attributes();
    void                    addAttributes(const QMap<QString, QVariant>& res);
    void                    delAttributes(const QStringList& res);
    void                    clearAttributes();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif  // DIGIKAM_KIPIIMAGEINFO_H
