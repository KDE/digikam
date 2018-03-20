/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-18
 * Description : setup Metadata tab.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "showfotosetupmetadata.h"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QApplication>
#include <QStyle>
#include <QStandardPaths>
#include <QFontDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "metaengine.h"
#include "metadatapanel.h"
#include "metadatasettings.h"
#include "dactivelabel.h"

namespace ShowFoto
{

class SetupMetadata::Private
{
public:

    Private() :
        exifRotateBox(0),
        exifSetOrientationBox(0),
        tab(0),
        tagsCfgPanel(0)
    {
    }

    QCheckBox*              exifRotateBox;
    QCheckBox*              exifSetOrientationBox;

    QTabWidget*             tab;

    Digikam::MetadataPanel* tagsCfgPanel;
};

SetupMetadata::SetupMetadata(QWidget* const parent )
    : QScrollArea(parent),
      d(new Private)
{
    d->tab = new QTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QWidget* const panel          = new QWidget(d->tab);
    QVBoxLayout* const mainLayout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox* const   rotationAdvGroup  = new QGroupBox(panel);
    QGridLayout* const rotationAdvLayout = new QGridLayout(rotationAdvGroup);

    QLabel* const rotationAdvExpl  = new QLabel(i18nc("@label", "Rotate actions"));
    QLabel* const rotationAdvIcon  = new QLabel;
    rotationAdvIcon->setPixmap(QIcon::fromTheme(QLatin1String("configure")).pixmap(32));

    d->exifRotateBox         = new QCheckBox(rotationAdvGroup);
    d->exifRotateBox->setText(i18n("Show images/thumbnails &rotated according to orientation tag."));
    d->exifSetOrientationBox = new QCheckBox(rotationAdvGroup);
    d->exifSetOrientationBox->setText(i18n("Set orientation tag to normal after rotate/flip."));

    rotationAdvLayout->addWidget(rotationAdvIcon,          0, 0, 1, 1);
    rotationAdvLayout->addWidget(rotationAdvExpl,          0, 1, 1, 1);
    rotationAdvLayout->addWidget(d->exifRotateBox,         1, 0, 1, 3);
    rotationAdvLayout->addWidget(d->exifSetOrientationBox, 2, 0, 1, 3);
    rotationAdvLayout->setColumnStretch(2, 10);
    rotationAdvGroup->setLayout(rotationAdvLayout);

    // --------------------------------------------------------

    QFrame* const box       = new QFrame(panel);
    QGridLayout* const grid = new QGridLayout(box);
    box->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    Digikam::DActiveLabel* const exiv2LogoLabel = new Digikam::DActiveLabel(QUrl(QLatin1String("http://www.exiv2.org")),
                                                                            QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/logo-exiv2.png")),
                                                                            box);
    exiv2LogoLabel->setWhatsThis(i18n("Visit Exiv2 project website"));

    QLabel* const explanation = new QLabel(box);
    explanation->setOpenExternalLinks(true);
    explanation->setWordWrap(true);
    QString txt;

    txt.append(i18n("<p><a href='http://en.wikipedia.org/wiki/Exif'>EXIF</a> - "
                    "a standard used by most digital cameras today to store technical "
                    "information (like aperture and shutter speed) about an image.</p>"));

    txt.append(i18n("<p><a href='http://en.wikipedia.org/wiki/IPTC_Information_Interchange_Model'>IPTC</a> - "
                    "an older standard used in digital photography to store "
                    "photographer information in images.</p>"));

    if (Digikam::MetaEngine::supportXmp())
    {
        txt.append(i18n("<p><a href='http://en.wikipedia.org/wiki/Extensible_Metadata_Platform'>XMP</a> - "
                        "a new standard used in digital photography, designed to replace IPTC.</p>"));
    }

    explanation->setText(txt);
    explanation->setFont(QFontDatabase::systemFont(QFontDatabase::SmallestReadableFont));

    grid->addWidget(exiv2LogoLabel, 0, 0, 1, 1);
    grid->addWidget(explanation,    0, 1, 1, 2);
    grid->setColumnStretch(1, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(0);

    // --------------------------------------------------------

    mainLayout->setContentsMargins(QMargins());
    mainLayout->setSpacing(spacing);
    mainLayout->addWidget(rotationAdvGroup);
    mainLayout->addSpacing(spacing);
    mainLayout->addWidget(box);
    mainLayout->addStretch();

    d->tab->insertTab(Behavior, panel, i18n("Behavior"));

    // --------------------------------------------------------

    d->tagsCfgPanel = new Digikam::MetadataPanel(d->tab);

    // --------------------------------------------------------

    readSettings();
}

SetupMetadata::~SetupMetadata()
{
    delete d;
}

void SetupMetadata::applySettings()
{
    Digikam::MetadataSettings* const mSettings = Digikam::MetadataSettings::instance();

    if (!mSettings)
    {
        return;
    }

    Digikam::MetadataSettingsContainer set;

    set.exifRotate         = d->exifRotateBox->isChecked();
    set.exifSetOrientation = d->exifSetOrientationBox->isChecked();
    mSettings->setSettings(set);

    d->tagsCfgPanel->applySettings();
}

void SetupMetadata::readSettings()
{
    Digikam::MetadataSettings* const mSettings = Digikam::MetadataSettings::instance();

    if (!mSettings)
    {
        return;
    }

    Digikam::MetadataSettingsContainer set     = mSettings->settings();

    d->exifRotateBox->setChecked(set.exifRotate);
    d->exifSetOrientationBox->setChecked(set.exifSetOrientation);
}

void SetupMetadata::setActiveTab(MetadataTab tab)
{
    d->tab->setCurrentIndex(tab);
}

}  // namespace ShowFoto
