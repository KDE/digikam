/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-23
 * Description : mics configuration setup tab
 *
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupmisc.h"

// Qt includes.

#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QLabel>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <kcombobox.h>
#include <khbox.h>

// Local includes.

#include "albumsettings.h"

namespace Digikam
{

class SetupMiscPriv
{
public:

    SetupMiscPriv()
    {
        showSplashCheck            = 0;
        showTrashDeleteDialogCheck = 0;
        sidebarApplyDirectlyCheck  = 0;
        scanAtStart                = 0;
        sidebarTypeLabel           = 0;
        sidebarType                = 0;
    }

    QLabel*    sidebarTypeLabel;

    QCheckBox* showSplashCheck;
    QCheckBox* showTrashDeleteDialogCheck;
    QCheckBox* sidebarApplyDirectlyCheck;
    QCheckBox* scanAtStart;

    KComboBox* sidebarType;
};

SetupMisc::SetupMisc(QWidget* parent)
         : QScrollArea(parent), d(new SetupMiscPriv)
{
    QWidget *panel = new QWidget(viewport());
    panel->setAutoFillBackground(false);
    setWidget(panel);
    setWidgetResizable(true);
    viewport()->setAutoFillBackground(false);

    QVBoxLayout *layout           = new QVBoxLayout(panel);

    d->showTrashDeleteDialogCheck = new QCheckBox(i18n("Confirm when moving items to the &trash"), panel);

    d->sidebarApplyDirectlyCheck  = new QCheckBox(i18n("Do not confirm when apply changes in the &right sidebar"), panel);

    d->showSplashCheck            = new QCheckBox(i18n("&Show splash screen at startup"), panel);

    d->scanAtStart                = new QCheckBox(i18n("&Scan for new items at startup (makes startup slower)"), panel);

    KHBox *hbox         = new KHBox(panel);
    d->sidebarTypeLabel = new QLabel(i18n("Sidebar tab title:"), hbox);
    d->sidebarType      = new KComboBox(hbox);
    d->sidebarType->addItem(i18n("Only For Active Tab"), 0);
    d->sidebarType->addItem(i18n("For All Tabs"),        1);
    d->sidebarType->setToolTip(i18n("Set this option to configure how sidebars tab title are visible."));

    // --------------------------------------------------------

    layout->setMargin(KDialog::spacingHint());
    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(d->showTrashDeleteDialogCheck);
    layout->addWidget(d->sidebarApplyDirectlyCheck);
    layout->addWidget(d->showSplashCheck);
    layout->addWidget(d->scanAtStart);
    layout->addWidget(hbox);
    layout->addStretch();

    readSettings();
    adjustSize();
}

SetupMisc::~SetupMisc()
{
    delete d;
}

void SetupMisc::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    settings->setShowSplashScreen(d->showSplashCheck->isChecked());
    settings->setShowTrashDeleteDialog(d->showTrashDeleteDialogCheck->isChecked());
    settings->setApplySidebarChangesDirectly(d->sidebarApplyDirectlyCheck->isChecked());
    settings->setScanAtStart(d->scanAtStart->isChecked());
    settings->setSidebarTitleStyle(d->sidebarType->currentIndex() == 0 ? KMultiTabBar::VSNET : KMultiTabBar::KDEV3ICON);
    settings->saveSettings();
}

void SetupMisc::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();

    d->showSplashCheck->setChecked(settings->getShowSplashScreen());
    d->showTrashDeleteDialogCheck->setChecked(settings->getShowTrashDeleteDialog());
    d->sidebarApplyDirectlyCheck->setChecked(settings->getApplySidebarChangesDirectly());
    d->scanAtStart->setChecked(settings->getScanAtStart());
    d->sidebarType->setCurrentIndex(settings->getSidebarTitleStyle() == KMultiTabBar::VSNET ? 0 : 1);
}

}  // namespace Digikam
