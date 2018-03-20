/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : save TIFF image options.
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

#include "tiffsettings.h"

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

namespace Digikam
{

class TIFFSettings::Private
{

public:

    Private()
    {
        TIFFGrid        = 0;
        TIFFcompression = 0;
    }

    QGridLayout* TIFFGrid;

    QCheckBox*   TIFFcompression;
};

TIFFSettings::TIFFSettings(QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->TIFFGrid        = new QGridLayout(this);
    d->TIFFcompression = new QCheckBox(i18n("Compress TIFF files"), this);

    d->TIFFcompression->setWhatsThis(i18n("<p>Toggle compression for TIFF images.</p>"
                                          "<p>If this option is enabled, the final size "
                                          "of the TIFF image is reduced.</p>"
                                          "<p>A lossless compression format (Deflate) "
                                          "is used to save the file.</p>"));

    d->TIFFGrid->addWidget(d->TIFFcompression, 0, 0, 1, 2);
    d->TIFFGrid->setColumnStretch(1, 10);
    d->TIFFGrid->setRowStretch(1, 10);
    d->TIFFGrid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->TIFFGrid->setSpacing(spacing);

    connect(d->TIFFcompression, SIGNAL(toggled(bool)),
            this, SIGNAL(signalSettingsChanged()));
}

TIFFSettings::~TIFFSettings()
{
    delete d;
}

void TIFFSettings::setCompression(bool b)
{
    d->TIFFcompression->setChecked(b);
}

bool TIFFSettings::getCompression() const
{
    return d->TIFFcompression->isChecked();
}

}  // namespace Digikam
