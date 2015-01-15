/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-18
 * Description : setup Metadata tab.
 *
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "showfotosetupmetadata.moc"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>
#include <kvbox.h>

// LibKExiv2 includes

#include <libkexiv2/kexiv2.h>
#include <libkexiv2/version.h>

// Local includes

#include "metadatapanel.h"
#include "metadatasettings.h"

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

    KTabWidget*             tab;

    Digikam::MetadataPanel* tagsCfgPanel;
};

SetupMetadata::SetupMetadata(QWidget* const parent )
    : QScrollArea(parent), d(new Private)
{
    d->tab = new KTabWidget(viewport());
    setWidget(d->tab);
    setWidgetResizable(true);

    QWidget* const panel          = new QWidget(d->tab);
    QVBoxLayout* const mainLayout = new QVBoxLayout(panel);

    // --------------------------------------------------------

    QGroupBox* const ExifGroup  = new QGroupBox(i18n("EXIF Actions"), panel);
    QVBoxLayout* const gLayout1 = new QVBoxLayout(ExifGroup);

    d->exifRotateBox            = new QCheckBox(ExifGroup);
    d->exifRotateBox->setText(i18n("Show images/thumbnails &rotated according to orientation tag."));

    d->exifSetOrientationBox    = new QCheckBox(ExifGroup);
    d->exifSetOrientationBox->setText(i18n("Set orientation tag to normal after rotate/flip."));

    gLayout1->addWidget(d->exifRotateBox);
    gLayout1->addWidget(d->exifSetOrientationBox);
    gLayout1->setMargin(KDialog::spacingHint());
    gLayout1->setSpacing(0);

    // --------------------------------------------------------

    QFrame* const box       = new QFrame(panel);
    QGridLayout* const grid = new QGridLayout(box);
    box->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    KUrlLabel* const exiv2LogoLabel = new KUrlLabel(box);
    exiv2LogoLabel->setText(QString());
    exiv2LogoLabel->setUrl("http://www.exiv2.org");
    exiv2LogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-exiv2.png")));
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

    if (KExiv2Iface::KExiv2::supportXmp())
    {
        txt.append(i18n("<p><a href='http://en.wikipedia.org/wiki/Extensible_Metadata_Platform'>XMP</a> - "
                        "a new standard used in digital photography, designed to replace IPTC.</p>"));
    }

    explanation->setText(txt);
    explanation->setFont(KGlobalSettings::smallestReadableFont());

    grid->addWidget(exiv2LogoLabel, 0, 0, 1, 1);
    grid->addWidget(explanation,    0, 1, 1, 2);
    grid->setColumnStretch(1, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(0);

    // --------------------------------------------------------

    mainLayout->setMargin(0);
    mainLayout->setSpacing(KDialog::spacingHint());
    mainLayout->addWidget(ExifGroup);
    mainLayout->addSpacing(KDialog::spacingHint());
    mainLayout->addWidget(box);
    mainLayout->addStretch();

    d->tab->insertTab(Behavior, panel, i18n("Behavior"));

    // --------------------------------------------------------

    d->tagsCfgPanel = new Digikam::MetadataPanel(d->tab);

    // --------------------------------------------------------

    readSettings();

    connect(exiv2LogoLabel, SIGNAL(leftClickedUrl(QString)),
            this, SLOT(slotProcessExiv2Url(QString)));

    // --------------------------------------------------------
}

SetupMetadata::~SetupMetadata()
{
    delete d;
}

void SetupMetadata::slotProcessExiv2Url(const QString& url)
{
    KToolInvocation::self()->invokeBrowser(url);
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
