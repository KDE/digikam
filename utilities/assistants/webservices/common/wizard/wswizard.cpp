/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items to web services.
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

#include "wswizard.h"

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
#include "wsintropage.h"
#include "wsalbumspage.h"
#include "wsimagespage.h"
#include "wssettingspage.h"
#include "wsfinalpage.h"

namespace Digikam
{

class WSWizard::Private
{
public:

    explicit Private()
      : iface(0),
        introPage(0),
        albumsPage(0),
        imagesPage(0),
        settingsPage(0),
        finalPage(0),
        settings(0)
    {
    }

    DInfoInterface*   iface;
    WSIntroPage*      introPage;
    WSAlbumsPage*     albumsPage;
    WSImagesPage*     imagesPage;
    WSSettingsPage*   settingsPage;
    WSFinalPage*      finalPage;
    WSSettings*       settings;
};

WSWizard::WSWizard(QWidget* const parent, DInfoInterface* const iface)
    : DWizardDlg(parent, QLatin1String("Web Services Dialog")),
      d(new Private)
{
    setOption(QWizard::NoCancelButtonOnLastPage);
    setWindowTitle(i18n("Export to Web Services"));

    d->iface             = iface;
    d->settings          = new WSSettings;

    KConfig config;
    KConfigGroup group   = config.group("Web Services Dialog Settings");
    d->settings->readSettings(group);

    d->introPage         = new WSIntroPage(this,    i18n("Welcome to Web Services Tool"));
    d->albumsPage        = new WSAlbumsPage(this,   i18n("Albums Selection"));
    d->imagesPage        = new WSImagesPage(this,   i18n("Images List"));
    d->settingsPage      = new WSSettingsPage(this, i18n("Web Service Settings"));
    d->finalPage         = new WSFinalPage(this,    i18n("Export by Web Service"));
}

WSWizard::~WSWizard()
{
    KConfig config;
    KConfigGroup group = config.group("Web Services Dialog Settings");
    d->settings->writeSettings(group);

    delete d;
}

void WSWizard::setItemsList(const QList<QUrl>& urls)
{
    d->imagesPage->setItemsList(urls);
}

DInfoInterface* WSWizard::iface() const
{
    return d->iface;
}

WSSettings* WSWizard::settings() const
{
    return d->settings;
}

bool WSWizard::validateCurrentPage()
{
    if (!DWizardDlg::validateCurrentPage())
        return false;

    return true;
}

int WSWizard::nextId() const
{
    if (d->settings->selMode == WSSettings::ALBUMS)
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

} // namespace Digikam
