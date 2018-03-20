/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-24
 * Description : DNG save settings widgets
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "dngsettings.h"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QComboBox>
#include <QApplication>
#include <QIcon>
#include <QUrl>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dcombobox.h"
#include "dngwriter.h"

namespace Digikam
{

class DNGSettings::Private
{
public:

    Private()
    {
        compressLossLess      = 0;
        previewModeCB         = 0;
        previewModeLabel      = 0;
        backupOriginalRawFile = 0;
    }

    QLabel*    previewModeLabel;

    QCheckBox* compressLossLess;
    QCheckBox* backupOriginalRawFile;

    DComboBox* previewModeCB;
};

DNGSettings::DNGSettings(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QGridLayout* const settingsBoxLayout = new QGridLayout(this);

    // ------------------------------------------------------------------------

    d->backupOriginalRawFile = new QCheckBox(i18n("Embed Original File"), this);
    d->compressLossLess      = new QCheckBox(i18n("Lossless Compression"), this);

    // ------------------------------------------------------------------------

    d->previewModeLabel = new QLabel(i18n("JPEG Preview:"), this);
    d->previewModeCB    = new DComboBox(this);
    d->previewModeCB->insertItem(DNGWriter::NONE,     i18nc("embedded preview type in dng file", "None"));
    d->previewModeCB->insertItem(DNGWriter::MEDIUM,   i18nc("embedded preview type in dng file", "Medium"));
    d->previewModeCB->insertItem(DNGWriter::FULLSIZE, i18nc("embedded preview type in dng file", "Full size"));
    d->previewModeCB->setDefaultIndex(DNGWriter::MEDIUM);

    settingsBoxLayout->addWidget(d->backupOriginalRawFile, 0, 0, 1, 1);
    settingsBoxLayout->addWidget(d->compressLossLess,      1, 0, 1, 1);
    settingsBoxLayout->addWidget(d->previewModeLabel,      3 ,0, 1, 1);
    settingsBoxLayout->addWidget(d->previewModeCB,         4 ,0 ,1, 1);
    settingsBoxLayout->setRowStretch(9, 10);
    settingsBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    settingsBoxLayout->setSpacing(spacing);

    connect(d->backupOriginalRawFile, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->compressLossLess, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->previewModeCB, SIGNAL(activated(int)),
            this, SIGNAL(signalSettingsChanged()));
}

DNGSettings::~DNGSettings()
{
    delete d;
}

void DNGSettings::setDefaultSettings()
{
    d->previewModeCB->slotReset();
    setCompressLossLess(true);
    setBackupOriginalRawFile(false);
}

void DNGSettings::setPreviewMode(int mode)
{
    d->previewModeCB->setCurrentIndex(mode);
}

int DNGSettings::previewMode() const
{
    return d->previewModeCB->currentIndex();
}

void DNGSettings::setCompressLossLess(bool b)
{
    d->compressLossLess->setChecked(b);
}

bool DNGSettings::compressLossLess() const
{
    return d->compressLossLess->isChecked();
}

void DNGSettings::setBackupOriginalRawFile(bool b)
{
    d->backupOriginalRawFile->setChecked(b);
}

bool DNGSettings::backupOriginalRawFile() const
{
    return d->backupOriginalRawFile->isChecked();
}

} // namespace Digikam
