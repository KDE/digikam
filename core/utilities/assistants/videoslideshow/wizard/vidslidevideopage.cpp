/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QGroupBox>

// KDE includes

#include <klocalizedstring.h>

// QtAv includes

#include <QtAV/VideoEncoder.h>

// Local includes

#include "vidslidewizard.h"
#include "transitionpreview.h"
#include "effectpreview.h"

using namespace QtAV;

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
        codecVal(0),
        transVal(0),
        effVal(0),
        duration(0),
        wizard(0),
        settings(0),
        transPreview(0),
        effPreview(0)
    {
        wizard = dynamic_cast<VidSlideWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
        }
    }

    QSpinBox*          framesVal;
    QComboBox*         typeVal;
    QComboBox*         bitrateVal;
    QComboBox*         stdVal;
    QComboBox*         codecVal;
    QComboBox*         transVal;
    QComboBox*         effVal;
    QLabel*            duration;
    VidSlideWizard*    wizard;
    VidSlideSettings*  settings;
    TransitionPreview* transPreview;
    EffectPreview*     effPreview;
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

    QLabel* const codecLabel = new QLabel(main);
    codecLabel->setWordWrap(false);
    codecLabel->setText(i18n("Video Codec:"));
    d->codecVal              = new QComboBox(main);
    d->codecVal->setEditable(false);

    QMap<VidSlideSettings::VidCodec, QString> map5                = VidSlideSettings::videoCodecNames();
    QMap<VidSlideSettings::VidCodec, QString>::const_iterator it5 = map5.constBegin();

    while (it5 != map5.constEnd())
    {
        d->codecVal->insertItem((int)it5.key(), it5.value(), (int)it5.key());

        // Disable codec entry if QtAV/ffmpeg codec is not available.

        VidSlideSettings tmp;
        tmp.vCodec = (VidSlideSettings::VidCodec)it5.key();

        if (!VideoEncoder::supportedCodecs().contains(tmp.videoCodec()))
            d->codecVal->setItemData((int)it5.key(), false, Qt::UserRole-1);

        ++it5;
    }

    codecLabel->setBuddy(d->codecVal);

    // --------------------

    QGroupBox* const effGrp = new QGroupBox(i18n("Effect While Displaying Images"), main);
    QLabel* const effLabel  = new QLabel(effGrp);
    effLabel->setWordWrap(false);
    effLabel->setText(i18n("Type:"));
    d->effVal               = new QComboBox(effGrp);
    d->effVal->setEditable(false);

    QMap<EffectMngr::EffectType, QString> map6                = EffectMngr::effectNames();
    QMap<EffectMngr::EffectType, QString>::const_iterator it6 = map6.constBegin();

    while (it6 != map6.constEnd())
    {
        d->effVal->insertItem((int)it6.key(), it6.value(), (int)it6.key());
        ++it6;
    }

    effLabel->setBuddy(d->effVal);

    QLabel* const effNote  = new QLabel(effGrp);
    effNote->setWordWrap(true);
    effNote->setText(i18n("<i>An effect is an visual panning or zooming applied while an image "
                          "is displayed. The effect duration will follow the number of frames used "
                          "to render the image on video stream.</i>"));

    d->effPreview              = new EffectPreview(effGrp);
    QGridLayout* const effGrid = new QGridLayout(effGrp);
    effGrid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    effGrid->addWidget(effLabel,      0, 0, 1, 1);
    effGrid->addWidget(d->effVal,     0, 1, 1, 1);
    effGrid->addWidget(effNote,       1, 0, 1, 2);
    effGrid->addWidget(d->effPreview, 0, 2, 2, 1);
    effGrid->setColumnStretch(1, 10);
    effGrid->setRowStretch(1, 10);

    // --------------------

    QGroupBox* const transGrp = new QGroupBox(i18n("Transition Between Images"), main);
    QLabel* const transLabel  = new QLabel(transGrp);
    transLabel->setWordWrap(false);
    transLabel->setText(i18n("Type:"));
    d->transVal               = new QComboBox(transGrp);
    d->transVal->setEditable(false);

    QMap<TransitionMngr::TransType, QString> map4                = TransitionMngr::transitionNames();
    QMap<TransitionMngr::TransType, QString>::const_iterator it4 = map4.constBegin();

    while (it4 != map4.constEnd())
    {
        d->transVal->addItem(it4.value(), (int)it4.key());
        ++it4;
    }

    transLabel->setBuddy(d->transVal);

    QLabel* const transNote  = new QLabel(transGrp);
    transNote->setWordWrap(true);
    transNote->setText(i18n("<i>A transition is an visual effect applied between two images. "
                            "For some effects, the duration can depend of random values and "
                            "can change while the slideshow.</i>"));

    d->transPreview              = new TransitionPreview(transGrp);
    QGridLayout* const transGrid = new QGridLayout(transGrp);
    transGrid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    transGrid->addWidget(transLabel,      0, 0, 1, 1);
    transGrid->addWidget(d->transVal,     0, 1, 1, 1);
    transGrid->addWidget(transNote,       1, 0, 1, 2);
    transGrid->addWidget(d->transPreview, 0, 2, 2, 1);
    transGrid->setColumnStretch(1, 10);
    transGrid->setRowStretch(1, 10);

    // --------------------

    d->duration     = new QLabel(main);

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
    grid->addWidget(codecLabel,      4, 0, 1, 1);
    grid->addWidget(d->codecVal,     4, 1, 1, 1);
    grid->addWidget(effGrp,          5, 0, 1, 2);
    grid->addWidget(transGrp,        6, 0, 1, 2);
    grid->addWidget(d->duration,     7, 0, 1, 2);
    grid->setRowStretch(8, 10);

    setPageWidget(main);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("video-mp4")));

    // --------------------

    connect(d->framesVal, SIGNAL(valueChanged(int)),
            this, SLOT(slotSlideDuration()));

    connect(d->stdVal, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotSlideDuration()));

    connect(d->transVal, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotTransitionChanged()));

    connect(d->effVal, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotEffectChanged()));
}

VidSlideVideoPage::~VidSlideVideoPage()
{
    delete d;
}

void VidSlideVideoPage::slotTransitionChanged()
{
    d->transPreview->stopPreview();
    d->transPreview->startPreview((TransitionMngr::TransType)d->transVal->currentIndex());
}

void VidSlideVideoPage::slotEffectChanged()
{
    d->effPreview->stopPreview();
    d->effPreview->startPreview((EffectMngr::EffectType)d->effVal->currentIndex());
}

void VidSlideVideoPage::slotSlideDuration()
{
    VidSlideSettings tmp;
    tmp.imgFrames = d->framesVal->value();
    tmp.vStandard = (VidSlideSettings::VidStd)d->stdVal->currentIndex();
    qreal titem   = tmp.imgFrames / tmp.videoFrameRate();
    qreal ttotal  = titem * d->settings->inputImages.count();
    d->duration->setText(i18n("Duration : %1 seconds by item, total %2 seconds (without transitions)",
                              titem, ttotal));
}

void VidSlideVideoPage::initializePage()
{
    d->framesVal->setValue(d->settings->imgFrames);
    d->typeVal->setCurrentIndex(d->settings->vType);
    d->bitrateVal->setCurrentIndex(d->settings->vbitRate);
    d->stdVal->setCurrentIndex(d->settings->vStandard);
    d->codecVal->setCurrentIndex(d->codecVal->findData(d->settings->vCodec));
    d->effVal->setCurrentIndex(d->settings->vEffect);
    d->transVal->setCurrentIndex(d->settings->transition);
    d->transPreview->setImagesList(d->settings->inputImages);
    d->effPreview->setImagesList(d->settings->inputImages);
    slotSlideDuration();
}

bool VidSlideVideoPage::validatePage()
{
    d->transPreview->stopPreview();
    d->effPreview->stopPreview();
    d->settings->imgFrames  = d->framesVal->value();
    d->settings->vType      = (VidSlideSettings::VidType)d->typeVal->currentIndex();
    d->settings->vbitRate   = (VidSlideSettings::VidBitRate)d->bitrateVal->currentIndex();
    d->settings->vStandard  = (VidSlideSettings::VidStd)d->stdVal->currentIndex();
    d->settings->vCodec     = (VidSlideSettings::VidCodec)d->codecVal->currentData().toInt();
    d->settings->vEffect    = (EffectMngr::EffectType)d->effVal->currentIndex();
    d->settings->transition = (TransitionMngr::TransType)d->transVal->currentIndex();

    return true;
}

} // namespace Digikam
