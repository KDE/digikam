/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : save JPEG image options.
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
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qcombobox.h>

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
        labelSubSampling     = 0;
        subSamplingCB        = 0;
    }

    QGridLayout  *JPEGGrid;

    QLabel       *labelJPEGcompression;
    QLabel       *labelSubSampling;

    QComboBox    *subSamplingCB;

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

    QWhatsThis::add(d->JPEGcompression, i18n("<p>The JPEG image quality:<p>"
                                             "<b>1</b>: low quality (high compression and small "
                                             "file size)<p>"
                                             "<b>50</b>: medium quality<p>"
                                             "<b>75</b>: good quality (default)<p>"
                                             "<b>100</b>: high quality (no compression and "
                                             "large file size)<p>"
                                             "<b>Note: JPEG always uses lossy compression.</b>"));

    d->labelWarning = new KActiveLabel(i18n("<qt><font size=-1 color=\"red\"><i>"
                          "Warning: <a href='http://en.wikipedia.org/wiki/JPEG'>JPEG</a> is a<br>"
                          "lossy compression<br>"
                          "image format!</p>"
                          "</i></qt>"), this);

    d->labelWarning->setFrameStyle(QFrame::Box | QFrame::Plain);
    d->labelWarning->setLineWidth(1);
    d->labelWarning->setFrameShape(QFrame::Box);

    d->labelSubSampling = new QLabel(i18n("Chroma subsampling:"), this);

    d->subSamplingCB = new QComboBox(false, this);
    d->subSamplingCB->insertItem(i18n("None"));    // 1x1, 1x1, 1x1 (4:4:4)
    d->subSamplingCB->insertItem(i18n("Medium"));  // 2x1, 1x1, 1x1 (4:2:2)
    d->subSamplingCB->insertItem(i18n("High"));    // 2x2, 1x1, 1x1 (4:1:1)
    QWhatsThis::add(d->subSamplingCB, i18n("<p>JPEG Chroma subsampling level \n(color is saved with less resolution "                                           "than luminance):<p>"
                                           "<b>None</b>=best: uses 4:4:4 ratio. Does not employ chroma "
                                           "subsampling at all. This preserves edges and contrasting "
                                           "colors, whilst adding no additional compression<p>"
                                           "<b>Medium</b>: uses 4:2:2 ratio. Medium compression: reduces "
                                           "the color resolution by one-third with little to "
                                           "no visual difference<p>"
                                           "<b>High</b>: use 4:1:1 ratio. High compression: suits "
                                           "images with soft edges but tends to alter colors<p>"
                                           "<b>Note: JPEG always uses lossy compression.</b>"));

    d->JPEGGrid->addMultiCellWidget(d->labelJPEGcompression, 0, 0, 0, 0);
    d->JPEGGrid->addMultiCellWidget(d->JPEGcompression,      0, 0, 1, 1);
    d->JPEGGrid->addMultiCellWidget(d->labelSubSampling,     1, 1, 0, 0);    
    d->JPEGGrid->addMultiCellWidget(d->subSamplingCB,        1, 1, 1, 1);    
    d->JPEGGrid->addMultiCellWidget(d->labelWarning,         0, 1, 2, 2);    
    d->JPEGGrid->setColStretch(1, 10);
    d->JPEGGrid->setRowStretch(2, 10);
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

void JPEGSettings::setSubSamplingValue(int val)
{
    d->subSamplingCB->setCurrentItem(val);
}

int JPEGSettings::getSubSamplingValue()
{
    return d->subSamplingCB->currentItem();
}

}  // namespace Digikam
