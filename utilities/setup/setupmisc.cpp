/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-23
 * Description : mics configuration setup tab
 *
 * Copyright (C) 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QApplication>
#include <QStyle>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "dwidgetutils.h"
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
        iconThemeLabel(0),
        showSplashCheck(0),
        showTrashDeleteDialogCheck(0),
        showPermanentDeleteDialogCheck(0),
        sidebarApplyDirectlyCheck(0),
        scrollItemToCenterCheck(0),
        scanAtStart(0),
        sidebarType(0),
        stringComparisonType(0),
        applicationStyle(0),
        iconTheme(0)
    {
    }

    QLabel*    sidebarTypeLabel;
    QLabel*    stringComparisonTypeLabel;
    QLabel*    applicationStyleLabel;
    QLabel*    iconThemeLabel;

    QCheckBox* showSplashCheck;
    QCheckBox* showTrashDeleteDialogCheck;
    QCheckBox* showPermanentDeleteDialogCheck;
    QCheckBox* sidebarApplyDirectlyCheck;
    QCheckBox* scrollItemToCenterCheck;
    QCheckBox* scanAtStart;

    QComboBox* sidebarType;
    QComboBox* stringComparisonType;
    QComboBox* applicationStyle;
    QComboBox* iconTheme;
};

SetupMisc::SetupMisc(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QWidget* const panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    // --------------------------------------------------------

    QVBoxLayout* const layout         = new QVBoxLayout(panel);
    d->showTrashDeleteDialogCheck     = new QCheckBox(i18n("Confirm when moving items to the &trash"), panel);
    d->showPermanentDeleteDialogCheck = new QCheckBox(i18n("Confirm when permanently deleting items"), panel);
    d->sidebarApplyDirectlyCheck      = new QCheckBox(i18n("Do not confirm when applying changes in the &right sidebar"), panel);
    d->scrollItemToCenterCheck        = new QCheckBox(i18n("Scroll current item to center of thumbbar"), panel);
    d->showSplashCheck                = new QCheckBox(i18n("&Show splash screen at startup"), panel);
    d->scanAtStart                    = new QCheckBox(i18n("&Scan for new items at startup (makes startup slower)"), panel);
    d->scanAtStart->setToolTip(i18n("Set this option to force digiKam to scan all collections for new items to\n"
                                    "register new elements in database. The scan is performed in the background through\n"
                                    "the progress manager available in the statusbar\n when digiKam main interface\n"
                                    "is loaded. If your computer is fast enough, this will have no effect on usability\n"
                                    "of digiKam while scanning. If your collections are huge or if you use a remote database,\n"
                                    "this can introduce low latency, and it's recommended to disable this option and to plan\n"
                                    "a manual scan through the maintenance tool at the right moment."));

    // --------------------------------------------------------

    DHBox* const tabStyleHbox = new DHBox(panel);
    d->sidebarTypeLabel       = new QLabel(i18n("Sidebar tab title:"), tabStyleHbox);
    d->sidebarType            = new QComboBox(tabStyleHbox);
    d->sidebarType->addItem(i18n("Only For Active Tab"), 0);
    d->sidebarType->addItem(i18n("For All Tabs"),        1);
    d->sidebarType->setToolTip(i18n("Set this option to configure how sidebar tab titles are visible. "
                                    "Use \"Only For Active Tab\" option if you use a small screen resolution as with a laptop computer."));

    // --------------------------------------------------------

    DHBox* const stringComparisonHbox = new DHBox(panel);
    d->stringComparisonTypeLabel      = new QLabel(i18n("String comparison type:"), stringComparisonHbox);
    d->stringComparisonType           = new QComboBox(stringComparisonHbox);
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

    DHBox* const appStyleHbox = new DHBox(panel);
    d->applicationStyleLabel  = new QLabel(i18n("Widget style:"), appStyleHbox);
    d->applicationStyle       = new QComboBox(appStyleHbox);
    d->applicationStyle->setToolTip(i18n("Set this option to choose the default window decoration and looks."));

    QStringList styleList = QStyleFactory::keys();

    for (int i = 0 ; i < styleList.count() ; ++i)
    {
        d->applicationStyle->addItem(styleList.at(i));
    }

#ifndef HAVE_APPSTYLE_SUPPORT
    // See Bug #365262
    appStyleHbox->setVisible(false);
#endif

    // --------------------------------------------------------

    DHBox* const iconThemeHbox = new DHBox(panel);
    d->iconThemeLabel          = new QLabel(i18n("Icon theme (changes after restart):"), iconThemeHbox);
    d->iconTheme               = new QComboBox(iconThemeHbox);
    d->iconTheme->setToolTip(i18n("Set this option to choose the default icon theme."));

    d->iconTheme->addItem(i18n("Use Icon Theme From System"), QString());

    const QString indexTheme = QLatin1String("/index.theme");
    const QString breezeDark = QLatin1String("/breeze-dark");
    const QString breeze     = QLatin1String("/breeze");

    bool foundBreezeDark     = false;
    bool foundBreeze         = false;

    foreach(const QString& path, QIcon::themeSearchPaths())
    {
        if (!foundBreeze && QFile::exists(path + breeze + indexTheme))
        {
            d->iconTheme->addItem(i18n("Breeze"), breeze.mid(1));
            foundBreeze = true;
        }

        if (!foundBreezeDark && QFile::exists(path + breezeDark + indexTheme))
        {
            d->iconTheme->addItem(i18n("Breeze Dark"), breezeDark.mid(1));
            foundBreezeDark = true;
        }
    }

    // --------------------------------------------------------

    layout->setContentsMargins(spacing, spacing, spacing, spacing);
    layout->setSpacing(spacing);
    layout->addWidget(d->showTrashDeleteDialogCheck);
    layout->addWidget(d->showPermanentDeleteDialogCheck);
    layout->addWidget(d->sidebarApplyDirectlyCheck);
    layout->addWidget(d->scrollItemToCenterCheck);
    layout->addWidget(d->showSplashCheck);
    layout->addWidget(d->scanAtStart);
    layout->addWidget(tabStyleHbox);
    layout->addWidget(appStyleHbox);
    layout->addWidget(iconThemeHbox);
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
    settings->setScanAtStart(d->scanAtStart->isChecked());
    settings->setScrollItemToCenter(d->scrollItemToCenterCheck->isChecked());
    settings->setSidebarTitleStyle(d->sidebarType->currentIndex() == 0 ? DMultiTabBar::ActiveIconText : DMultiTabBar::AllIconsText);
    settings->setStringComparisonType((ApplicationSettings::StringComparisonType)d->stringComparisonType->itemData(d->stringComparisonType->currentIndex()).toInt());

#ifdef HAVE_APPSTYLE_SUPPORT
    settings->setApplicationStyle(d->applicationStyle->currentText());
#endif

    settings->setIconTheme(d->iconTheme->currentData().toString());
    settings->saveSettings();
}

void SetupMisc::readSettings()
{
    ApplicationSettings* const settings = ApplicationSettings::instance();

    d->showSplashCheck->setChecked(settings->getShowSplashScreen());
    d->showTrashDeleteDialogCheck->setChecked(settings->getShowTrashDeleteDialog());
    d->showPermanentDeleteDialogCheck->setChecked(settings->getShowPermanentDeleteDialog());
    d->sidebarApplyDirectlyCheck->setChecked(settings->getApplySidebarChangesDirectly());
    d->sidebarApplyDirectlyCheck->setChecked(settings->getApplySidebarChangesDirectly());
    d->scanAtStart->setChecked(settings->getScanAtStart());
    d->scrollItemToCenterCheck->setChecked(settings->getScrollItemToCenter());
    d->sidebarType->setCurrentIndex(settings->getSidebarTitleStyle() == DMultiTabBar::ActiveIconText ? 0 : 1);
    d->stringComparisonType->setCurrentIndex(settings->getStringComparisonType());

#ifdef HAVE_APPSTYLE_SUPPORT
    d->applicationStyle->setCurrentIndex(d->applicationStyle->findText(settings->getApplicationStyle(), Qt::MatchFixedString));
#endif

    d->iconTheme->setCurrentIndex(d->iconTheme->findData(settings->getIconTheme()));
}

}  // namespace Digikam
