/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : save TIFF image options.
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qstring.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>

// Local includes.

#include "tiffsettings.h"
#include "tiffsettings.moc"

namespace Digikam
{

class TIFFSettingsPriv
{

public:

    TIFFSettingsPriv()
    {
        TIFFGrid        = 0;
        TIFFcompression = 0;
    }

    QGridLayout *TIFFGrid;
    
    QCheckBox   *TIFFcompression;
};

TIFFSettings::TIFFSettings(QWidget *parent)
            : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new TIFFSettingsPriv;

    d->TIFFGrid        = new QGridLayout(this, 1, 1, KDialog::spacingHint());
    d->TIFFcompression = new QCheckBox(i18n("Compress TIFF files"), this);

    QWhatsThis::add( d->TIFFcompression, i18n("<p>Toggle compression for TIFF images.<p>"
                                              "If you enable this option, you can reduce "
                                              "the final file size of the TIFF image.</p>"
                                              "<p>A lossless compression format (Deflate) "
                                              "is used to save the file.<p>"));
    d->TIFFGrid->addMultiCellWidget(d->TIFFcompression, 0, 0, 0, 1);
    d->TIFFGrid->setColStretch(1, 10);
}

TIFFSettings::~TIFFSettings()
{
    delete d;
}

void TIFFSettings::setCompression(bool b)
{
    d->TIFFcompression->setChecked(b);
}

bool TIFFSettings::getCompression()
{
    return d->TIFFcompression->isChecked();
}

}  // namespace Digikam

