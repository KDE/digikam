/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-02
 * Description : setup Misc tab.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2008 by Arnd Baecker <arnd dot baecker at web dot de>
 * Copyright (C)      2014 by Mohamed Anwer <m dot anwer at gmx dot com>
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
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "dlayoutbox.h"
#include "showfotosettings.h"

using namespace Digikam;

namespace ShowFoto
{

class SetupMisc::Private
{
public:

    Private() :
        sidebarTypeLabel(0),
        applicationStyleLabel(0),
        iconThemeLabel(0),
        showSplash(0),
        nativeFileDialog(0),
        itemCenter(0),
        showMimeOverImage(0),
        showCoordinates(0),
        sortReverse(0),
        sidebarType(0),
        sortOrderComboBox(0),
        applicationStyle(0),
        iconTheme(0),
        settings(ShowfotoSettings::instance())
    {
    }

    QLabel*              sidebarTypeLabel;
    QLabel*              applicationStyleLabel;
    QLabel*              iconThemeLabel;

    QCheckBox*           showSplash;
    QCheckBox*           nativeFileDialog;
    QCheckBox*           itemCenter;
    QCheckBox*           showMimeOverImage;
    QCheckBox*           showCoordinates;
    QCheckBox*           sortReverse;

    QComboBox*           sidebarType;
    QComboBox*           sortOrderComboBox;
    QComboBox*           applicationStyle;
    QComboBox*           iconTheme;

    ShowfotoSettings*    settings;
};

// --------------------------------------------------------

SetupMisc::SetupMisc(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    QWidget* const panel      = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    const int spacing         = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);
    QVBoxLayout* const layout = new QVBoxLayout(panel);

    // -- Sort Order Options --------------------------------------------------------

    QGroupBox* const sortOptionsGroup = new QGroupBox(i18n("Images Sort Order"), panel);
    QVBoxLayout* const gLayout4       = new QVBoxLayout();

    DHBox* const sortBox = new DHBox(sortOptionsGroup);
    new QLabel(i18n("Sort images by:"), sortBox);
    d->sortOrderComboBox = new QComboBox(sortBox);
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

    // Thumbnails Options ----------------------------------------------------------------------

    QGroupBox* const thOptionsGroup = new QGroupBox(i18n("Thumbnails"), panel);
    QVBoxLayout* const gLayout3     = new QVBoxLayout();

    d->showMimeOverImage = new QCheckBox(i18n("&Show image Format"),          thOptionsGroup);
    d->showMimeOverImage->setWhatsThis(i18n("Set this option to show image format over image thumbnail."));
    d->showCoordinates   = new QCheckBox(i18n("&Show Geolocation Indicator"), thOptionsGroup);
    d->showCoordinates->setWhatsThis(i18n("Set this option to indicate if image has geolocation information."));

    gLayout3->addWidget(d->showMimeOverImage);
    gLayout3->addWidget(d->showCoordinates);
    thOptionsGroup->setLayout(gLayout3);

    // -- Application Behavior Options --------------------------------------------------------

    QGroupBox* const abOptionsGroup = new QGroupBox(i18n("Application Behavior"), panel);
    QVBoxLayout* const gLayout5     = new QVBoxLayout();

    d->showSplash       = new QCheckBox(i18n("&Show splash screen at startup"),            abOptionsGroup);
    d->nativeFileDialog = new QCheckBox(i18n("Use file dialogs from the system"),          abOptionsGroup);
    d->itemCenter       = new QCheckBox(i18n("Scroll current item to center of thumbbar"), abOptionsGroup);

    DHBox* const tabStyleHbox = new DHBox(abOptionsGroup);
    d->sidebarTypeLabel       = new QLabel(i18n("Sidebar tab title:"), tabStyleHbox);
    d->sidebarType            = new QComboBox(tabStyleHbox);
    d->sidebarType->addItem(i18n("Only For Active Tab"), 0);
    d->sidebarType->addItem(i18n("For All Tabs"),        1);
    d->sidebarType->setToolTip(i18n("Set this option to configure how sidebars tab title are visible."));

    DHBox* const appStyleHbox = new DHBox(abOptionsGroup);
    d->applicationStyleLabel  = new QLabel(i18n("Widget style:"), appStyleHbox);
    d->applicationStyle       = new QComboBox(appStyleHbox);
    d->applicationStyle->setToolTip(i18n("Set this option to choose the default window decoration and looks."));

    QStringList styleList     = QStyleFactory::keys();

    for (int i = 0; i < styleList.count(); i++)
    {
        d->applicationStyle->addItem(styleList.at(i));
    }

#ifndef HAVE_APPSTYLE_SUPPORT
    // See Bug #365262
    appStyleHbox->setVisible(false);
#endif

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

    gLayout5->addWidget(d->showSplash);
    gLayout5->addWidget(d->nativeFileDialog);
    gLayout5->addWidget(d->itemCenter);
    gLayout5->addWidget(tabStyleHbox);
    gLayout5->addWidget(appStyleHbox);
    gLayout5->addWidget(iconThemeHbox);
    abOptionsGroup->setLayout(gLayout5);

    // --------------------------------------------------------

    layout->addWidget(sortOptionsGroup);
    layout->addWidget(thOptionsGroup);
    layout->addWidget(abOptionsGroup);
    layout->addStretch();
    layout->setContentsMargins(spacing, spacing, spacing, spacing);
    layout->setSpacing(spacing);

    // --------------------------------------------------------

    readSettings();
}

SetupMisc::~SetupMisc()
{
    delete d;
}

void SetupMisc::readSettings()
{
    d->showSplash->setChecked(d->settings->getShowSplash());
    d->nativeFileDialog->setChecked(d->settings->getNativeFileDialog());
    d->itemCenter->setChecked(d->settings->getItemCenter());
    d->showMimeOverImage->setChecked(d->settings->getShowFormatOverThumbnail());
    d->showCoordinates->setChecked(d->settings->getShowCoordinates());
    d->sidebarType->setCurrentIndex(d->settings->getRightSideBarStyle());
    d->sortOrderComboBox->setCurrentIndex(d->settings->getSortRole());
    d->sortReverse->setChecked(d->settings->getReverseSort());
#ifdef HAVE_APPSTYLE_SUPPORT
    d->applicationStyle->setCurrentIndex(d->applicationStyle->findText(d->settings->getApplicationStyle(), Qt::MatchFixedString));
#endif
    d->iconTheme->setCurrentIndex(d->iconTheme->findData(d->settings->getIconTheme()));
}

void SetupMisc::applySettings()
{
    d->settings->setShowSplash(d->showSplash->isChecked());
    d->settings->setNativeFileDialog(d->nativeFileDialog->isChecked());
    d->settings->setItemCenter(d->itemCenter->isChecked());
    d->settings->setShowFormatOverThumbnail(d->showMimeOverImage->isChecked());
    d->settings->setShowCoordinates(d->showCoordinates->isChecked());
    d->settings->setRightSideBarStyle(d->sidebarType->currentIndex());
    d->settings->setSortRole(d->sortOrderComboBox->currentIndex());
    d->settings->setReverseSort(d->sortReverse->isChecked());
#ifdef HAVE_APPSTYLE_SUPPORT
    d->settings->setApplicationStyle(d->applicationStyle->currentText());
#endif
    d->settings->setIconTheme(d->iconTheme->currentData().toString());
    d->settings->syncConfig();
}

} // namespace ShowFoto
