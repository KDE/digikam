/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-23
 * Description : setup image editor output files settings.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupiofiles.h"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "jpegsettings.h"
#include "pgfsettings.h"
#include "pngsettings.h"
#include "tiffsettings.h"

#ifdef HAVE_JASPER
#   include "jp2ksettings.h"
#endif // HAVE_JASPER

namespace Digikam
{

class SetupIOFiles::Private
{
public:

    Private() :
        JPEGOptions(0),
        PNGOptions(0),
        TIFFOptions(0),
#ifdef HAVE_JASPER
        JPEG2000Options(0),
#endif // HAVE_JASPER
        PGFOptions(0),
        showImageSettingsDialog(0)
    {
    }

    QWidget* createGroupBox(QWidget* const w) const
    {
        QGroupBox* const  box     = new QGroupBox;
        QVBoxLayout* const layout = new QVBoxLayout;
        layout->addWidget(w);
        box->setLayout(layout);
        return box;
    }

public:

    static const QString configGroupName;
    static const QString configJPEGCompressionEntry;
    static const QString configJPEGSubSamplingEntry;
    static const QString configPNGCompressionEntry;
    static const QString configTIFFCompressionEntry;
    static const QString configJPEG2000CompressionEntry;
    static const QString configJPEG2000LossLessEntry;
    static const QString configPGFCompressionEntry;
    static const QString configPGFLossLessEntry;
    static const QString configShowImageSettingsDialog;

    JPEGSettings*        JPEGOptions;
    PNGSettings*         PNGOptions;
    TIFFSettings*        TIFFOptions;
#ifdef HAVE_JASPER
    JP2KSettings*        JPEG2000Options;
#endif // HAVE_JASPER
    PGFSettings*         PGFOptions;

    QCheckBox*           showImageSettingsDialog;
};

const QString SetupIOFiles::Private::configGroupName(QLatin1String("ImageViewer Settings"));
const QString SetupIOFiles::Private::configJPEGCompressionEntry(QLatin1String("JPEGCompression"));
const QString SetupIOFiles::Private::configJPEGSubSamplingEntry(QLatin1String("JPEGSubSampling"));
const QString SetupIOFiles::Private::configPNGCompressionEntry(QLatin1String("PNGCompression"));
const QString SetupIOFiles::Private::configTIFFCompressionEntry(QLatin1String("TIFFCompression"));
const QString SetupIOFiles::Private::configJPEG2000CompressionEntry(QLatin1String("JPEG2000Compression"));
const QString SetupIOFiles::Private::configJPEG2000LossLessEntry(QLatin1String("JPEG2000LossLess"));
const QString SetupIOFiles::Private::configPGFCompressionEntry(QLatin1String("PGFCompression"));
const QString SetupIOFiles::Private::configPGFLossLessEntry(QLatin1String("PGFLossLess"));
const QString SetupIOFiles::Private::configShowImageSettingsDialog(QLatin1String("ShowImageSettingsDialog"));

// --------------------------------------------------------

SetupIOFiles::SetupIOFiles(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    QWidget* const panel    = new QWidget;
    QVBoxLayout* const vbox = new QVBoxLayout;
    d->JPEGOptions          = new JPEGSettings;
    d->PNGOptions           = new PNGSettings;
    d->TIFFOptions          = new TIFFSettings;
#ifdef HAVE_JASPER
    d->JPEG2000Options      = new JP2KSettings;
#endif // HAVE_JASPER
    d->PGFOptions           = new PGFSettings;

    // Show Settings Dialog Option

    d->showImageSettingsDialog = new QCheckBox(panel);
    d->showImageSettingsDialog->setText(i18n("Show Settings Dialog when Saving Image Files"));
    d->showImageSettingsDialog->setWhatsThis(i18n("<ul><li>Checked: A dialog where settings can be changed when saving image files</li>"
                                                  "<li>Unchecked: Default settings are used when saving image files</li></ul>"));

    vbox->addWidget(d->createGroupBox(d->JPEGOptions));
    vbox->addWidget(d->createGroupBox(d->PNGOptions));
    vbox->addWidget(d->createGroupBox(d->TIFFOptions));
#ifdef HAVE_JASPER
    vbox->addWidget(d->createGroupBox(d->JPEG2000Options));
#endif // HAVE_JASPER
    vbox->addWidget(d->createGroupBox(d->PGFOptions));
    vbox->addWidget(d->createGroupBox(d->showImageSettingsDialog));
    vbox->addStretch();

    panel->setLayout(vbox);
    setWidget(panel);
    setWidgetResizable(true);

    // --------------------------------------------------------

    readSettings();
}

SetupIOFiles::~SetupIOFiles()
{
    delete d;
}

void SetupIOFiles::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configJPEGCompressionEntry,     d->JPEGOptions->getCompressionValue());
    group.writeEntry(d->configJPEGSubSamplingEntry,     d->JPEGOptions->getSubSamplingValue());
    group.writeEntry(d->configPNGCompressionEntry,      d->PNGOptions->getCompressionValue());
    group.writeEntry(d->configTIFFCompressionEntry,     d->TIFFOptions->getCompression());
#ifdef HAVE_JASPER
    group.writeEntry(d->configJPEG2000CompressionEntry, d->JPEG2000Options->getCompressionValue());
    group.writeEntry(d->configJPEG2000LossLessEntry,    d->JPEG2000Options->getLossLessCompression());
#endif // HAVE_JASPER
    group.writeEntry(d->configPGFCompressionEntry,      d->PGFOptions->getCompressionValue());
    group.writeEntry(d->configPGFLossLessEntry,         d->PGFOptions->getLossLessCompression());
    group.writeEntry(d->configShowImageSettingsDialog,  d->showImageSettingsDialog->isChecked());
    config->sync();
}

void SetupIOFiles::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->JPEGOptions->setCompressionValue(group.readEntry(d->configJPEGCompressionEntry,         75));
    d->JPEGOptions->setSubSamplingValue(group.readEntry(d->configJPEGSubSamplingEntry,         1));  // Medium sub-sampling
    d->PNGOptions->setCompressionValue(group.readEntry(d->configPNGCompressionEntry,           9));
    d->TIFFOptions->setCompression(group.readEntry(d->configTIFFCompressionEntry,              false));
#ifdef HAVE_JASPER
    d->JPEG2000Options->setCompressionValue(group.readEntry(d->configJPEG2000CompressionEntry, 75));
    d->JPEG2000Options->setLossLessCompression(group.readEntry(d->configJPEG2000LossLessEntry, true));
#endif // HAVE_JASPER
    d->PGFOptions->setCompressionValue(group.readEntry(d->configPGFCompressionEntry,           3));
    d->PGFOptions->setLossLessCompression(group.readEntry(d->configPGFLossLessEntry,           true));
    d->showImageSettingsDialog->setChecked(group.readEntry(d->configShowImageSettingsDialog,   true));
}

}  // namespace Digikam
