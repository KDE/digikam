/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-23
 * Description : mics configuration setup tab
 *
 * Copyright (C) 2004      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2017      by Simon Frei <freisim93 at gmail dot com>
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

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QGroupBox>
#include <QHash>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QStyle>
#include <QStyleFactory>
#include <QVBoxLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "dlayoutbox.h"
#include "applicationsettings.h"

namespace Digikam
{

class SetupMisc::Private
{
public:

    Private() :
        tab(0),
        sidebarTypeLabel(0),
        stringComparisonTypeLabel(0),
        applicationStyleLabel(0),
        iconThemeLabel(0),
        minSimilarityBoundLabel(0),
        showSplashCheck(0),
        showTrashDeleteDialogCheck(0),
        showPermanentDeleteDialogCheck(0),
        sidebarApplyDirectlyCheck(0),
        useNativeFileDialogCheck(0),
        drawFramesToGroupedCheck(0),
        scrollItemToCenterCheck(0),
        showOnlyPersonTagsInPeopleSidebarCheck(0),
        scanAtStart(0),
        cleanAtStart(0),
        sidebarType(0),
        stringComparisonType(0),
        applicationStyle(0),
        iconTheme(0),
        minimumSimilarityBound(0),
        groupingButtons(QHash<int, QButtonGroup*>())
    {
    }

    QTabWidget*               tab;

    QLabel*                   sidebarTypeLabel;
    QLabel*                   stringComparisonTypeLabel;
    QLabel*                   applicationStyleLabel;
    QLabel*                   iconThemeLabel;
    QLabel*                   minSimilarityBoundLabel;

    QCheckBox*                showSplashCheck;
    QCheckBox*                showTrashDeleteDialogCheck;
    QCheckBox*                showPermanentDeleteDialogCheck;
    QCheckBox*                sidebarApplyDirectlyCheck;
    QCheckBox*                useNativeFileDialogCheck;
    QCheckBox*                drawFramesToGroupedCheck;
    QCheckBox*                scrollItemToCenterCheck;
    QCheckBox*                showOnlyPersonTagsInPeopleSidebarCheck;
    QCheckBox*                scanAtStart;
    QCheckBox*                cleanAtStart;

    QComboBox*                sidebarType;
    QComboBox*                stringComparisonType;
    QComboBox*                applicationStyle;
    QComboBox*                iconTheme;

    QSpinBox*                 minimumSimilarityBound;

    QHash<int, QButtonGroup*> groupingButtons;
};

SetupMisc::SetupMisc(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    d->tab = new QTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    // --------------------------------------------------------

    QWidget* const behaviourPanel = new QWidget(d->tab);
    QVBoxLayout* const layout     = new QVBoxLayout(behaviourPanel);

    // --------------------------------------------------------

    DHBox* const stringComparisonHbox = new DHBox(behaviourPanel);
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

    d->showTrashDeleteDialogCheck     = new QCheckBox(i18n("Confirm when moving items to the &trash"), behaviourPanel);
    d->showPermanentDeleteDialogCheck = new QCheckBox(i18n("Confirm when permanently deleting items"), behaviourPanel);
    d->sidebarApplyDirectlyCheck      = new QCheckBox(i18n("Do not confirm when applying changes in the &right sidebar"), behaviourPanel);
    d->scanAtStart                    = new QCheckBox(i18n("&Scan for new items at startup (makes startup slower)"), behaviourPanel);
    d->scanAtStart->setToolTip(i18n("Set this option to force digiKam to scan all collections for new items to\n"
                                    "register new elements in database. The scan is performed in the background through\n"
                                    "the progress manager available in the statusbar\n when digiKam main interface\n"
                                    "is loaded. If your computer is fast enough, this will have no effect on usability\n"
                                    "of digiKam while scanning. If your collections are huge or if you use a remote database,\n"
                                    "this can introduce low latency, and it's recommended to disable this option and to plan\n"
                                    "a manual scan through the maintenance tool at the right moment."));

    // ---------------------------------------------------------

    d->cleanAtStart                   = new QCheckBox(i18n("Remove obsolete core database objects (makes startup slower)"), behaviourPanel);
    d->cleanAtStart->setToolTip(i18n("Set this option to force digiKam to clean up the core database from obsolete item entries.\n"
                                     "Entries are only deleted if the connected image/video/audio file was already removed, i.e.\n"
                                     "the database object wastes space.\n"
                                     "This option does not clean up other databases as the thumbnails or recognition db.\n"
                                     "For clean up routines for other databases, please use the maintenance."));

    // -- Application Behavior Options --------------------------------------------------------

    QGroupBox* const abOptionsGroup = new QGroupBox(i18n("Application Behavior"), behaviourPanel);
    QVBoxLayout* const gLayout5     = new QVBoxLayout();

    d->showSplashCheck                        = new QCheckBox(i18n("&Show splash screen at startup"), abOptionsGroup);
    d->useNativeFileDialogCheck               = new QCheckBox(i18n("Use file dialogs from the system"), abOptionsGroup);
    d->drawFramesToGroupedCheck               = new QCheckBox(i18n("Draw frames around grouped items"), abOptionsGroup);
    d->scrollItemToCenterCheck                = new QCheckBox(i18n("Scroll current item to center of thumbbar"), abOptionsGroup);
    d->showOnlyPersonTagsInPeopleSidebarCheck = new QCheckBox(i18n("Show only face tags for assigning names in people sidebar"), abOptionsGroup);

    DHBox* const minSimilarityBoundHbox       = new DHBox(abOptionsGroup);
    d->minSimilarityBoundLabel                = new QLabel(i18n("Lower bound for minimum similarity:"), minSimilarityBoundHbox);
    d->minimumSimilarityBound                 = new QSpinBox(minSimilarityBoundHbox);
    d->minimumSimilarityBound->setSuffix(QLatin1String("%"));
    d->minimumSimilarityBound->setRange(1, 100);
    d->minimumSimilarityBound->setSingleStep(1);
    d->minimumSimilarityBound->setValue(40);
    d->minimumSimilarityBound->setToolTip(i18n("Select here the lower bound of "
                                               "the minimum similarity threshold "
                                               "for fuzzy and duplicates searches. "
                                               "The default value is 40. Selecting "
                                               "a lower value than 40 can make the search <b>really</b> slow."));
    d->minSimilarityBoundLabel->setBuddy(d->minimumSimilarityBound);

    DHBox* const tabStyleHbox = new DHBox(abOptionsGroup);
    d->sidebarTypeLabel       = new QLabel(i18n("Sidebar tab title:"), tabStyleHbox);
    d->sidebarType            = new QComboBox(tabStyleHbox);
    d->sidebarType->addItem(i18n("Only For Active Tab"), 0);
    d->sidebarType->addItem(i18n("For All Tabs"),        1);
    d->sidebarType->setToolTip(i18n("Set this option to configure how sidebar tab titles are visible. "
                                    "Use \"Only For Active Tab\" option if you use a small screen resolution as with a laptop computer."));

    DHBox* const appStyleHbox = new DHBox(abOptionsGroup);
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

    DHBox* const iconThemeHbox = new DHBox(abOptionsGroup);
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

    gLayout5->addWidget(d->showSplashCheck);
    gLayout5->addWidget(d->useNativeFileDialogCheck);
    gLayout5->addWidget(d->drawFramesToGroupedCheck);
    gLayout5->addWidget(d->scrollItemToCenterCheck);
    gLayout5->addWidget(d->showOnlyPersonTagsInPeopleSidebarCheck);
    gLayout5->addWidget(minSimilarityBoundHbox);
    gLayout5->addWidget(tabStyleHbox);
    gLayout5->addWidget(appStyleHbox);
    gLayout5->addWidget(iconThemeHbox);
    abOptionsGroup->setLayout(gLayout5);

    // --------------------------------------------------------

    layout->setContentsMargins(spacing, spacing, spacing, spacing);
    layout->setSpacing(spacing);
    layout->addWidget(stringComparisonHbox);
    layout->addWidget(d->scanAtStart);
    layout->addWidget(d->cleanAtStart);
    layout->addWidget(d->showTrashDeleteDialogCheck);
    layout->addWidget(d->showPermanentDeleteDialogCheck);
    layout->addWidget(d->sidebarApplyDirectlyCheck);
    layout->addWidget(abOptionsGroup);
    layout->addStretch();

    // --------------------------------------------------------

    d->tab->insertTab(Behaviour, behaviourPanel, i18nc("@title:tab", "Behaviour"));

    // --------------------------------------------------------

    QWidget* const groupingPanel = new QWidget(d->tab);
    QGridLayout* const grid      = new QGridLayout(groupingPanel);

    // --------------------------------------------------------

    QLabel* const description    = new QLabel(i18n("Perform the following operations on all group members:"), groupingPanel);
    description->setToolTip(i18n("When images are grouped the following operations<br/>"
                                 "are performed only on the displayed item (No)<br/>"
                                 "or on all the hidden items in the group as well (Yes).<br/>"
                                 "If Ask is selected, there will be a prompt every<br/>"
                                 "time this operation is executed."));

    QLabel* const noLabel        = new QLabel(i18n("No"), groupingPanel);
    QLabel* const yesLabel       = new QLabel(i18n("Yes"), groupingPanel);
    QLabel* const askLabel       = new QLabel(i18n("Ask"), groupingPanel);

    QHash<int, QLabel*> labels;

    for (int i = 0 ; i != ApplicationSettings::Unspecified ; ++i)
    {
        labels.insert(i, new QLabel(ApplicationSettings::operationTypeTitle(
                                    (ApplicationSettings::OperationType)i), groupingPanel));
        QString explanation = ApplicationSettings::operationTypeExplanation(
                              (ApplicationSettings::OperationType)i);

        if (!explanation.isEmpty())
        {
            labels.value(i)->setToolTip(explanation);
        }

        d->groupingButtons.insert(i, new QButtonGroup(groupingPanel));
        d->groupingButtons.value(i)->addButton(new QRadioButton(groupingPanel), 0);
        d->groupingButtons.value(i)->addButton(new QRadioButton(groupingPanel), 1);
        d->groupingButtons.value(i)->addButton(new QRadioButton(groupingPanel), 2);
    }

    // --------------------------------------------------------

    grid->addWidget(description, 0, 0, 1, 4);
    grid->addWidget(noLabel,     1, 1, 1, 1);
    grid->addWidget(yesLabel,    1, 2, 1, 1);
    grid->addWidget(askLabel,    1, 3, 1, 1);

    for (int i = 0 ; i != ApplicationSettings::Unspecified ; ++i)
    {
        grid->addWidget(labels.value(i),                        i+2, 0, 1, 1);
        grid->addWidget(d->groupingButtons.value(i)->button(0), i+2, 1, 1, 1);
        grid->addWidget(d->groupingButtons.value(i)->button(1), i+2, 2, 1, 1);
        grid->addWidget(d->groupingButtons.value(i)->button(2), i+2, 3, 1, 1);
    }

    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);
    grid->setColumnStretch(0, 10);
    grid->setColumnMinimumWidth(1, 30);
    grid->setColumnMinimumWidth(2, 30);
    grid->setColumnMinimumWidth(3, 30);
    grid->setRowStretch(20, 10);

    // --------------------------------------------------------

    d->tab->insertTab(Grouping, groupingPanel, i18nc("@title:tab", "Grouping"));

    // --------------------------------------------------------

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
    settings->setMinimumSimilarityBound(d->minimumSimilarityBound->value());
    settings->setApplySidebarChangesDirectly(d->sidebarApplyDirectlyCheck->isChecked());
    settings->setScanAtStart(d->scanAtStart->isChecked());
    settings->setCleanAtStart(d->cleanAtStart->isChecked());
    settings->setUseNativeFileDialog(d->useNativeFileDialogCheck->isChecked());
    settings->setDrawFramesToGrouped(d->drawFramesToGroupedCheck->isChecked());
    settings->setScrollItemToCenter(d->scrollItemToCenterCheck->isChecked());
    settings->setShowOnlyPersonTagsInPeopleSidebar(d->showOnlyPersonTagsInPeopleSidebarCheck->isChecked());
    settings->setSidebarTitleStyle(d->sidebarType->currentIndex() == 0 ? DMultiTabBar::ActiveIconText : DMultiTabBar::AllIconsText);
    settings->setStringComparisonType((ApplicationSettings::StringComparisonType)
                                      d->stringComparisonType->itemData(d->stringComparisonType->currentIndex()).toInt());

    for (int i = 0 ; i != ApplicationSettings::Unspecified ; ++i)
    {
        settings->setGroupingOperateOnAll((ApplicationSettings::OperationType)i,
                                          (ApplicationSettings::ApplyToEntireGroup)d->groupingButtons.value(i)->checkedId());
    }

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
    d->minimumSimilarityBound->setValue(settings->getMinimumSimilarityBound());
    d->sidebarApplyDirectlyCheck->setChecked(settings->getApplySidebarChangesDirectly());
    d->sidebarApplyDirectlyCheck->setChecked(settings->getApplySidebarChangesDirectly());
    d->scanAtStart->setChecked(settings->getScanAtStart());
    d->cleanAtStart->setChecked(settings->getCleanAtStart());
    d->useNativeFileDialogCheck->setChecked(settings->getUseNativeFileDialog());
    d->drawFramesToGroupedCheck->setChecked(settings->getDrawFramesToGrouped());
    d->scrollItemToCenterCheck->setChecked(settings->getScrollItemToCenter());
    d->showOnlyPersonTagsInPeopleSidebarCheck->setChecked(settings->showOnlyPersonTagsInPeopleSidebar());
    d->sidebarType->setCurrentIndex(settings->getSidebarTitleStyle() == DMultiTabBar::ActiveIconText ? 0 : 1);
    d->stringComparisonType->setCurrentIndex(settings->getStringComparisonType());

    for (int i = 0 ; i != ApplicationSettings::Unspecified ; ++i)
    {
        d->groupingButtons.value(i)->button((int)settings->getGroupingOperateOnAll((ApplicationSettings::OperationType)i))->setChecked(true);
    }

#ifdef HAVE_APPSTYLE_SUPPORT
    d->applicationStyle->setCurrentIndex(d->applicationStyle->findText(settings->getApplicationStyle(),
                                                                       Qt::MatchFixedString));
#endif

    d->iconTheme->setCurrentIndex(d->iconTheme->findData(settings->getIconTheme()));
}

} // namespace Digikam
