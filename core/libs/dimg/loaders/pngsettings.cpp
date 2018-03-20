/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : save PNG image options.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "pngsettings.h"

// Qt includes

#include <QApplication>
#include <QStyle>
#include <QString>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"

namespace Digikam
{

class PNGSettings::Private
{

public:

    Private()
    {
        PNGGrid             = 0;
        labelPNGcompression = 0;
        PNGcompression      = 0;
    }

    QGridLayout*  PNGGrid;

    QLabel*       labelPNGcompression;

    DIntNumInput* PNGcompression;
};

PNGSettings::PNGSettings(QWidget* parent)
    : QWidget(parent), d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->PNGGrid        = new QGridLayout(this);
    d->PNGcompression = new DIntNumInput(this);
    d->PNGcompression->setDefaultValue(6);
    d->PNGcompression->setRange(1, 9, 1);
    d->labelPNGcompression = new QLabel(i18n("PNG compression:"), this);

    d->PNGcompression->setWhatsThis(i18n("<p>The compression value for PNG images:</p>"
                                         "<p><b>1</b>: low compression (large file size but "
                                         "short compression duration - default)<br/>"
                                         "<b>5</b>: medium compression<br/>"
                                         "<b>9</b>: high compression (small file size but "
                                         "long compression duration)</p>"
                                         "<p><b>Note: PNG is always a lossless image "
                                         "compression format.</b></p>"));

    d->PNGGrid->addWidget(d->labelPNGcompression, 0, 0, 1, 2);
    d->PNGGrid->addWidget(d->PNGcompression,      1, 1, 1, 2);
    d->PNGGrid->setColumnStretch(1, 10);
    d->PNGGrid->setRowStretch(2, 10);
    d->PNGGrid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->PNGGrid->setSpacing(spacing);

    connect(d->PNGcompression, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));
}

PNGSettings::~PNGSettings()
{
    delete d;
}

void PNGSettings::setCompressionValue(int val)
{
    d->PNGcompression->setValue(val);
}

int PNGSettings::getCompressionValue() const
{
    return d->PNGcompression->value();
}

int PNGSettings::convertCompressionForLibPng(int value)
{
    // PNG compression slider settings : 1 - 9 ==> libpng settings : 100 - 1.

    return((int)(((1.0 - 100.0) / 8.0) * (float)value + 100.0 - ((1.0 - 100.0) / 8.0)));
}

}  // namespace Digikam
