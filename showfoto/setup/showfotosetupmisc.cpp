/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-02
 * Description : setup Misc tab.
 *
 * Copyright (C) 2005-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2008 by Arnd Baecker <arnd dot baecker at web dot de>
 * Copyright (C)      2014 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#include "showfotosetupmisc.h"

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

//Local includes

#include "showfotosettings.h"

namespace ShowFoto
{

class SetupMisc::Private
{
public:

    Private() :
        sidebarTypeLabel(0),
        applicationStyleLabel(0),
        showSplash(0),
        showMimeOverImage(0),
        showCoordinates(0),
        sortReverse(0),
        useTrash(0),
        sidebarType(0),
        sortOrderComboBox(0),
        applicationStyle(0),
        settings(0)
    {}

    QLabel*              sidebarTypeLabel;
    QLabel*              applicationStyleLabel;

    QCheckBox*           showSplash;
    QCheckBox*           showMimeOverImage;
    QCheckBox*           showCoordinates;
    QCheckBox*           sortReverse;
    QCheckBox*           useTrash;

    KComboBox*           sidebarType;
    KComboBox*           sortOrderComboBox;
    KComboBox*           applicationStyle;

    ShowfotoSettings*    settings;
};

// --------------------------------------------------------

SetupMisc::SetupMisc(QWidget* const parent)
    : QScrollArea(parent), d(new Private)
{
    QWidget* const panel      = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* const layout = new QVBoxLayout(panel);

    // -- Misc Options --------------------------------------------------------

    QGroupBox* const miscOptionsGroup = new QGroupBox(i18n("Behavior"), panel);
    QVBoxLayout* const gLayout5       = new QVBoxLayout();

    d->useTrash          = new QCheckBox(i18n("&Deleted items should go to the trash"), miscOptionsGroup);
    d->showSplash        = new QCheckBox(i18n("&Show splash screen at startup"), miscOptionsGroup);
    d->showMimeOverImage = new QCheckBox(i18n("&Show image Format"), miscOptionsGroup);
    d->showMimeOverImage->setWhatsThis(i18n("Set this option to show image format over image thumbnail."));
    d->showCoordinates   = new QCheckBox(i18n("&Show Geolocation Indicator"), miscOptionsGroup);
    d->showCoordinates->setWhatsThis(i18n("Set this option to indicate if image has geolocation information."));

    KHBox* const tabStyleHbox = new KHBox(miscOptionsGroup);
    d->sidebarTypeLabel       = new QLabel(i18n("Sidebar tab title:"), tabStyleHbox);
    d->sidebarType            = new KComboBox(tabStyleHbox);
    d->sidebarType->addItem(i18n("Only For Active Tab"), 0);
    d->sidebarType->addItem(i18n("For All Tabs"),        1);
    d->sidebarType->setToolTip(i18n("Set this option to configure how sidebars tab title are visible."));

    KHBox* const appStyleHbox = new KHBox(miscOptionsGroup);
    d->applicationStyleLabel  = new QLabel(i18n("Widget style:"), appStyleHbox);
    d->applicationStyle       = new KComboBox(appStyleHbox);
    d->applicationStyle->setToolTip(i18n("Set this option to choose the default window decoration and looks."));

    QStringList styleList = QStyleFactory::keys();

    for (int i = 0; i < styleList.count(); ++i)
    {
        d->applicationStyle->addItem(styleList.at(i));
    }

    gLayout5->addWidget(d->useTrash);
    gLayout5->addWidget(d->showSplash);
    gLayout5->addWidget(d->showMimeOverImage);
    gLayout5->addWidget(d->showCoordinates);
    gLayout5->addWidget(tabStyleHbox);
    gLayout5->addWidget(appStyleHbox);
    miscOptionsGroup->setLayout(gLayout5);

    // -- Sort Order Options --------------------------------------------------------

    QGroupBox* const sortOptionsGroup = new QGroupBox(i18n("Sort order for images"), panel);
    QVBoxLayout* const gLayout4       = new QVBoxLayout();

    KHBox* const sortBox = new KHBox(sortOptionsGroup);
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
}

SetupMisc::~SetupMisc()
{
    delete d;
}

void SetupMisc::readSettings()
{
    d->settings = ShowfotoSettings::instance();

    d->useTrash->setChecked(d->settings->getDeleteItem2Trash());
    d->showSplash->setChecked(d->settings->getShowSplash());
    d->showMimeOverImage->setChecked(d->settings->getShowFormatOverThumbnail());
    d->showCoordinates->setChecked(d->settings->getShowCoordinates());
    d->sidebarType->setCurrentIndex(d->settings->getRightSideBarStyle());
    d->sortOrderComboBox->setCurrentIndex(d->settings->getSortRole());
    d->sortReverse->setChecked(d->settings->getReverseSort());
    d->applicationStyle->setCurrentIndex(d->applicationStyle->findText(d->settings->getApplicationStyle()));
}

void SetupMisc::applySettings()
{
    d->settings->setDeleteItem2Trash(d->useTrash->isChecked());
    d->settings->setShowSplash(d->showSplash->isChecked());
    d->settings->setShowFormatOverThumbnail(d->showMimeOverImage->isChecked());
    d->settings->setShowCoordinates(d->showCoordinates->isChecked());
    d->settings->setRightSideBarStyle(d->sidebarType->currentIndex());
    d->settings->setSortRole(d->sortOrderComboBox->currentIndex());
    d->settings->setReverseSort(d->sortReverse->isChecked());
    d->settings->setApplicationStyle(d->applicationStyle->currentText());
    d->settings->syncConfig();
}

}   // namespace ShowFoto
