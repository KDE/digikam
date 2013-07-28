/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 
 * Description :
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
#include <klocale.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kicon.h>
#include <kurllabel.h>
#include <kurlrequester.h>

// Local includes

#include "picklabelwidget.h"

namespace Digikam
{

class SetupImageQualitySorter::Private
{
public:
    Private() :
        enableSorter(0),
        detectBlur(0),
        detectNoise(0),
        detectCompression(0),
        setRejected(0),
        setPending(0),
        setAccepted(0),
        setSpeed(0)
    {}

    QCheckBox*    enableSorter;
    QCheckBox*    detectBlur;
    QCheckBox*    detectNoise;
    QCheckBox*    detectCompression;
    QCheckBox*    setRejected;
    QCheckBox*    setPending;
    QCheckBox*    setAccepted;

    KIntNumInput* setSpeed;
};

// --------------------------------------------------------

SetupImageQualitySorter::SetupImageQualitySorter(QWidget* const parent)
    : QScrollArea(parent), d(new Private)
{
    QWidget* panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QVBoxLayout* layout = new QVBoxLayout(panel);

    d->enableSorter = new QCheckBox(i18n("Enable Image Quality Sorting"), panel);
    d->enableSorter->setWhatsThis(i18n("If this option is enabled, the selected images will be selected based on quality."));

    d->setSpeed = new KIntNumInput(5, panel);
    d->setSpeed->setRange(1, 3, 1);
    d->setSpeed->setSliderEnabled(true);
    d->setSpeed->setLabel(i18n("Speed"), Qt::AlignLeft | Qt::AlignTop);
    d->setSpeed->setWhatsThis(i18n("Tradeoff between speed and accuracy of sorting algorithm"));

    d->detectBlur = new QCheckBox(i18n("Detect Blur"), panel);
    d->detectBlur->setWhatsThis(i18n("Detect the amount of blur in the images passed to it"));

    d->detectNoise = new QCheckBox(i18n("Detect Noise"), panel);
    d->detectNoise->setWhatsThis(i18n("Detect the amount of noise in the images passed to it"));

    d->detectCompression = new QCheckBox(i18n("Detect Compression"), panel);
    d->detectCompression->setWhatsThis(i18n("Detect the amount of compression in the images passed to it"));

    QLabel* workIcon1     = new QLabel;
    workIcon1->setPixmap(SmallIcon("flag-red"));

    QLabel* workIcon2     = new QLabel;
    workIcon2->setPixmap(SmallIcon("flag-yellow"));

    QLabel* workIcon3     = new QLabel;
    workIcon3->setPixmap(SmallIcon("flag-green"));

    d->setRejected = new QCheckBox(i18n("Assign 'Rejected' Label to Low Quality Pictures"), panel);
    d->setRejected->setWhatsThis(i18n("Show the image caption at the bottom of the screen."));

    d->setPending = new QCheckBox(i18n("Assign 'Pending' Label to Medium Quality Pictures"), panel);
    d->setPending->setWhatsThis(i18n("Show the image title at the bottom of the screen."));

    d->setAccepted = new QCheckBox(i18n("Assign 'Accepted' Label to High Quality Pictures"), panel);
    d->setAccepted->setWhatsThis(i18n("Show the image caption at the bottom of the screen if no titles existed."));

    layout->addWidget(d->enableSorter);
    layout->addWidget(d->setSpeed);
    layout->addWidget(d->detectBlur);
    layout->addWidget(d->detectNoise);
    layout->addWidget(d->detectCompression);
  
    QHBoxLayout* layouth1  = new QHBoxLayout;
    layouth1->addWidget(d->setRejected);
    layouth1->addWidget(workIcon1);

    QHBoxLayout* layouth2  = new QHBoxLayout;

    layouth2->addWidget(d->setPending);
    layouth2->addWidget(workIcon2);

    QHBoxLayout* layouth3  = new QHBoxLayout;

    layouth3->addWidget(d->setAccepted);
    layouth3->addWidget(workIcon3);

    layout->addLayout(layouth1);
    layout->addLayout(layouth2);
    layout->addLayout(layouth3);
    layout->addStretch();
    layout->setMargin(KDialog::spacingHint());
    layout->setSpacing(KDialog::spacingHint());

    readSettings();

    // --------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);
}

SetupImageQualitySorter::~SetupImageQualitySorter()
{
    delete d;
}

void SetupImageQualitySorter::slotSetUnchecked(int)
{
}

void SetupImageQualitySorter::applySettings()
{
}

void SetupImageQualitySorter::readSettings()
{

}

}   // namespace Digikam
