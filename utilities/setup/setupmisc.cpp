/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-23
 * Description : mics configuration setup tab
 *
 * Copyright (C) 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QStyleFactory>

// KDE includes

#include <kapplication.h>
#include <kcombobox.h>
#include <kdialog.h>
#include <khbox.h>
#include <klocale.h>

// Local includes

#include "applicationsettings.h"

namespace Digikam
{

class SetupMisc::Private
{
public:

    Private() :
        sidebarTypeLabel(0),
        stringComparisonTypeLabel(0),
        applicationStyleLabel(0),
        showSplashCheck(0),
        showTrashDeleteDialogCheck(0),
        showPermanentDeleteDialogCheck(0),
        sidebarApplyDirectlyCheck(0),
        sidebarType(0),
        stringComparisonType(0),
        applicationStyle(0)
    {
    }

    QLabel*    sidebarTypeLabel;
    QLabel*    stringComparisonTypeLabel;
    QLabel*    applicationStyleLabel;

    QCheckBox* showSplashCheck;
    QCheckBox* showTrashDeleteDialogCheck;
    QCheckBox* showPermanentDeleteDialogCheck;
    QCheckBox* sidebarApplyDirectlyCheck;

    KComboBox* sidebarType;
    KComboBox* stringComparisonType;
    KComboBox* applicationStyle;
};

SetupMisc::SetupMisc(QWidget* const parent)
    : QScrollArea(parent), d(new Private)
{
    QWidget* const panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    // --------------------------------------------------------

    QVBoxLayout* const layout         = new QVBoxLayout(panel);
    d->showTrashDeleteDialogCheck     = new QCheckBox(i18n("Confirm when moving items to the &trash"), panel);
    d->showPermanentDeleteDialogCheck = new QCheckBox(i18n("Confirm when permanently deleting items"), panel);
    d->sidebarApplyDirectlyCheck      = new QCheckBox(i18n("Do not confirm when applying changes in the &right sidebar"), panel);
    d->showSplashCheck                = new QCheckBox(i18n("&Show splash screen at startup"), panel);
    
    // --------------------------------------------------------

    KHBox* const tabStyleHbox = new KHBox(panel);
    d->sidebarTypeLabel       = new QLabel(i18n("Sidebar tab title:"), tabStyleHbox);
    d->sidebarType            = new KComboBox(tabStyleHbox);
    d->sidebarType->addItem(i18n("Only For Active Tab"), 0);
    d->sidebarType->addItem(i18n("For All Tabs"),        1);
    d->sidebarType->setToolTip(i18n("Set this option to configure how sidebar tab titles are visible."));

    // --------------------------------------------------------

    KHBox* const stringComparisonHbox = new KHBox(panel);
    d->stringComparisonTypeLabel      = new QLabel(i18n("String comparison type:"), stringComparisonHbox);
    d->stringComparisonType           = new KComboBox(stringComparisonHbox);
    d->stringComparisonType->addItem(i18nc("method to compare strings", "Natural"), ApplicationSettings::Natural);
    d->stringComparisonType->addItem(i18nc("method to compare strings", "Normal"),  ApplicationSettings::Normal);
    d->stringComparisonType->setToolTip(i18n("<qt>Sets the way in which strings are compared inside digiKam. "
                                             "This eg. influences the sorting of the tree views.<br/>"
                                             "<b>Natural</b> tries to compare strings in a way that regards some normal conventions "
                                             "and will eg. result in sorting numbers naturally even if they have a different number of digits.<br/>"
                                             "<b>Normal</b> uses a more technical approach. "
                                             "Use this style if you eg. want to entitle albums with ISO dates (201006 or 20090523) "
                                             "and the albums should be sorted according to these dates.</qt>"));

    // --------------------------------------------------------

    KHBox* const appStyleHbox = new KHBox(panel);
    d->applicationStyleLabel  = new QLabel(i18n("Widget style:"), appStyleHbox);
    d->applicationStyle       = new KComboBox(appStyleHbox);
    d->applicationStyle->setToolTip(i18n("Set this option to choose the default window decoration and looks."));

    QStringList styleList = QStyleFactory::keys();

    for (int i = 0; i < styleList.count(); ++i)
    {
        d->applicationStyle->addItem(styleList.at(i));
    }

    // --------------------------------------------------------

    layout->setMargin(KDialog::spacingHint());
    layout->setSpacing(KDialog::spacingHint());
    layout->addWidget(d->showTrashDeleteDialogCheck);
    layout->addWidget(d->showPermanentDeleteDialogCheck);
    layout->addWidget(d->sidebarApplyDirectlyCheck);
    layout->addWidget(d->showSplashCheck);
    layout->addWidget(tabStyleHbox);
    layout->addWidget(appStyleHbox);
    layout->addWidget(stringComparisonHbox);
    layout->addStretch();

    readSettings();
    adjustSize();

    // --------------------------------------------------------
}

SetupMisc::~SetupMisc()
{
    delete d;
}

void SetupMisc::applySettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    settings->setShowSplashScreen(d->showSplashCheck->isChecked());
    settings->setShowTrashDeleteDialog(d->showTrashDeleteDialogCheck->isChecked());
    settings->setShowPermanentDeleteDialog(d->showPermanentDeleteDialogCheck->isChecked());
    settings->setApplySidebarChangesDirectly(d->sidebarApplyDirectlyCheck->isChecked());
    settings->setSidebarTitleStyle(d->sidebarType->currentIndex() == 0 ? KMultiTabBar::VSNET : KMultiTabBar::KDEV3ICON);
    settings->setStringComparisonType((ApplicationSettings::StringComparisonType)d->stringComparisonType->itemData(d->stringComparisonType->currentIndex()).toInt());
    settings->setApplicationStyle(d->applicationStyle->currentText());
    settings->saveSettings();
}

void SetupMisc::readSettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    d->showSplashCheck->setChecked(settings->getShowSplashScreen());
    d->showTrashDeleteDialogCheck->setChecked(settings->getShowTrashDeleteDialog());
    d->showPermanentDeleteDialogCheck->setChecked(settings->getShowPermanentDeleteDialog());
    d->sidebarApplyDirectlyCheck->setChecked(settings->getApplySidebarChangesDirectly());
    d->sidebarType->setCurrentIndex(settings->getSidebarTitleStyle() == KMultiTabBar::VSNET ? 0 : 1);
    d->stringComparisonType->setCurrentIndex(settings->getStringComparisonType());
    d->applicationStyle->setCurrentIndex(d->applicationStyle->findText(settings->getApplicationStyle()));
}

}  // namespace Digikam
