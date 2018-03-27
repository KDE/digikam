/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "vidslidewizard.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QMenu>
#include <QApplication>
#include <QComboBox>
#include <QListWidget>
#include <QTextBrowser>

// KDE includes

#include <klocalizedstring.h>
#include <kconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dwizardpage.h"
#include "digikam_debug.h"
#include "vidslideintropage.h"
#include "vidslidealbumspage.h"
#include "vidslideimagespage.h"
#include "vidslidevideopage.h"
#include "vidslideoutputpage.h"
#include "vidslidefinalpage.h"

namespace Digikam
{

class VidSlideWizard::Private
{
public:

    Private()
      : iface(0),
        introPage(0),
        albumsPage(0),
        imagesPage(0),
        videoPage(0),
        outputPage(0),
        finalPage(0),
        settings(0)
    {
    }

    DInfoInterface*         iface;
    VidSlideIntroPage*      introPage;
    VidSlideAlbumsPage*     albumsPage;
    VidSlideImagesPage*     imagesPage;
    VidSlideVideoPage*      videoPage;
    VidSlideOutputPage*     outputPage;
    VidSlideFinalPage*      finalPage;
    VidSlideSettings*       settings;
};

VidSlideWizard::VidSlideWizard(QWidget* const parent, DInfoInterface* const iface)
    : DWizardDlg(parent, QLatin1String("Video SlideShow Dialog")),
      d(new Private)
{
    setOption(QWizard::NoCancelButtonOnLastPage);
    setWindowTitle(i18n("Create a Video Slideshow"));

    d->iface             = iface;
    d->settings          = new VidSlideSettings;

    KConfig config;
    KConfigGroup group   = config.group("Video SlideShow Dialog Settings");
    d->settings->readSettings(group);

    d->introPage         = new VidSlideIntroPage(this,  i18n("Welcome to Video Slideshow Tool"));
    d->albumsPage        = new VidSlideAlbumsPage(this, i18n("Albums Selection"));
    d->imagesPage        = new VidSlideImagesPage(this, i18n("Images List"));
    d->videoPage         = new VidSlideVideoPage(this,  i18n("Video Settings"));
    d->outputPage        = new VidSlideOutputPage(this, i18n("Output Settings"));
    d->finalPage         = new VidSlideFinalPage(this,  i18n("Generating Video Slideshow"));

    connect(this, SIGNAL(currentIdChanged(int)),
            this, SLOT(slotCurrentIdChanged(int)));
}

VidSlideWizard::~VidSlideWizard()
{
    KConfig config;
    KConfigGroup group = config.group("Video SlideShow Dialog Settings");
    d->settings->writeSettings(group);

    delete d;
}

void VidSlideWizard::setItemsList(const QList<QUrl>& urls)
{
    d->imagesPage->setItemsList(urls);
}

DInfoInterface* VidSlideWizard::iface() const
{
    return d->iface;
}

VidSlideSettings* VidSlideWizard::settings() const
{
    return d->settings;
}

bool VidSlideWizard::validateCurrentPage()
{
    if (!DWizardDlg::validateCurrentPage())
        return false;

    return true;
}

int VidSlideWizard::nextId() const
{
    if (d->settings->selMode == VidSlideSettings::ALBUMS)
    {
        if (currentPage() == d->introPage)
            return d->albumsPage->id();
    }
    else
    {
        if (currentPage() == d->introPage)
            return d->imagesPage->id();
    }

    return DWizardDlg::nextId();
}

void VidSlideWizard::slotCurrentIdChanged(int id)
{
    if (page(id) == d->videoPage)
    {
        d->videoPage->slotTransitionChanged();
        d->videoPage->slotEffectChanged();
    }
}

} // namespace Digikam
