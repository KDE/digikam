/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 * Acknowledge : based on the expoblending tool
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "panoitemspage.h"

// Qt includes

#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include <QTimer>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimageslist.h"
#include "panomanager.h"
#include "dlayoutbox.h"

namespace Digikam
{

class PanoItemsPage::Private
{
public:

    explicit Private()
      : list(0),
        mngr(0)
    {
    }

    DImagesList* list;

    PanoManager*     mngr;
};

PanoItemsPage::PanoItemsPage(PanoManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, i18nc("@title:window", "<b>Set Panorama Images</b>")),
      d(new Private)
{
    d->mngr              = mngr;
    DVBox* const vbox    = new DVBox(this);
    QLabel* const label1 = new QLabel(vbox);
    label1->setWordWrap(true);
    label1->setText(i18n("<qt>"
                         "<p>Set here the list of your images to blend into a panorama. "
                         "Please follow these conditions:</p>"
                         "<ul><li>Images are taken from the same point of view.</li>"
                         "<li>Images are taken with the same camera (and lens).</li>"
                         "<li>Do not mix images with different color depth.</li></ul>"
                         "<p>Note that, in the case of a 360° panorama, the first image "
                         "in the list will be the image that will be in the center of "
                         "the panorama.</p>"
                         "</qt>"));

    d->list = new DImagesList(vbox);
    d->list->setObjectName(QLatin1String("Panorama ImagesList"));
    d->list->slotAddImages(d->mngr->itemsList());

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-stack.png")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

    connect(d->list, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    QTimer::singleShot(0, this, SLOT(slotSetupList()));
}

PanoItemsPage::~PanoItemsPage()
{
    delete d;
}

void PanoItemsPage::slotSetupList()
{
    slotImageListChanged();
}

QList<QUrl> PanoItemsPage::itemUrls() const
{
    return d->list->imageUrls();
}

bool PanoItemsPage::validatePage()
{
    d->mngr->setItemsList(d->list->imageUrls());

    return true;
}

void PanoItemsPage::slotImageListChanged()
{
    setComplete(d->list->imageUrls().count() > 1);
    emit completeChanged();
}

} // namespace Digikam
