/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : a widget to select image collections using
 *               digiKam album folder views
 *
 * Copyright (C) 2008-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef KIPIIMAGECOLLECTIONSELECTOR_H
#define KIPIIMAGECOLLECTIONSELECTOR_H

// Qt includes

#include <QList>
#include <QString>

// LibKIPI includes

#include <libkipi/imagecollection.h>
#include <libkipi/imagecollectionselector.h>

// Local includes

#include "searchtextbar.h"
#include "albummanager.h"

namespace Digikam
{

class KipiInterface;

class KipiImageCollectionSelector : public KIPI::ImageCollectionSelector
{
    Q_OBJECT

public:

    explicit KipiImageCollectionSelector(KipiInterface* const iface, QWidget* const parent = 0);
    ~KipiImageCollectionSelector();

    QList<KIPI::ImageCollection> selectedImageCollections() const;
    void enableVirtualCollections(bool flag=true);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif  // KIPIIMAGECOLLECTIONSELECTOR_H
