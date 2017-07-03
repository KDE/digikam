/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
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

#include "vidslideintropage.h"

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
#include "vidslidewizard.h"
#include "vidslidesettings.h"

namespace Digikam
{

class VidSlideIntroPage::Private
{
public:

    Private(QWizard* const dialog)
      : imageGetOption(0),
        hbox(0),
        wizard(0),
        iface(0)
    {
        wizard = dynamic_cast<VidSlideWizard*>(dialog);

        if (wizard)
        {
            iface = wizard->iface();
        }
    }

    QComboBox*       imageGetOption;
    DHBox*           hbox;
    VidSlideWizard*  wizard;
    DInfoInterface*  iface;
};

VidSlideIntroPage::VidSlideIntroPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    DVBox* const vbox  = new DVBox(this);
    QLabel* const desc = new QLabel(vbox);

    desc->setWordWrap(true);
    desc->setOpenExternalLinks(true);
    desc->setText(i18n("<qt>"
                        "<p><h1><b>Welcome to Video Slideshow tool</b></h1></p>"
                        "<p>This assistant will guide you to export</p>"
                        "<p>your images as a video stream.</p>"
                        "<p>You can generate quickly a "
                        "<a href='https://en.wikipedia.org/wiki/Time-lapse_photography'>Time-lapse</a> "
                        "movie from images</p>"
                        "<p>captured with a tripod mounted camera controlled with an "
                        "<a href='https://en.wikipedia.org/wiki/Intervalometer#Photography'>intervalometer</a>.</p>"
                        "<p></p><p>You can also create a video presentation with transition</p>"
                        "<p>effects and audio tracks to show on a TV screen.</p>"
                        "</qt>"));

    // ComboBox for image selection method

    d->hbox                     = new DHBox(vbox);
    QLabel* const getImageLabel = new QLabel(i18n("&Choose image selection method:"), d->hbox);
    d->imageGetOption           = new QComboBox(d->hbox);
    d->imageGetOption->insertItem(VidSlideSettings::ALBUMS, i18n("Albums"));
    d->imageGetOption->insertItem(VidSlideSettings::IMAGES, i18n("Images"));
    getImageLabel->setBuddy(d->imageGetOption);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("view-presentation")));
}

VidSlideIntroPage::~VidSlideIntroPage()
{
    delete d;
}

void VidSlideIntroPage::initializePage()
{
    bool albumSupport = (d->iface && d->iface->supportAlbums());

    if (!albumSupport)
    {
        d->imageGetOption->setCurrentIndex(VidSlideSettings::IMAGES);
        d->hbox->setEnabled(false);
    }
    else
    {
        d->imageGetOption->setCurrentIndex(d->wizard->settings()->selMode);
    }
}

bool VidSlideIntroPage::validatePage()
{
    d->wizard->settings()->selMode = (VidSlideSettings::Selection)d->imageGetOption->currentIndex();

    return true;
}

} // namespace Digikam
