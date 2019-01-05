/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate jAlbum image galleries
 *
 * Copyright (C) 2013-2019 by Andrew Goodbody <ajg zero two at elfringham dot co dot uk>
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

#include "jalbumintropage.h"

// Qt includes

#include <QLabel>
#include <QPixmap>
#include <QComboBox>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "jalbumwizard.h"
#include "jalbuminfo.h"
#include "dlayoutbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN JAlbumIntroPage::Private
{
public:

    explicit Private(QWizard* const dialog)
      : imageGetOption(0),
        hbox(0),
        wizard(0),
        info(0),
        iface(0)
    {
        wizard = dynamic_cast<JAlbumWizard*>(dialog);

        if (wizard)
        {
            info  = wizard->jalbumInfo();
            iface = info->m_iface;
        }
    }

    QComboBox*       imageGetOption;
    DHBox*           hbox;
    JAlbumWizard*    wizard;
    JAlbumInfo*      info;
    DInfoInterface*  iface;
};

JAlbumIntroPage::JAlbumIntroPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    DVBox* const vbox  = new DVBox(this);
    QLabel* const desc = new QLabel(vbox);

    desc->setWordWrap(true);
    desc->setOpenExternalLinks(true);
    desc->setText(i18n("<qt>"
                        "<p><h1><b>Welcome to jAlbum album tool</b></h1></p>"
                        "<p>This assistant will guide you to export quickly</p><p></p>"
                        "<p>your images as a jAlbum project.</p>"
                        "</qt>"));

    // ComboBox for image selection method

    d->hbox                     = new DHBox(vbox);
    QLabel* const getImageLabel = new QLabel(i18n("&Choose image selection method:"), d->hbox);
    d->imageGetOption           = new QComboBox(d->hbox);
    d->imageGetOption->insertItem(JAlbumInfo::ALBUMS, i18n("Albums"));
    d->imageGetOption->insertItem(JAlbumInfo::IMAGES, i18n("Images"));
    getImageLabel->setBuddy(d->imageGetOption);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("text-html")));
}

JAlbumIntroPage::~JAlbumIntroPage()
{
    delete d;
}

void JAlbumIntroPage::initializePage()
{
    bool albumSupport = (d->iface && d->iface->supportAlbums());

    if (!albumSupport)
    {
        d->imageGetOption->setCurrentIndex(JAlbumInfo::IMAGES);
        d->hbox->setEnabled(false);
    }
    else
    {
        d->imageGetOption->setCurrentIndex(d->info->m_getOption);
    }
}

bool JAlbumIntroPage::validatePage()
{
    d->info->m_getOption = (JAlbumInfo::ImageGetOption)d->imageGetOption->currentIndex();

    return true;
}

} // namespace Digikam
