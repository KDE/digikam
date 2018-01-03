/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-06
 * Description : save PGF image options.
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

#include "pgfsettings.h"

// Qt includes

#include <QString>
#include <QLabel>
#include <QCheckBox>
#include <QLayout>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"

namespace Digikam
{

class PGFSettings::Private
{

public:

    Private()
    {
        PGFGrid             = 0;
        labelPGFcompression = 0;
        PGFcompression      = 0;
        PGFLossLess         = 0;
    }

    QGridLayout*  PGFGrid;

    QLabel*       labelPGFcompression;

    QCheckBox*    PGFLossLess;

    DIntNumInput* PGFcompression;
};

PGFSettings::PGFSettings(QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->PGFGrid     = new QGridLayout(this);
    d->PGFLossLess = new QCheckBox(i18n("Lossless PGF files"), this);

    d->PGFLossLess->setWhatsThis(i18n("<p>Toggle lossless compression for PGF images.</p>"
                                      "<p>If this option is enabled, a lossless method will be used "
                                      "to compress PGF pictures.</p>"));

    d->PGFcompression = new DIntNumInput(this);
    d->PGFcompression->setDefaultValue(3);
    d->PGFcompression->setRange(1, 9, 1);
    d->labelPGFcompression = new QLabel(i18n("PGF quality:"), this);

    d->PGFcompression->setWhatsThis(i18n("<p>The quality value for PGF images:</p>"
                                         "<p><b>1</b>: high quality (no compression and "
                                         "large file size)<br/>"
                                         "<b>3</b>: good quality (default)<br/>"
                                         "<b>6</b>: medium quality<br/>"
                                         "<b>9</b>: low quality (high compression and small "
                                         "file size)</p>"
                                         "<p><b>Note: PGF is not a lossless image "
                                         "compression format when you use this setting.</b></p>"));

    d->PGFGrid->addWidget(d->PGFLossLess,         0, 0, 1, 2);
    d->PGFGrid->addWidget(d->labelPGFcompression, 1, 0, 1, 2);
    d->PGFGrid->addWidget(d->PGFcompression,      2, 0, 1, 2);
    d->PGFGrid->setColumnStretch(1, 10);
    d->PGFGrid->setRowStretch(3, 10);
    d->PGFGrid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->PGFGrid->setSpacing(spacing);

    connect(d->PGFLossLess, SIGNAL(toggled(bool)),
            this, SLOT(slotTogglePGFLossLess(bool)));

    connect(d->PGFLossLess, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->PGFcompression, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

PGFSettings::~PGFSettings()
{
    delete d;
}

void PGFSettings::setCompressionValue(int val)
{
    d->PGFcompression->setValue(val);
}

int PGFSettings::getCompressionValue() const
{
    return d->PGFcompression->value();
}

void PGFSettings::setLossLessCompression(bool b)
{
    d->PGFLossLess->setChecked(b);
    slotTogglePGFLossLess(d->PGFLossLess->isChecked());
}

bool PGFSettings::getLossLessCompression() const
{
    return d->PGFLossLess->isChecked();
}

void PGFSettings::slotTogglePGFLossLess(bool b)
{
    d->PGFcompression->setEnabled(!b);
    d->labelPGFcompression->setEnabled(!b);
}

}  // namespace Digikam
