/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2007-08-02
 * Description : save JPEG image options.
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
#include <kactivelabel.h>

// Local includes.

#include "jpegsettings.h"
#include "jpegsettings.moc"

namespace Digikam
{

class JPEGSettingsPriv
{

public:

    JPEGSettingsPriv()
    {
        JPEGGrid             = 0;
        labelJPEGcompression = 0;
        JPEGcompression      = 0;
        labelWarning         = 0;
    }

    QGridLayout  *JPEGGrid;

    QLabel       *labelJPEGcompression;

    KActiveLabel *labelWarning;

    KIntNumInput *JPEGcompression;
};

JPEGSettings::JPEGSettings(QWidget *parent)
            : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new JPEGSettingsPriv;

    d->JPEGGrid        = new QGridLayout(this, 1, 2, KDialog::spacingHint());
    d->JPEGcompression = new KIntNumInput(75, this);
    d->JPEGcompression->setRange(1, 100, 1, true );
    d->labelJPEGcompression = new QLabel(i18n("JPEG quality:"), this);

    QWhatsThis::add(d->JPEGcompression, i18n("<p>The quality value for JPEG images:<p>"
                                             "<b>1</b>: low quality (high compression and small "
                                             "file size)<p>"
                                             "<b>50</b>: medium quality<p>"
                                             "<b>75</b>: good quality (default)<p>"
                                             "<b>100</b>: high quality (no compression and "
                                             "large file size)<p>"
                                             "<b>Note: JPEG use a lossy compression image algorithm.</b>"));

    d->labelWarning = new KActiveLabel(i18n("<qt><font size=-1 color=\"red\"><i>"
                          "Warning: <a href='http://en.wikipedia.org/wiki/JPEG'>JPEG</a> is a<br>"
                          "lossy compression<br>"
                          "image format!</p>"
                          "</i></qt>"), this);

    d->labelWarning->setFrameStyle(QFrame::Box | QFrame::Plain);
    d->labelWarning->setLineWidth(1);
    d->labelWarning->setFrameShape(QFrame::Box);

    d->JPEGGrid->addMultiCellWidget(d->labelJPEGcompression, 0, 0, 0, 0);
    d->JPEGGrid->addMultiCellWidget(d->JPEGcompression, 0, 0, 1, 1);
    d->JPEGGrid->addMultiCellWidget(d->labelWarning, 0, 0, 2, 2);    
    d->JPEGGrid->setColStretch(1, 10);
    d->JPEGGrid->setRowStretch(1, 10);
}

JPEGSettings::~JPEGSettings()
{
    delete d;
}

void JPEGSettings::setCompressionValue(int val)
{
    d->JPEGcompression->setValue(val);
}

int JPEGSettings::getCompressionValue()
{
    return d->JPEGcompression->value();
}

}  // namespace Digikam

