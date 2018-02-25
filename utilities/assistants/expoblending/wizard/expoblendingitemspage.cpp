/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Benjamin Girault, <benjamin dot girault at gmail dot com>
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

#include "expoblendingitemspage.h"

// Qt includes

#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include <QTimer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dimageslist.h"
#include "expoblendingmanager.h"
#include "expoblendingthread.h"
#include "dlayoutbox.h"

namespace Digikam
{

class ItemsPage::Private
{
public:

    explicit Private()
      : list(0),
        mngr(0)
    {
    }

    DImagesList*         list;
    ExpoBlendingManager* mngr;
};

ItemsPage::ItemsPage(ExpoBlendingManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, i18nc("@title:window", "<b>Set Bracketed Images</b>")),
      d(new Private)
{
    d->mngr = mngr;

    DVBox* const vbox    = new DVBox(this);
    QLabel* const label1 = new QLabel(vbox);
    label1->setWordWrap(true);
    label1->setText(i18n("<qt>"
                         "<p>Set here the list of your bracketed images to fuse. Please follow these conditions:</p>"
                         "<ul><li>At least 2 images from the same subject must be added to the stack.</li>"
                         "<li>Do not mix images with different color depth.</li>"
                         "<li>All images must have the same dimensions.</li></ul>"
                         "</qt>"));

    d->list = new DImagesList(vbox);
    d->list->setObjectName(QLatin1String("ExpoBlending ImagesList"));
    d->list->listView()->setColumn(DImagesListView::User1, i18nc("@title:column", "Exposure (EV)"), true);
    d->list->slotAddImages(d->mngr->itemsList());

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-stack.png")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

    connect(d->mngr->thread(), SIGNAL(starting(Digikam::ExpoBlendingActionData)),
            this, SLOT(slotExpoBlendingAction(Digikam::ExpoBlendingActionData)));

    connect(d->mngr->thread(), SIGNAL(finished(Digikam::ExpoBlendingActionData)),
            this, SLOT(slotExpoBlendingAction(Digikam::ExpoBlendingActionData)));

    connect(d->list, SIGNAL(signalAddItems(const QList<QUrl>&)),
            this, SLOT(slotAddItems(const QList<QUrl>&)));

    connect(d->list, SIGNAL(signalImageListChanged()),
            this, SLOT(slotImageListChanged()));

    QTimer::singleShot(0, this, SLOT(slotSetupList()));
}

ItemsPage::~ItemsPage()
{
    delete d;
}

void ItemsPage::slotSetupList()
{
    slotAddItems(d->mngr->itemsList());
}

void ItemsPage::slotAddItems(const QList<QUrl>& urls)
{
    if (!urls.empty())
    {
        d->mngr->thread()->identifyFiles(urls);

        if (!d->mngr->thread()->isRunning())
            d->mngr->thread()->start();
    }

    slotImageListChanged();
}

QList<QUrl> ItemsPage::itemUrls() const
{
    return d->list->imageUrls();
}

void ItemsPage::setIdentity(const QUrl& url, const QString& identity)
{
    DImagesListViewItem* item = d->list->listView()->findItem(url);

    if (item)
    {
        item->setText(DImagesListView::User1, identity);
    }
}

void ItemsPage::slotImageListChanged()
{
    emit signalItemsPageIsValid(d->list->imageUrls().count() > 1);
}

void ItemsPage::slotExpoBlendingAction(const Digikam::ExpoBlendingActionData& ad)
{
    QString text;

    if (!ad.starting)           // Something is complete...
    {
        switch (ad.action)
        {
            case(EXPOBLENDING_IDENTIFY):
            {
                setIdentity(ad.inUrls[0], ad.message);
                break;
            }
            default:
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown action";
                break;
            }
        }
    }
}

} // namespace Digikam
