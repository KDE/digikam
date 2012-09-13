/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-02
 * Description : setup Misc tab.
 *
 * Copyright (C) 2005-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2008 by Arnd Baecker <arnd dot baecker at web dot de>
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
#include <QColor>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QStyleFactory>

// KDE includes

#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmultitabbar.h>
#include <knuminput.h>
#include <kvbox.h>

namespace ShowFoto
{

class SetupMisc::Private
{
public:

    Private() :
        sidebarTypeLabel(0),
        applicationStyleLabel(0),
        showSplash(0),
        sortReverse(0),
        useTrash(0),
        sidebarType(0),
        sortOrderComboBox(0),
        applicationStyle(0)
    {}

    static const QString configGroupName;
    static const QString configDeleteItem2TrashEntry;
    static const QString configShowSplashEntry;
    static const QString configSidebarTitleStyleEntry;
    static const QString configSortOrderEntry;
    static const QString configReverseSortEntry;
    static const QString configApplicationStyleEntry;

    QLabel*              sidebarTypeLabel;
    QLabel*              applicationStyleLabel;

    QCheckBox*           showSplash;
    QCheckBox*           sortReverse;
    QCheckBox*           useTrash;

    KComboBox*           sidebarType;
    KComboBox*           sortOrderComboBox;
    KComboBox*           applicationStyle;
};

const QString SetupMisc::Private::configGroupName("ImageViewer Settings");
const QString SetupMisc::Private::configDeleteItem2TrashEntry("DeleteItem2Trash");
const QString SetupMisc::Private::configShowSplashEntry("ShowSplash");
const QString SetupMisc::Private::configSidebarTitleStyleEntry("Sidebar Title Style");
const QString SetupMisc::Private::configSortOrderEntry("SortOrder");
const QString SetupMisc::Private::configReverseSortEntry("ReverseSort");
const QString SetupMisc::Private::configApplicationStyleEntry("Application Style");

// --------------------------------------------------------

SetupMisc::SetupMisc(QWidget* const parent)
    : QScrollArea(parent), d(new Private)
{
    QWidget* panel      = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* layout = new QVBoxLayout(panel);

    // -- Misc Options --------------------------------------------------------

    QGroupBox* miscOptionsGroup = new QGroupBox(i18n("Behavior"), panel);
    QVBoxLayout* gLayout5       = new QVBoxLayout();

    d->useTrash         = new QCheckBox(i18n("&Deleted items should go to the trash"), miscOptionsGroup);
    d->showSplash       = new QCheckBox(i18n("&Show splash screen at startup"), miscOptionsGroup);

    KHBox* tabStyleHbox = new KHBox(miscOptionsGroup);
    d->sidebarTypeLabel = new QLabel(i18n("Sidebar tab title:"), tabStyleHbox);
    d->sidebarType      = new KComboBox(tabStyleHbox);
    d->sidebarType->addItem(i18n("Only For Active Tab"), 0);
    d->sidebarType->addItem(i18n("For All Tabs"),        1);
    d->sidebarType->setToolTip(i18n("Set this option to configure how sidebars tab title are visible."));

    KHBox* appStyleHbox      = new KHBox(miscOptionsGroup);
    d->applicationStyleLabel = new QLabel(i18n("Widget style:"), appStyleHbox);
    d->applicationStyle      = new KComboBox(appStyleHbox);
    d->applicationStyle->setToolTip(i18n("Set this option to choose the default window decoration and looks."));

    QStringList styleList = QStyleFactory::keys();
    for (int i = 0; i < styleList.count(); ++i)
        d->applicationStyle->addItem(styleList.at(i));

    gLayout5->addWidget(d->useTrash);
    gLayout5->addWidget(d->showSplash);
    gLayout5->addWidget(tabStyleHbox);
    gLayout5->addWidget(appStyleHbox);
    miscOptionsGroup->setLayout(gLayout5);

    // -- Sort Order Options --------------------------------------------------------

    QGroupBox* sortOptionsGroup = new QGroupBox(i18n("Sort order for images"), panel);
    QVBoxLayout* gLayout4       = new QVBoxLayout();

    KHBox* sortBox       = new KHBox(sortOptionsGroup);
    new QLabel(i18n("Sort images by:"), sortBox);
    d->sortOrderComboBox = new KComboBox(sortBox);
    d->sortOrderComboBox->insertItem(SortByDate,     i18nc("sort images by date", "Date"));
    d->sortOrderComboBox->insertItem(SortByName,     i18nc("sort images by name", "Name"));
    d->sortOrderComboBox->insertItem(SortByFileSize, i18nc("sort images by size", "File Size"));
    d->sortOrderComboBox->setWhatsThis(i18n("Here, select whether newly-loaded "
                                            "images are sorted by their date, name, or size on disk."));

    d->sortReverse = new QCheckBox(i18n("Reverse ordering"), sortOptionsGroup);
    d->sortReverse->setWhatsThis(i18n("If this option is enabled, newly-loaded "
                                      "images will be sorted in descending order."));

    gLayout4->addWidget(sortBox);
    gLayout4->addWidget(d->sortReverse);
    sortOptionsGroup->setLayout(gLayout4);

    // --------------------------------------------------------

    layout->addWidget(miscOptionsGroup);
    layout->addWidget(sortOptionsGroup);
    layout->addStretch();
    layout->setSpacing(KDialog::spacingHint());
    layout->setMargin(KDialog::spacingHint());

    // --------------------------------------------------------

    readSettings();

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);
}

SetupMisc::~SetupMisc()
{
    delete d;
}

void SetupMisc::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->useTrash->setChecked(group.readEntry(d->configDeleteItem2TrashEntry,          false));
    d->showSplash->setChecked(group.readEntry(d->configShowSplashEntry,              true));
    d->sidebarType->setCurrentIndex(group.readEntry(d->configSidebarTitleStyleEntry, 0));
    d->sortOrderComboBox->setCurrentIndex(group.readEntry(d->configSortOrderEntry,   (int)SortByDate));
    d->sortReverse->setChecked(group.readEntry(d->configReverseSortEntry,            false));
    d->applicationStyle->setCurrentIndex(d->applicationStyle->
        findText(group.readEntry(d->configApplicationStyleEntry, kapp->style()->objectName())));
}

void SetupMisc::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configDeleteItem2TrashEntry,  d->useTrash->isChecked());
    group.writeEntry(d->configShowSplashEntry,        d->showSplash->isChecked());
    group.writeEntry(d->configSidebarTitleStyleEntry, d->sidebarType->currentIndex());
    group.writeEntry(d->configSortOrderEntry,         d->sortOrderComboBox->currentIndex());
    group.writeEntry(d->configReverseSortEntry,       d->sortReverse->isChecked());
    group.writeEntry(d->configApplicationStyleEntry,  d->applicationStyle->currentText());
    config->sync();
}

}   // namespace ShowFoto
