/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-06
 * Description : save PGF image options.
 *
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "pgfsettings.moc"

// Qt includes

#include <QString>
#include <QLabel>
#include <QCheckBox>
#include <QLayout>
#include <QGridLayout>

// KDE includes

#include <klocale.h>
#include <kdialog.h>
#include <knuminput.h>

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

    KIntNumInput* PGFcompression;
};

PGFSettings::PGFSettings(QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    d->PGFGrid     = new QGridLayout(this);
    d->PGFLossLess = new QCheckBox(i18n("Lossless PGF files"), this);

    d->PGFLossLess->setWhatsThis(i18n("<p>Toggle lossless compression for PGF images.</p>"
                                      "<p>If this option is enabled, a lossless method will be used "
                                      "to compress PGF pictures.</p>"));

    d->PGFcompression = new KIntNumInput(3, this);
    d->PGFcompression->setRange(1, 9, 1);
    d->PGFcompression->setSliderEnabled(true);
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
    d->PGFGrid->setMargin(KDialog::spacingHint());
    d->PGFGrid->setSpacing(KDialog::spacingHint());

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
