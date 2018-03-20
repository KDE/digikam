/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-08-02
 * Description : save JPEG image options.
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

#include "jpegsettings.h"

// Qt includes

#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QString>
#include <QComboBox>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"

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

    QComboBox*    subSamplingCB;

    DIntNumInput* JPEGcompression;
};

JPEGSettings::JPEGSettings(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    d->JPEGGrid             = new QGridLayout(this);
    d->JPEGcompression      = new DIntNumInput(this);
    d->JPEGcompression->setDefaultValue(75);
    d->JPEGcompression->setRange(1, 100, 1);
    d->labelJPEGcompression = new QLabel(i18n("JPEG quality:"), this);

    d->JPEGcompression->setWhatsThis(i18n("<p>The JPEG quality:</p>"
                                          "<p><b>1</b>: low quality (high compression and small "
                                          "file size)<br/>"
                                          "<b>50</b>: medium quality<br/>"
                                          "<b>75</b>: good quality (default)<br/>"
                                          "<b>100</b>: high quality (no compression and "
                                          "large file size)</p>"
                                          "<p><b>Note: JPEG always uses lossy compression.</b></p>"));

    d->labelWarning = new QLabel(i18n("<font color='red'><i>"
                                      "Warning: <a href='http://en.wikipedia.org/wiki/JPEG'>JPEG</a> is a "
                                      "lossy image compression format."
                                      "</i></font>"), this);

    d->labelWarning->setOpenExternalLinks(true);
    d->labelWarning->setFrameStyle(QFrame::Box | QFrame::Plain);
    d->labelWarning->setLineWidth(1);
    d->labelWarning->setFrameShape(QFrame::Box);

    d->labelSubSampling = new QLabel(i18n("Chroma subsampling:"), this);

    d->subSamplingCB = new QComboBox(this);
    d->subSamplingCB->insertItem(0, i18n("4:4:4 (best quality)")); // 1x1, 1x1, 1x1 (4:4:4)
    d->subSamplingCB->insertItem(1, i18n("4:2:2 (good quality)")); // 2x1, 1x1, 1x1 (4:2:2)
    d->subSamplingCB->insertItem(2, i18n("4:2:0 (low quality)"));  // 2x2, 1x1, 1x1 (4:2:0)
    d->subSamplingCB->insertItem(3, i18n("4:1:1 (low quality)"));  // 4x1, 1x1, 1x1 (4:1:1)
    d->subSamplingCB->setWhatsThis(i18n("<p>Chroma subsampling reduces file size by taking advantage of the "
                                        "eye's lesser sensitivity to color resolution. How perceptible the "
                                        "difference is depends on the image - large photos will generally "
                                        "show no difference, while sharp, down-scaled pixel graphics may "
                                        "lose fine color detail.</p>"
                                        "<p><b>4:4:4</b> - No chroma subsampling, highest "
                                        "quality but lowest compression.</p>"
                                        "<p><b>4:2:2</b> - Chroma halved horizontally, average "
                                        "compression, average quality.</p>"
                                        "<p><b>4:2:0</b> - Chroma quartered in 2x2 blocks, "
                                        "high compression but low quality.</p>"
                                        "<p><b>4:1:1</b> - Chroma quartered in 4x1 blocks, "
                                        "high compression but low quality.</p>"
                                        "<p><b>Note: JPEG always uses lossy compression.</b></p>"));

    d->JPEGGrid->addWidget(d->labelJPEGcompression, 0, 0, 1, 2);
    d->JPEGGrid->addWidget(d->JPEGcompression,      1, 0, 1, 2);
    d->JPEGGrid->addWidget(d->labelSubSampling,     2, 0, 1, 2);
    d->JPEGGrid->addWidget(d->subSamplingCB,        3, 0, 1, 2);
    d->JPEGGrid->addWidget(d->labelWarning,         4, 0, 1, 1);
    d->JPEGGrid->setColumnStretch(1, 10);
    d->JPEGGrid->setRowStretch(5, 10);
    d->JPEGGrid->setContentsMargins(spacing, spacing, spacing, spacing);
    d->JPEGGrid->setSpacing(spacing);

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
