/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items by email.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "mailimagespage.h"

// Qt includes

#include <QIcon>
#include <QPixmap>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "mailwizard.h"
#include "dimageslist.h"
#include "dlayoutbox.h"

namespace Digikam
{

class MailImagesPage::Private
{
public:

    Private(QWizard* const dialog)
      : imageList(0),
        wizard(0),
        iface(0)
    {
        wizard = dynamic_cast<MailWizard*>(dialog);

        if (wizard)
        {
            iface = wizard->iface();
        }
    }

    DImagesList*     imageList;
    MailWizard*      wizard;
    DInfoInterface*  iface;
};

MailImagesPage::MailImagesPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    DVBox* const vbox  = new DVBox(this);
    QLabel* const desc = new QLabel(vbox);
    desc->setText(i18n("<p>This view list all items to export by mail.</p>"));

    d->imageList       = new DImagesList(vbox);
    d->imageList->setControlButtonsPlacement(DImagesList::ControlButtonsBelow);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("image-stack")));

    connect(d->imageList, SIGNAL(signalImageListChanged()),
            this, SIGNAL(completeChanged()));
}

MailImagesPage::~MailImagesPage()
{
    delete d;
}

void MailImagesPage::setItemsList(const QList<QUrl>& urls)
{
    d->imageList->slotAddImages(urls);
}

void MailImagesPage::initializePage()
{
    d->imageList->setIface(d->iface);
    d->imageList->listView()->clear();

    if (d->wizard->settings()->selMode == MailSettings::IMAGES)
    {
        d->imageList->loadImagesFromCurrentSelection();
    }
    else
    {
        setItemsList(d->wizard->settings()->inputImages);
    }
}

bool MailImagesPage::validatePage()
{
    if (d->imageList->imageUrls().empty())
        return false;

    d->wizard->settings()->inputImages = d->imageList->imageUrls();

    return true;
}

bool MailImagesPage::isComplete() const
{
    return (!d->imageList->imageUrls().empty());
}

} // namespace Digikam
