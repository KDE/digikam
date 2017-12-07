/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : class to get/set image collection
 *               information/properties using digiKam album database.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2005 by Ralf Holzer <ralf at well dot com>
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014      by Mohamed Anwer <m dot anwer at gmx dot com>
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

#ifndef DIGIKAM_KIPIIMAGECOLLECTION_H
#define DIGIKAM_KIPIIMAGECOLLECTION_H

// Qt includes

#include <QString>
#include <QDate>
#include <QUrl>

// Libkipi includes

#include <KIPI/ImageCollection>
#include <KIPI/ImageCollectionShared>

// Local includes

#include "albummanager.h"

using namespace KIPI;

namespace Digikam
{

class KipiImageCollection : public ImageCollectionShared
{

public:

    enum Type
    {
        AllItems,
        SelectedItems
    };

public:

    KipiImageCollection(Type type, Album* const album, const QString& filter, QList<QUrl> imagesUrlList = QList<QUrl>());
    ~KipiImageCollection();

    virtual QString     name();
    virtual QString     comment();
    virtual QString     category();
    virtual QDate       date();
    virtual QList<QUrl> images();
    virtual QUrl        url();
    virtual QUrl        uploadUrl();
    virtual QUrl        uploadRoot();
    virtual QString     uploadRootName();
    virtual bool        isDirectory();

    virtual bool operator==(ImageCollectionShared&);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif  // DIGIKAM_KIPIIMAGECOLLECTION_H
