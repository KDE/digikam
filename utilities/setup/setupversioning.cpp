/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-06
 * Description : setup tab for image versioning 
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

// Qt includes

#include <QCheckBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>

// KDE includes

#include <KComboBox>
#include <KLocale>
#include <KHBox>
#include <KDialog>

// Local includes

#include "setupversioning.h"
#include "albumsettings.h"

namespace Digikam
{

class SetupVersioning::SetupVersioningPriv
{
public:

    SetupVersioningPriv()
    {
        formatForStoringRAWLabel = 0;
        showAllVersions          = 0;
        saveIntermediateVersions = 0;
        formatForStoringRAW      = 0;
        keepOriginalsInPlace     = 0;
    }

    QLabel*    formatForStoringRAWLabel;

    QCheckBox* showAllVersions;
    QCheckBox* saveIntermediateVersions;
    QCheckBox* keepOriginalsInPlace;

    KComboBox* formatForStoringRAW;
};

SetupVersioning::SetupVersioning(QWidget* parent)
               : QScrollArea(parent), d(new SetupVersioningPriv)
{
    QWidget* panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* layout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox* versioningOptionsGroup = new QGroupBox(i18n("Versioning Options"), panel);
    QVBoxLayout* gLayout              = new QVBoxLayout(versioningOptionsGroup);

    d->showAllVersions = new QCheckBox(i18n("Show all available versions in the main view"), versioningOptionsGroup);
    d->showAllVersions->setWhatsThis(i18n("If enabled, you will see all available versions "
                                          "in the main images view. If disabled, you'll see "
                                          "only the latest created/selected versio and you can "
                                          "switch to the other versions using the right sidebar."));

    KHBox* tabStyleHbox = new KHBox(panel);
    d->formatForStoringRAWLabel = new QLabel(i18n("Format to store versions from RAW images:"), tabStyleHbox);
    d->formatForStoringRAW      = new KComboBox(tabStyleHbox);
    d->formatForStoringRAW->addItem("JPG", 0);
    d->formatForStoringRAW->addItem("PNG", 1);
    d->formatForStoringRAW->setToolTip(i18n("Set this option to configure in what format should be new versions of RAW images stored."));

    d->saveIntermediateVersions = new QCheckBox(i18n("Create new file for every modification"),
                                            versioningOptionsGroup);
    d->saveIntermediateVersions->setWhatsThis(i18n("Set this option to automatically save each "
                                                    "modification you make in image editor "
                                                    "into new file. This will be viewed "
                                                    "as a subversion of the current image"));

    d->keepOriginalsInPlace = new QCheckBox(i18n("Keep originals and other versions in place"), versioningOptionsGroup);
    d->keepOriginalsInPlace->setWhatsThis(i18n("Set this to keep all files in their original position. "
                                               "If unset, images will be moved into subfolder 'Originals'"));
    d->keepOriginalsInPlace->setDisabled(true);

    gLayout->addWidget(tabStyleHbox);
    gLayout->addWidget(d->showAllVersions);
    gLayout->addWidget(d->saveIntermediateVersions);
    gLayout->addWidget(d->keepOriginalsInPlace);
    gLayout->setMargin(KDialog::spacingHint());
    gLayout->setSpacing(0);

    // --------------------------------------------------------

    layout->addWidget(versioningOptionsGroup);
    layout->setMargin(0);
    layout->setSpacing(KDialog::spacingHint());
    layout->addStretch();

    // --------------------------------------------------------

    readSettings();

    // --------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);
}

SetupVersioning::~SetupVersioning()
{
    delete d;
}

void SetupVersioning::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    settings->setShowAllVersions(d->showAllVersions->isChecked());
    settings->setSaveIntermediateVersions(d->saveIntermediateVersions->isChecked());
    settings->setKeepOriginalsInPlace(d->keepOriginalsInPlace->isChecked());
    settings->setFormatForStoringRAW(d->formatForStoringRAW->currentText());

    settings->saveSettings();
}

void SetupVersioning::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    d->showAllVersions->setChecked(settings->getShowAllVersions());
    d->saveIntermediateVersions->setChecked(settings->getSaveIntermediateVersions());
    d->keepOriginalsInPlace->setChecked(settings->getKeepOriginalsInPlace());
    d->formatForStoringRAW->setCurrentItem(settings->getFormatForStoringRAW());
}

} // namespace Digikam
