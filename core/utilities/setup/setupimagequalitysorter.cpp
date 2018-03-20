/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : Image Quality setup page
 *
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
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

#include "setupimagequalitysorter.h"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QIcon>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "picklabelwidget.h"
#include "dnuminput.h"
#include "imagequalitysettings.h"

namespace Digikam
{

class SetupImageQualitySorter::Private
{
public:

    Private() :
        optionsView(0),
        enableSorter(0),
        useFullImage(0),
        detectBlur(0),
        detectNoise(0),
        detectCompression(0),
        detectOverexposure(0),
        setRejected(0),
        setPending(0),
        setAccepted(0),
        setSpeed(0),
        setRejectedThreshold(0),
        setPendingThreshold(0),
        setAcceptedThreshold(0),
        setBlurWeight(0),
        setNoiseWeight(0),
        setCompressionWeight(0)
    {
    }

    DVBox*        optionsView;
    QCheckBox*    enableSorter;
    QCheckBox*    useFullImage;
    QCheckBox*    detectBlur;
    QCheckBox*    detectNoise;
    QCheckBox*    detectCompression;
    QCheckBox*    detectOverexposure;
    QCheckBox*    setRejected;
    QCheckBox*    setPending;
    QCheckBox*    setAccepted;

    DIntNumInput* setSpeed;
    DIntNumInput* setRejectedThreshold;
    DIntNumInput* setPendingThreshold;
    DIntNumInput* setAcceptedThreshold;
    DIntNumInput* setBlurWeight;
    DIntNumInput* setNoiseWeight;
    DIntNumInput* setCompressionWeight;
};

// --------------------------------------------------------

SetupImageQualitySorter::SetupImageQualitySorter(QWidget* const parent)
    : QScrollArea(parent),
      d(new Private)
{
    QWidget* const panel      = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* const layout = new QVBoxLayout(panel);

    d->enableSorter = new QCheckBox(i18n("Enable Image Quality Sorting (Experimental)"), panel);
    d->enableSorter->setWhatsThis(i18n("Enable this option to assign automatically Pick Labels based on image quality."));

    d->optionsView  = new DVBox(panel);

    layout->addWidget(d->enableSorter);
    layout->addWidget(d->optionsView);

    // ------------------------------------------------------------------------------

    d->detectBlur  = new QCheckBox(i18n("Detect Blur"), d->optionsView);
    d->detectBlur->setWhatsThis(i18n("Detect the amount of blur in the images passed to it"));

    d->detectNoise = new QCheckBox(i18n("Detect Noise"), d->optionsView);
    d->detectNoise->setWhatsThis(i18n("Detect the amount of noise in the images passed to it"));

    d->detectCompression  = new QCheckBox(i18n("Detect Compression"), d->optionsView);
    d->detectCompression->setWhatsThis(i18n("Detect the amount of compression in the images passed to it"));

    d->detectOverexposure = new QCheckBox(i18n("Detect Overexposure"), d->optionsView);
    d->detectOverexposure->setWhatsThis(i18n("Detect if the images are overexposed"));

    // ------------------------------------------------------------------------------

    DHBox* const hlay1      = new DHBox(d->optionsView);

    d->setRejected          = new QCheckBox(i18n("Assign 'Rejected' Label to Low Quality Pictures"), hlay1);
    d->setRejected->setWhatsThis(i18n("Low quality images detected by blur, noise, and compression analysis will be assigned to Rejected label."));

    QWidget* const hspace1  = new QWidget(hlay1);
    hlay1->setStretchFactor(hspace1, 10);

    QLabel* const workIcon1 = new QLabel(hlay1);
    workIcon1->setPixmap(QIcon::fromTheme(QLatin1String("flag-red")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));

    // ------------------------------------------------------------------------------

    DHBox* const hlay2      = new DHBox(d->optionsView);

    d->setPending           = new QCheckBox(i18n("Assign 'Pending' Label to Medium Quality Pictures"), hlay2);
    d->setPending->setWhatsThis(i18n("Medium quality images detected by blur, noise, and compression analysis will be assigned to Pending label."));

    QWidget* const hspace2  = new QWidget(hlay2);
    hlay2->setStretchFactor(hspace2, 10);

    QLabel* const workIcon2 = new QLabel(hlay2);
    workIcon2->setPixmap(QIcon::fromTheme(QLatin1String("flag-yellow")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));

    // ------------------------------------------------------------------------------

    DHBox* const hlay3      = new DHBox(d->optionsView);

    d->setAccepted          = new QCheckBox(i18n("Assign 'Accepted' Label to High Quality Pictures"), hlay3);
    d->setAccepted->setWhatsThis(i18n("High quality images detected by blur, noise, and compression analysis will be assigned to Accepted label."));

    QWidget* const hspace3  = new QWidget(hlay3);
    hlay3->setStretchFactor(hspace3, 10);

    QLabel* const workIcon3 = new QLabel(hlay3);
    workIcon3->setPixmap(QIcon::fromTheme(QLatin1String("flag-green")).pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize)));

    // ------------------------------------------------------------------------------

    QWidget* const settings = new QWidget(d->optionsView);
    QGridLayout* const glay = new QGridLayout(settings);

    QLabel* const lbl1 = new QLabel(i18n("Speed:"), settings);
    lbl1->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->setSpeed        = new DIntNumInput(settings);
    d->setSpeed->setDefaultValue(5);
    d->setSpeed->setRange(1, 3, 1);
    d->setSpeed->setWhatsThis(i18n("Tradeoff between speed and accuracy of sorting algorithm"));

    QLabel* const lbl2 = new QLabel(i18n("Rejected threshold:"), settings);
    lbl2->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->setRejectedThreshold = new DIntNumInput(settings);
    d->setRejectedThreshold->setDefaultValue(5);
    d->setRejectedThreshold->setRange(1, 100, 1);
    d->setRejectedThreshold->setWhatsThis(i18n("Threshold below which all pictures are assigned Rejected Label"));

    QLabel* const lbl3 = new QLabel(i18n("Pending threshold:"), settings);
    lbl3->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->setPendingThreshold  = new DIntNumInput(settings);
    d->setPendingThreshold->setDefaultValue(5);
    d->setPendingThreshold->setRange(1, 100, 1);
    d->setPendingThreshold->setWhatsThis(i18n("Threshold below which all pictures are assigned Pending Label"));

    QLabel* const lbl4 = new QLabel(i18n("Accepted threshold:"), settings);
    lbl4->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->setAcceptedThreshold = new DIntNumInput(settings);
    d->setAcceptedThreshold->setDefaultValue(5);
    d->setAcceptedThreshold->setRange(1, 100, 1);
    d->setAcceptedThreshold->setWhatsThis(i18n("Threshold above which all pictures are assigned Accepted Label"));

    QLabel* const lbl5 = new QLabel(i18n("Blur Weight:"), settings);
    lbl5->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->setBlurWeight        = new DIntNumInput(settings);
    d->setBlurWeight->setDefaultValue(5);
    d->setBlurWeight->setRange(1, 100, 1);
    d->setBlurWeight->setWhatsThis(i18n("Weight to assign to Blur Algorithm"));

    QLabel* const lbl6 = new QLabel(i18n("Noise Weight:"), settings);
    lbl6->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->setNoiseWeight       = new DIntNumInput(settings);
    d->setNoiseWeight->setDefaultValue(5);
    d->setNoiseWeight->setRange(1, 100, 1);
    d->setNoiseWeight->setWhatsThis(i18n("Weight to assign to Noise Algorithm"));

    QLabel* const lbl7 = new QLabel(i18n("Compression Weight:"), settings);
    lbl7->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->setCompressionWeight = new DIntNumInput(settings);
    d->setCompressionWeight->setDefaultValue(5);
    d->setCompressionWeight->setRange(1, 100, 1);
    d->setCompressionWeight->setWhatsThis(i18n("Weight to assign to Compression Algorithm"));

    glay->addWidget(lbl1,                     0, 0, 1, 1);
    glay->addWidget(d->setSpeed,              0, 1, 1, 1);
    glay->addWidget(lbl2,                     1, 0, 1, 1);
    glay->addWidget(d->setRejectedThreshold,  1, 1, 1, 1);
    glay->addWidget(lbl3,                     2, 0, 1, 1);
    glay->addWidget(d->setPendingThreshold,   2, 1, 1, 1);
    glay->addWidget(lbl4,                     3, 0, 1, 1);
    glay->addWidget(d->setAcceptedThreshold,  3, 1, 1, 1);
    glay->addWidget(lbl5,                     4, 0, 1, 1);
    glay->addWidget(d->setBlurWeight,         4, 1, 1, 1);
    glay->addWidget(lbl6,                     5, 0, 1, 1);
    glay->addWidget(d->setNoiseWeight,        5, 1, 1, 1);
    glay->addWidget(lbl7,                     6, 0, 1, 1);
    glay->addWidget(d->setCompressionWeight,  6, 1, 1, 1);
    glay->setRowStretch(7, 10);

    connect(d->enableSorter, SIGNAL(toggled(bool)),
            d->optionsView, SLOT(setEnabled(bool)));

    readSettings();
}

SetupImageQualitySorter::~SetupImageQualitySorter()
{
    delete d;
}

void SetupImageQualitySorter::applySettings()
{
    ImageQualitySettings imq;

    imq.enableSorter      = d->enableSorter->isChecked();
    imq.speed             = d->setSpeed->value();
    imq.detectBlur        = d->detectBlur->isChecked();
    imq.detectNoise       = d->detectNoise->isChecked();
    imq.detectCompression = d->detectCompression->isChecked();
    imq.detectOverexposure= d->detectOverexposure->isChecked();
    imq.lowQRejected      = d->setRejected->isChecked();
    imq.mediumQPending    = d->setPending->isChecked();
    imq.highQAccepted     = d->setAccepted->isChecked();
    imq.rejectedThreshold = d->setRejectedThreshold->value();
    imq.pendingThreshold  = d->setPendingThreshold->value();
    imq.acceptedThreshold = d->setAcceptedThreshold->value();
    imq.blurWeight        = d->setBlurWeight->value();
    imq.noiseWeight       = d->setNoiseWeight->value();
    imq.compressionWeight = d->setCompressionWeight->value();

    imq.writeToConfig();
}

void SetupImageQualitySorter::readSettings()
{
    ImageQualitySettings imq;
    imq.readFromConfig();

    d->enableSorter->setChecked(imq.enableSorter);
    d->setSpeed->setValue(imq.speed);
    d->detectBlur->setChecked(imq.detectBlur);
    d->detectNoise->setChecked(imq.detectNoise);
    d->detectCompression->setChecked(imq.detectCompression);
    d->detectOverexposure->setChecked(imq.detectOverexposure);
    d->setRejected->setChecked(imq.lowQRejected);
    d->setPending->setChecked(imq.mediumQPending);
    d->setAccepted->setChecked(imq.highQAccepted);
    d->setRejectedThreshold->setValue(imq.rejectedThreshold);
    d->setPendingThreshold->setValue(imq.pendingThreshold);
    d->setAcceptedThreshold->setValue(imq.acceptedThreshold);
    d->setBlurWeight->setValue(imq.blurWeight);
    d->setNoiseWeight->setValue(imq.noiseWeight);
    d->setCompressionWeight->setValue(imq.compressionWeight);

    d->optionsView->setEnabled(imq.enableSorter);
}

}   // namespace Digikam
