/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2007-08-02
 * Description : save PNG image options.
 *
 * Copyright 2007 by Gilles Caulier
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
#include <qlayout.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kdialog.h>
#include <knuminput.h>

// Local includes.

#include "pngsettings.h"
#include "pngsettings.moc"

namespace Digikam
{

class PNGSettingsPriv
{

public:

    PNGSettingsPriv()
    {
        PNGGrid             = 0;
        labelPNGcompression = 0;
        PNGcompression      = 0;
    }

    QGridLayout  *PNGGrid;

    QLabel       *labelPNGcompression;

    KIntNumInput *PNGcompression;
};

PNGSettings::PNGSettings(QWidget *parent)
           : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new PNGSettingsPriv;

    d->PNGGrid        = new QGridLayout(this, 1, 1, KDialog::spacingHint());
    d->PNGcompression = new KIntNumInput(9, this);
    d->PNGcompression->setRange(1, 9, 1, true );
    d->labelPNGcompression = new QLabel(i18n("PNG compression:"), this);

    QWhatsThis::add(d->PNGcompression, i18n("<p>The compression value for PNG images:<p>"
                                            "<b>1</b>: low compression (large file size but "
                                            "short compression duration - default)<p>"
                                            "<b>5</b>: medium compression<p>"
                                            "<b>9</b>: high compression (small file size but "
                                            "long compression duration)<p>"
                                            "<b>Note: PNG is always a lossless image "
                                            "compression format.</b>"));
    d->PNGGrid->addMultiCellWidget(d->labelPNGcompression, 0, 0, 0, 0);
    d->PNGGrid->addMultiCellWidget(d->PNGcompression, 0, 0, 1, 1);
    d->PNGGrid->setColStretch(1, 10);
}

PNGSettings::~PNGSettings()
{
    delete d;
}

void PNGSettings::setCompressionValue(int val)
{
    d->PNGcompression->setValue(val);
}

int PNGSettings::getCompressionValue()
{
    return d->PNGcompression->value();
}

}  // namespace Digikam
