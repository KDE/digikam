/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : Image Quality setup page
 *
 * Copyright (C) 2013-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "setupimagequalitysorter.moc"

// Qt includes

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kvbox.h>
#include <klocale.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kicon.h>
#include <kurllabel.h>
#include <kurlrequester.h>

// Local includes

#include "picklabelwidget.h"
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
    {}

    KVBox*        optionsView;
    QCheckBox*    enableSorter;
    QCheckBox*    useFullImage;
    QCheckBox*    detectBlur;
    QCheckBox*    detectNoise;
    QCheckBox*    detectCompression;
    QCheckBox*    detectOverexposure;
    QCheckBox*    setRejected;
    QCheckBox*    setPending;
    QCheckBox*    setAccepted;

    KIntNumInput* setSpeed;
    KIntNumInput* setRejectedThreshold;
    KIntNumInput* setPendingThreshold;
    KIntNumInput* setAcceptedThreshold;
    KIntNumInput* setBlurWeight;
    KIntNumInput* setNoiseWeight;
    KIntNumInput* setCompressionWeight;
};

// --------------------------------------------------------

SetupImageQualitySorter::SetupImageQualitySorter(QWidget* const parent)
    : QScrollArea(parent), d(new Private)
{
    QWidget* const panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* const layout = new QVBoxLayout(panel);

    d->enableSorter = new QCheckBox(i18n("Enable Image Quality Sorting (Experimental)"), panel);
    d->enableSorter->setWhatsThis(i18n("Enable this option to assign automatically Pick Labels based on image quality."));

    d->optionsView  = new KVBox(panel);

    layout->addWidget(d->enableSorter);
    layout->addWidget(d->optionsView);

    // ------------------------------------------------------------------------------

    d->setSpeed    = new KIntNumInput(5, d->optionsView);
    d->setSpeed->setRange(1, 3, 1);
    d->setSpeed->setSliderEnabled(true);
    d->setSpeed->setLabel(i18n("Speed:"), Qt::AlignLeft | Qt::AlignTop);
    d->setSpeed->setWhatsThis(i18n("Tradeoff between speed and accuracy of sorting algorithm"));

    d->detectBlur  = new QCheckBox(i18n("Detect Blur"), d->optionsView);
    d->detectBlur->setWhatsThis(i18n("Detect the amount of blur in the images passed to it"));

    d->detectNoise = new QCheckBox(i18n("Detect Noise"), d->optionsView);
    d->detectNoise->setWhatsThis(i18n("Detect the amount of noise in the images passed to it"));

    d->detectCompression  = new QCheckBox(i18n("Detect Compression"), d->optionsView);
    d->detectCompression->setWhatsThis(i18n("Detect the amount of compression in the images passed to it"));

    d->detectOverexposure = new QCheckBox(i18n("Detect Overexposure"), d->optionsView);
    d->detectOverexposure->setWhatsThis(i18n("Detect if the images are overexposed"));

    // ------------------------------------------------------------------------------

    KHBox* const hlay1      = new KHBox(d->optionsView);

    d->setRejected          = new QCheckBox(i18n("Assign 'Rejected' Label to Low Quality Pictures"), hlay1);
    d->setRejected->setWhatsThis(i18n("Low quality images detected by blur, noise, and compression analysis will be assigned to Rejected label."));

    QWidget* const hspace1  = new QWidget(hlay1);
    hlay1->setStretchFactor(hspace1, 10);

    QLabel* const workIcon1 = new QLabel(hlay1);
    workIcon1->setPixmap(SmallIcon("flag-red"));

    // ------------------------------------------------------------------------------

    KHBox* const hlay2      = new KHBox(d->optionsView);

    d->setPending           = new QCheckBox(i18n("Assign 'Pending' Label to Medium Quality Pictures"), hlay2);
    d->setPending->setWhatsThis(i18n("Medium quality images detected by blur, noise, and compression analysis will be assigned to Pending label."));

    QWidget* const hspace2  = new QWidget(hlay2);
    hlay2->setStretchFactor(hspace2, 10);

    QLabel* const workIcon2 = new QLabel(hlay2);
    workIcon2->setPixmap(SmallIcon("flag-yellow"));

    // ------------------------------------------------------------------------------

    KHBox* const hlay3      = new KHBox(d->optionsView);

    d->setAccepted          = new QCheckBox(i18n("Assign 'Accepted' Label to High Quality Pictures"), hlay3);
    d->setAccepted->setWhatsThis(i18n("High quality images detected by blur, noise, and compression analysis will be assigned to Accepted label."));

    QWidget* const hspace3  = new QWidget(hlay3);
    hlay3->setStretchFactor(hspace3, 10);

    QLabel* const workIcon3 = new QLabel(hlay3);
    workIcon3->setPixmap(SmallIcon("flag-green"));

    // ------------------------------------------------------------------------------

    d->setRejectedThreshold = new KIntNumInput(5, d->optionsView);
    d->setRejectedThreshold->setRange(1, 100, 1);
    d->setRejectedThreshold->setSliderEnabled(true);
    d->setRejectedThreshold->setLabel(i18n("Rejected threshold:"), Qt::AlignLeft | Qt::AlignTop);
    d->setRejectedThreshold->setWhatsThis(i18n("Threshold below which all pictures are assigned Rejected Label"));

    d->setPendingThreshold  = new KIntNumInput(5, d->optionsView);
    d->setPendingThreshold->setRange(1, 100, 1);
    d->setPendingThreshold->setSliderEnabled(true);
    d->setPendingThreshold->setLabel(i18n("Pending threshold:"), Qt::AlignLeft | Qt::AlignTop);
    d->setPendingThreshold->setWhatsThis(i18n("Threshold below which all pictures are assigned Pending Label"));

    d->setAcceptedThreshold = new KIntNumInput(5, d->optionsView);
    d->setAcceptedThreshold->setRange(1, 100, 1);
    d->setAcceptedThreshold->setSliderEnabled(true);
    d->setAcceptedThreshold->setLabel(i18n("Accepted threshold:"), Qt::AlignLeft | Qt::AlignTop);
    d->setAcceptedThreshold->setWhatsThis(i18n("Threshold above which all pictures are assigned Accepted Label"));

    d->setBlurWeight        = new KIntNumInput(5, d->optionsView);
    d->setBlurWeight->setRange(1, 100, 1);
    d->setBlurWeight->setSliderEnabled(true);
    d->setBlurWeight->setLabel(i18n("Blur Weight:"), Qt::AlignLeft | Qt::AlignTop);
    d->setBlurWeight->setWhatsThis(i18n("Weight to assign to Blur Algorithm"));

    d->setNoiseWeight       = new KIntNumInput(5, d->optionsView);
    d->setNoiseWeight->setRange(1, 100, 1);
    d->setNoiseWeight->setSliderEnabled(true);
    d->setNoiseWeight->setLabel(i18n("Noise Weight:"), Qt::AlignLeft | Qt::AlignTop);
    d->setNoiseWeight->setWhatsThis(i18n("Weight to assign to Noise Algorithm"));

    d->setCompressionWeight = new KIntNumInput(5, d->optionsView);
    d->setCompressionWeight->setRange(1, 100, 1);
    d->setCompressionWeight->setSliderEnabled(true);
    d->setCompressionWeight->setLabel(i18n("Compression Weight:"), Qt::AlignLeft | Qt::AlignTop);
    d->setCompressionWeight->setWhatsThis(i18n("Weight to assign to Compression Algorithm"));

    QWidget* const vspace   = new QWidget(d->optionsView);
    d->optionsView->setStretchFactor(vspace, 10);

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
