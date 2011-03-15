/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-23
 * Description : setup image editor output files settings.
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupiofiles.moc"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>

// Local includes

#include "jp2ksettings.h"
#include "jpegsettings.h"
#include "pgfsettings.h"
#include "pngsettings.h"
#include "tiffsettings.h"

namespace Digikam
{

class SetupIOFiles::SetupIOFilesPriv
{
public:

    SetupIOFilesPriv() :
        JPEGOptions(0),
        PNGOptions(0),
        TIFFOptions(0),
        JPEG2000Options(0),
        PGFOptions(0)

#ifdef _WIN32
        ,
        showImageSettingsDialog(0)
#endif
    {}

    static const QString configGroupName;
    static const QString configJPEGCompressionEntry;
    static const QString configJPEGSubSamplingEntry;
    static const QString configPNGCompressionEntry;
    static const QString configTIFFCompressionEntry;
    static const QString configJPEG2000CompressionEntry;
    static const QString configJPEG2000LossLessEntry;
    static const QString configPGFCompressionEntry;
    static const QString configPGFLossLessEntry;

    JPEGSettings* JPEGOptions;
    PNGSettings*  PNGOptions;
    TIFFSettings* TIFFOptions;
    JP2KSettings* JPEG2000Options;
    PGFSettings*  PGFOptions;

#ifdef _WIN32
    static const QString configShowImageSettingsDialog;
    QCheckBox*           showImageSettingsDialog;
#endif
};

const QString SetupIOFiles::SetupIOFilesPriv::configGroupName("ImageViewer Settings");
const QString SetupIOFiles::SetupIOFilesPriv::configJPEGCompressionEntry("JPEGCompression");
const QString SetupIOFiles::SetupIOFilesPriv::configJPEGSubSamplingEntry("JPEGSubSampling");
const QString SetupIOFiles::SetupIOFilesPriv::configPNGCompressionEntry("PNGCompression");
const QString SetupIOFiles::SetupIOFilesPriv::configTIFFCompressionEntry("TIFFCompression");
const QString SetupIOFiles::SetupIOFilesPriv::configJPEG2000CompressionEntry("JPEG2000Compression");
const QString SetupIOFiles::SetupIOFilesPriv::configJPEG2000LossLessEntry("JPEG2000LossLess");
const QString SetupIOFiles::SetupIOFilesPriv::configPGFCompressionEntry("PGFCompression");
const QString SetupIOFiles::SetupIOFilesPriv::configPGFLossLessEntry("PGFLossLess");

#ifdef _WIN32
const QString SetupIOFilesPriv::configShowImageSettingsDialog("ShowImageSettingsDialog");
#endif

// --------------------------------------------------------

static QWidget* createGroupBox(QWidget* w)
{
    QGroupBox*   box    = new QGroupBox;
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(w);
    box->setLayout(layout);
    return box;
}

SetupIOFiles::SetupIOFiles(QWidget* parent )
    : QScrollArea(parent), d(new SetupIOFilesPriv)
{
    QWidget* panel     = new QWidget;
    QVBoxLayout* vbox  = new QVBoxLayout;
    d->JPEGOptions     = new JPEGSettings;
    d->PNGOptions      = new PNGSettings;
    d->TIFFOptions     = new TIFFSettings;
    d->JPEG2000Options = new JP2KSettings;
    d->PGFOptions      = new PGFSettings;

#ifdef _WIN32
    // Show Settings Dialog Option

    d->showImageSettingsDialog = new QCheckBox(panel);
    d->showImageSettingsDialog->setText(i18n("Show Settings Dialog when Saving Image Files"));
    d->showImageSettingsDialog->setWhatsThis( i18n("<ul><li>Checked: A dialog where settings can be changed when saving image files</li>"
                                                   "<li>Unchecked: Default settings are used when saving image files</li></ul>"));
#endif

    vbox->addWidget(createGroupBox(d->JPEGOptions));
    vbox->addWidget(createGroupBox(d->PNGOptions));
    vbox->addWidget(createGroupBox(d->TIFFOptions));
    vbox->addWidget(createGroupBox(d->JPEG2000Options));
    vbox->addWidget(createGroupBox(d->PGFOptions));
#ifdef _WIN32
    vbox->addWidget(createGroupBox(d->showImageSettingsDialog));
#endif
    vbox->addStretch();

    panel->setLayout(vbox);
    setWidget(panel);
    setWidgetResizable(true);

    // --------------------------------------------------------

    readSettings();

    // --------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);
}

SetupIOFiles::~SetupIOFiles()
{
    delete d;
}

void SetupIOFiles::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configJPEGCompressionEntry,     d->JPEGOptions->getCompressionValue());
    group.writeEntry(d->configJPEGSubSamplingEntry,     d->JPEGOptions->getSubSamplingValue());
    group.writeEntry(d->configPNGCompressionEntry,      d->PNGOptions->getCompressionValue());
    group.writeEntry(d->configTIFFCompressionEntry,     d->TIFFOptions->getCompression());
    group.writeEntry(d->configJPEG2000CompressionEntry, d->JPEG2000Options->getCompressionValue());
    group.writeEntry(d->configJPEG2000LossLessEntry,    d->JPEG2000Options->getLossLessCompression());
    group.writeEntry(d->configPGFCompressionEntry,      d->PGFOptions->getCompressionValue());
    group.writeEntry(d->configPGFLossLessEntry,         d->PGFOptions->getLossLessCompression());
#ifdef _WIN32
    group.writeEntry(d->configShowImageSettingsDialog,  d->showImageSettingsDialog->isChecked());
#endif
    config->sync();
}

void SetupIOFiles::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->JPEGOptions->setCompressionValue(group.readEntry(d->configJPEGCompressionEntry,          75) );
    d->JPEGOptions->setSubSamplingValue(group.readEntry(d->configJPEGSubSamplingEntry,          1) ); // Medium sub-sampling
    d->PNGOptions->setCompressionValue(group.readEntry(d->configPNGCompressionEntry,            9) );
    d->TIFFOptions->setCompression(group.readEntry(d->configTIFFCompressionEntry,               false));
    d->JPEG2000Options->setCompressionValue( group.readEntry(d->configJPEG2000CompressionEntry, 75) );
    d->JPEG2000Options->setLossLessCompression( group.readEntry(d->configJPEG2000LossLessEntry, true) );
    d->PGFOptions->setCompressionValue( group.readEntry(d->configPGFCompressionEntry,           3) );
    d->PGFOptions->setLossLessCompression( group.readEntry(d->configPGFLossLessEntry,           true) );
#ifdef _WIN32
    d->showImageSettingsDialog->setChecked( group.readEntry(d->configShowImageSettingsDialog,   true) );
#endif
}

}  // namespace Digikam
