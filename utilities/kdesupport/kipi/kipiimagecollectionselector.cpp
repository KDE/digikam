/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : a widget to select image collections using
 *               digiKam album folder views
 *
 * Copyright (C) 2008-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "kipiimagecollectionselector.h"

// Qt includes

#include <QHeaderView>
#include <QTreeWidgetItemIterator>
#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QTabWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "kipiimagecollection.h"
#include "kipiinterface.h"
#include "applicationsettings.h"
#include "albumselecttabs.h"
#include "abstractalbummodel.h"
#include "albumlabelstreeview.h"

namespace Digikam
{

class KipiImageCollectionSelector::Private
{
public:

    Private() :
        tab(0),
        iface(0)
    {
    }

    void fillCollectionsFromCheckedModel(QList<KIPI::ImageCollection>& collectionList,
                                         AbstractCheckableAlbumModel* const model,
                                         const QString& ext)
    {
        foreach(Album* const album, model->checkedAlbums())
        {
            if (!album)
            {
                continue;
            }

            KipiImageCollection* const col = new KipiImageCollection(KipiImageCollection::AllItems,
                                                                     album, ext);
            collectionList.append(col);
        }
    }

    void fillCollectionsFromCheckedLabels(QList<KIPI::ImageCollection>& collectionList,
                                          AlbumLabelsSearchHandler* handler,
                                          const QString& ext)
    {
        Album* const album = handler->albumForSelectedItems();

        if (!album)
        {
            return;
        }

        KipiImageCollection* const col = new KipiImageCollection(KipiImageCollection::AllItems, album,
                                                                 ext, handler->imagesUrls());
        collectionList.append(col);
    }

public:

    AlbumSelectTabs* tab;
    KipiInterface*   iface;
};

KipiImageCollectionSelector::KipiImageCollectionSelector(KipiInterface* const iface, QWidget* const parent)
    : KIPI::ImageCollectionSelector(parent),
      d(new Private)
{
    d->iface                = iface;
    d->tab                  = new AlbumSelectTabs(this);
    QHBoxLayout* const hlay = new QHBoxLayout(this);
    hlay->addWidget(d->tab);
    hlay->setContentsMargins(QMargins());
    hlay->setSpacing(0);
}

KipiImageCollectionSelector::~KipiImageCollectionSelector()
{
    delete d;
}

QList<KIPI::ImageCollection> KipiImageCollectionSelector::selectedImageCollections() const
{
    QString ext = ApplicationSettings::instance()->getAllFileFilter();
    QList<KIPI::ImageCollection> list;

    foreach(AbstractCheckableAlbumModel* const model, d->tab->albumModels())
    {
        d->fillCollectionsFromCheckedModel(list, model, ext);
    }

    d->fillCollectionsFromCheckedLabels(list, d->tab->albumLabelsHandler(), ext);

    qCDebug(DIGIKAM_GENERAL_LOG) << list.count() << " collection items selected";

    return list;
}

void KipiImageCollectionSelector::enableVirtualCollections(bool flag)
{
    d->tab->enableVirtualAlbums(flag);
}

}  // namespace Digikam
