/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "vidslidevideopage.h"

// Qt includes

#include <QIcon>
#include <QLabel>
#include <QSpinBox>
#include <QUrl>
#include <QWidget>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "vidslidewizard.h"

namespace Digikam
{

class VidSlideVideoPage::Private
{
public:

    Private(QWizard* const dialog)
      : framesVal(0),
        typeVal(0),
        bitrateVal(0),
        stdVal(0),
        transVal(0),
        duration(0),
        wizard(0),
        settings(0)
    {
        wizard = dynamic_cast<VidSlideWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
        }
    }

    QSpinBox*         framesVal;
    QComboBox*        typeVal;
    QComboBox*        bitrateVal;
    QComboBox*        stdVal;
    QComboBox*        transVal;
    QLabel*           duration;
    VidSlideWizard*   wizard;
    VidSlideSettings* settings;
};

VidSlideVideoPage::VidSlideVideoPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    setObjectName(QLatin1String("VideoPage"));

    QWidget* const main       = new QWidget(this);

    // --------------------

    QLabel* const framesLabel = new QLabel(main);
    framesLabel->setWordWrap(false);
    framesLabel->setText(i18n("Number of Frames by Image:"));
    d->framesVal              = new QSpinBox(main);
    d->framesVal->setRange(1, 15000);
    framesLabel->setBuddy(d->framesVal);

    // --------------------

    QLabel* const stdLabel = new QLabel(main);
    stdLabel->setWordWrap(false);
    stdLabel->setText(i18n("Video Standard:"));
    d->stdVal              = new QComboBox(main);
    d->stdVal->setEditable(false);

    QMap<VidSlideSettings::VidStd, QString> map3                = VidSlideSettings::videoStdNames();
    QMap<VidSlideSettings::VidStd, QString>::const_iterator it3 = map3.constBegin();

    while (it3 != map3.constEnd())
    {
        d->stdVal->addItem(it3.value(), (int)it3.key());
        ++it3;
    }

    stdLabel->setBuddy(d->stdVal);

    // --------------------

    QLabel* const typeLabel = new QLabel(main);
    typeLabel->setWordWrap(false);
    typeLabel->setText(i18n("Video Type:"));
    d->typeVal              = new QComboBox(main);
    d->typeVal->setEditable(false);

    QMap<VidSlideSettings::VidType, QString> map                = VidSlideSettings::videoTypeNames();
    QMap<VidSlideSettings::VidType, QString>::const_iterator it = map.constBegin();

    while (it != map.constEnd())
    {
        d->typeVal->addItem(it.value(), (int)it.key());
        ++it;
    }

    typeLabel->setBuddy(d->typeVal);

    // --------------------

    QLabel* const bitrateLabel = new QLabel(main);
    bitrateLabel->setWordWrap(false);
    bitrateLabel->setText(i18n("Video Bit Rate:"));
    d->bitrateVal              = new QComboBox(main);
    d->bitrateVal->setEditable(false);

    QMap<VidSlideSettings::VidBitRate, QString> map2                = VidSlideSettings::videoBitRateNames();
    QMap<VidSlideSettings::VidBitRate, QString>::const_iterator it2 = map2.constBegin();

    while (it2 != map2.constEnd())
    {
        d->bitrateVal->addItem(it2.value(), (int)it2.key());
        ++it2;
    }

    bitrateLabel->setBuddy(d->bitrateVal);

    // --------------------

    QLabel* const transLabel = new QLabel(main);
    transLabel->setWordWrap(false);
    transLabel->setText(i18n("Transition Type:"));
    d->transVal              = new QComboBox(main);
    d->transVal->setEditable(false);

    QMap<TransitionMngr::TransType, QString> map4                = TransitionMngr::transitionNames();
    QMap<TransitionMngr::TransType, QString>::const_iterator it4 = map4.constBegin();

    while (it4 != map4.constEnd())
    {
        d->transVal->addItem(it4.value(), (int)it4.key());
        ++it4;
    }

    transLabel->setBuddy(d->transVal);

    // --------------------

    d->duration = new QLabel(main);

    // --------------------

    QGridLayout* const grid = new QGridLayout(main);
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->addWidget(framesLabel,     0, 0, 1, 1);
    grid->addWidget(d->framesVal,    0, 1, 1, 1);
    grid->addWidget(stdLabel,        1, 0, 1, 1);
    grid->addWidget(d->stdVal,       1, 1, 1, 1);
    grid->addWidget(typeLabel,       2, 0, 1, 1);
    grid->addWidget(d->typeVal,      2, 1, 1, 1);
    grid->addWidget(bitrateLabel,    3, 0, 1, 1);
    grid->addWidget(d->bitrateVal,   3, 1, 1, 1);
    grid->addWidget(transLabel,      4, 0, 1, 1);
    grid->addWidget(d->transVal,     4, 1, 1, 1);
    grid->addWidget(d->duration,     5, 0, 1, 2);
    grid->setRowStretch(6, 10);

    setPageWidget(main);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("video-mp4")));

    // --------------------

    connect(d->framesVal, SIGNAL(valueChanged(int)),
            this, SLOT(slotSlideDuration()));

    connect(d->stdVal, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotSlideDuration()));
}

VidSlideVideoPage::~VidSlideVideoPage()
{
    delete d;
}

void VidSlideVideoPage::slotSlideDuration()
{
    VidSlideSettings tmp;
    tmp.aframes   = d->framesVal->value();
    tmp.vStandard = (VidSlideSettings::VidStd)d->stdVal->currentIndex();
    qreal titem   = tmp.aframes / tmp.videoFrameRate();
    qreal ttotal  = titem * d->settings->inputImages.count();
    d->duration->setText(i18n("Duration : %1 seconds by item, total %2 seconds", titem, ttotal));
}

void VidSlideVideoPage::initializePage()
{
    d->framesVal->setValue(d->settings->aframes);
    d->typeVal->setCurrentIndex(d->settings->outputType);
    d->bitrateVal->setCurrentIndex(d->settings->vbitRate);
    d->stdVal->setCurrentIndex(d->settings->vStandard);
    d->transVal->setCurrentIndex(d->settings->transition);
    slotSlideDuration();
}

bool VidSlideVideoPage::validatePage()
{
    d->settings->aframes    = d->framesVal->value();
    d->settings->outputType = (VidSlideSettings::VidType)d->typeVal->currentIndex();
    d->settings->vbitRate   = (VidSlideSettings::VidBitRate)d->bitrateVal->currentIndex();
    d->settings->vStandard  = (VidSlideSettings::VidStd)d->stdVal->currentIndex();
    d->settings->transition = (TransitionMngr::TransType)d->transVal->currentIndex();

    return true;
}

} // namespace Digikam
