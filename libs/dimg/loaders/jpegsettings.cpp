/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : save JPEG image options.
 *
 * Copyright (C) 2007-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "jpegsettings.moc"

// Qt includes

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QString>

// KDE includes

#include <kcombobox.h>
#include <kdialog.h>
#include <klocale.h>
#include <knuminput.h>

namespace Digikam
{

class JPEGSettings::Private
{

public:

    Private() :
        JPEGGrid(0),
        labelJPEGcompression(0),
        labelWarning(0),
        labelSubSampling(0),
        subSamplingCB(0),
        JPEGcompression(0)
    {
    }

    QGridLayout*  JPEGGrid;

    QLabel*       labelJPEGcompression;
    QLabel*       labelWarning;
    QLabel*       labelSubSampling;

    KComboBox*    subSamplingCB;

    KIntNumInput* JPEGcompression;
};

JPEGSettings::JPEGSettings(QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    d->JPEGGrid             = new QGridLayout(this);
    d->JPEGcompression      = new KIntNumInput(75, this);
    d->JPEGcompression->setRange(1, 100);
    d->JPEGcompression->setSliderEnabled(true);
    d->labelJPEGcompression = new QLabel(i18n("JPEG quality:"), this);

    d->JPEGcompression->setWhatsThis(i18n("<p>The JPEG quality:</p>"
                                          "<p><b>1</b>: low quality (high compression and small "
                                          "file size)<br/>"
                                          "<b>50</b>: medium quality<br/>"
                                          "<b>75</b>: good quality (default)<br/>"
                                          "<b>100</b>: high quality (no compression and "
                                          "large file size)</p>"
                                          "<p><b>Note: JPEG always uses lossy compression.</b></p>"));

    d->labelWarning = new QLabel(i18n("<font size='-1' color='red'><i>"
                                      "Warning: <a href='http://en.wikipedia.org/wiki/JPEG'>JPEG</a> is a "
                                      "lossy image compression format."
                                      "</i></font>"), this);

    d->labelWarning->setOpenExternalLinks(true);
    d->labelWarning->setFrameStyle(QFrame::Box | QFrame::Plain);
    d->labelWarning->setLineWidth(1);
    d->labelWarning->setFrameShape(QFrame::Box);

    d->labelSubSampling = new QLabel(i18n("Chroma subsampling:"), this);

    d->subSamplingCB = new KComboBox(this);
    d->subSamplingCB->insertItem(0, i18n("None"));    // 1x1, 1x1, 1x1 (4:4:4)
    d->subSamplingCB->insertItem(1, i18n("Medium"));  // 2x1, 1x1, 1x1 (4:2:2)
    d->subSamplingCB->insertItem(2, i18n("High"));    // 2x2, 1x1, 1x1 (4:1:1)
    d->subSamplingCB->setWhatsThis(i18n("<p>JPEG Chroma subsampling level \n(color is saved with less resolution "
                                        "than luminance):</p>"
                                        "<p><b>None</b>=best: uses 4:4:4 ratio. Does not employ chroma "
                                        "subsampling at all. This preserves edges and contrasting "
                                        "colors, whilst adding no additional compression</p>"
                                        "<p><b>Medium</b>: uses 4:2:2 ratio. Medium compression: reduces "
                                        "the color resolution by one-third with little to "
                                        "no visual difference</p>"
                                        "<p><b>High</b>: use 4:1:1 ratio. High compression: suits "
                                        "images with soft edges but tends to alter colors</p>"
                                        "<p><b>Note: JPEG always uses lossy compression.</b></p>"));

    d->JPEGGrid->addWidget(d->labelJPEGcompression, 0, 0, 1, 2);
    d->JPEGGrid->addWidget(d->JPEGcompression,      1, 0, 1, 2);
    d->JPEGGrid->addWidget(d->labelSubSampling,     2, 0, 1, 2);
    d->JPEGGrid->addWidget(d->subSamplingCB,        3, 0, 1, 2);
    d->JPEGGrid->addWidget(d->labelWarning,         4, 0, 1, 1);
    d->JPEGGrid->setColumnStretch(1, 10);
    d->JPEGGrid->setRowStretch(5, 10);
    d->JPEGGrid->setMargin(KDialog::spacingHint());
    d->JPEGGrid->setSpacing(KDialog::spacingHint());

    connect(d->JPEGcompression, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->subSamplingCB, SIGNAL(activated(int)),
            this, SIGNAL(signalSettingsChanged()));
}

JPEGSettings::~JPEGSettings()
{
    delete d;
}

void JPEGSettings::setCompressionValue(int val)
{
    d->JPEGcompression->setValue(val);
}

int JPEGSettings::getCompressionValue() const
{
    return d->JPEGcompression->value();
}

void JPEGSettings::setSubSamplingValue(int val)
{
    d->subSamplingCB->setCurrentIndex(val);
}

int JPEGSettings::getSubSamplingValue() const
{
    return d->subSamplingCB->currentIndex();
}

int JPEGSettings::convertCompressionForLibJpeg(int value)
{
    // JPEG quality slider settings : 1 - 100 ==> libjpeg settings : 25 - 100.

    return((int)((75.0 / 100.0) * (float)value + 26.0 - (75.0 / 100.0)));
}

}  // namespace Digikam
