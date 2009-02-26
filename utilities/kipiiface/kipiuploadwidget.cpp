/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-26-02
 * Description : a widget to select an image collection
 *               to upload new items using digiKam album folder views
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Luka Renko <lure at kubuntu dot org>
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

#include "kipiuploadwidget.h"
#include "kipiuploadwidget.moc"

// Qt includes.

#include <QVBoxLayout>

// KDE includes.

#include <kdialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kmessagebox.h>
#include <kinputdialog.h>

// Local includes.

#include "album.h"
#include "albumselectwidget.h"
#include "treefolderitem.h"
#include "kipiinterface.h"
#include "kipiimagecollection.h"

namespace Digikam
{

class KipiUploadWidgetPriv
{
public:

    KipiUploadWidgetPriv()
    {
        albumSel = 0;
        iface    = 0;
    }

    AlbumSelectWidget *albumSel;

    KipiInterface     *iface;
};

KipiUploadWidget::KipiUploadWidget(KipiInterface* iface, QWidget *parent)
                : KIPI::UploadWidget(parent),
                  d(new KipiUploadWidgetPriv)
{
    d->iface          = iface;
    QVBoxLayout *vlay = new QVBoxLayout(this);
    d->albumSel       = new AlbumSelectWidget(this);
    vlay->addWidget(d->albumSel);
    vlay->setMargin(0);
    vlay->setSpacing(KDialog::spacingHint());
}

KipiUploadWidget::~KipiUploadWidget()
{
    delete d;
}

KIPI::ImageCollection KipiUploadWidget::selectedImageCollection() const
{
    KIPI::ImageCollection collection;
    if (d->iface)
    {
        QString ext = d->iface->fileExtensions();

        TreeAlbumItem* item = dynamic_cast<TreeAlbumItem*>(d->albumSel->albumView()->currentItem());
        if (item)
            collection = new KipiImageCollection(KipiImageCollection::AllItems, item->album(), ext);
    }
    return collection;
}

}  // namespace Digikam
