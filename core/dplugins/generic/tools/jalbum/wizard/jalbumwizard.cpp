/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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

#include "jalbumwizard.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QMenu>
#include <QApplication>
#include <QComboBox>
#include <QListWidget>
#include <QTextBrowser>

// KDE includes

#include <kconfigdialogmanager.h>
#include <klocalizedstring.h>

// Local includes

#include "dinfointerface.h"
#include "dwizardpage.h"
#include "digikam_debug.h"
#include "jalbumfinalpage.h"
#include "jalbuminfo.h"
#include "jalbumintropage.h"
#include "jalbumoutputpage.h"
#include "jalbumselectionpage.h"

namespace Digikam
{

class Q_DECL_HIDDEN JAlbumWizard::Private
{
public:

    explicit Private()
      : info(0),
        configManager(0),
        introPage(0),
        selectionPage(0),
        outputPage(0),
        finalPage(0)
    {
    }

    JAlbumInfo*            info;
    KConfigDialogManager*  configManager;

    JAlbumIntroPage*       introPage;
    JAlbumSelectionPage*   selectionPage;
    JAlbumOutputPage*      outputPage;
    JAlbumFinalPage*       finalPage;
};

JAlbumWizard::JAlbumWizard(QWidget* const parent, DInfoInterface* const iface)
    : DWizardDlg(parent, QLatin1String("jAlbum Album Creation Dialog")),
      d(new Private)
{
    setOption(QWizard::NoCancelButtonOnLastPage);
    setWindowTitle(i18n("Create jAlbum Album"));

    d->info = new JAlbumInfo(iface);
    d->info->load();

    d->introPage         = new JAlbumIntroPage(this,         i18n("Welcome to jAlbum Album Tool"));
    d->selectionPage     = new JAlbumSelectionPage(this,     i18n("Items Selection"));
    d->outputPage        = new JAlbumOutputPage(this,        i18n("Paths Selection"));
    d->finalPage         = new JAlbumFinalPage(this,         i18n("Generating jAlbum"));
    d->configManager     = new KConfigDialogManager(this, d->info);
    d->configManager->updateWidgets();
}

JAlbumWizard::~JAlbumWizard()
{
    delete d;
}

void JAlbumWizard::setItemsList(const QList<QUrl>& urls)
{
    d->selectionPage->setItemsList(urls);
}

bool JAlbumWizard::validateCurrentPage()
{
    if (!DWizardDlg::validateCurrentPage())
        return false;

    if (currentPage() == d->outputPage)
    {
        d->configManager->updateSettings();
        d->info->save();
    }

    return true;
}

int JAlbumWizard::nextId() const
{
    return DWizardDlg::nextId();
}

JAlbumInfo* JAlbumWizard::jalbumInfo() const
{
    return d->info;
}

} // namespace Digikam
