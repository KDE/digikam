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

#include "mailintropage.h"

// Qt includes

#include <QLabel>
#include <QPixmap>
#include <QComboBox>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "mailwizard.h"
#include "mailsettings.h"

namespace Digikam
{

class MailIntroPage::Private
{
public:

    Private(QWizard* const dialog)
      : imageGetOption(0),
        hbox(0),
        wizard(0),
        iface(0)
    {
        wizard = dynamic_cast<MailWizard*>(dialog);

        if (wizard)
        {
            iface = wizard->iface();
        }
    }

    QComboBox*      imageGetOption;
    DHBox*          hbox;
    MailWizard*     wizard;
    DInfoInterface* iface;
};

MailIntroPage::MailIntroPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    DVBox* const vbox  = new DVBox(this);
    QLabel* const desc = new QLabel(vbox);

    desc->setWordWrap(true);
    desc->setOpenExternalLinks(true);
    desc->setText(i18n("<qt>"
                       "<p><h1><b>Welcome to Email Tool</b></h1></p>"
                       "<p>This assistant will guide you to send "
                       "your items with a mail client application.</p>"
                       "<p>Before to export contents, you will be able to adjust attachements "
                       "properties accordingly with your mail service capabilities.</p>"
                       "</qt>"));

    // ComboBox for image selection method

    d->hbox                     = new DHBox(vbox);
    QLabel* const getImageLabel = new QLabel(i18n("&Choose image selection method:"), d->hbox);
    d->imageGetOption           = new QComboBox(d->hbox);
    d->imageGetOption->insertItem(MailSettings::ALBUMS, i18n("Albums"));
    d->imageGetOption->insertItem(MailSettings::IMAGES, i18n("Images"));
    getImageLabel->setBuddy(d->imageGetOption);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("view-presentation")));
}

MailIntroPage::~MailIntroPage()
{
    delete d;
}

void MailIntroPage::initializePage()
{
    bool albumSupport = (d->iface && d->iface->supportAlbums());

    if (!albumSupport)
    {
        d->imageGetOption->setCurrentIndex(MailSettings::IMAGES);
        d->hbox->setEnabled(false);
    }
    else
    {
        d->imageGetOption->setCurrentIndex(d->wizard->settings()->selMode);
    }
}

bool MailIntroPage::validatePage()
{
    d->wizard->settings()->selMode = (MailSettings::Selection)d->imageGetOption->currentIndex();

    return true;
}

} // namespace Digikam
