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
        transVal(0),
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
    QComboBox*        transVal;
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

    QMap<TransitionMngr::TransType, QString> map3                = TransitionMngr::transitionNames();
    QMap<TransitionMngr::TransType, QString>::const_iterator it3 = map3.constBegin();

    while (it3 != map3.constEnd())
    {
        d->transVal->addItem(it3.value(), (int)it3.key());
        ++it3;
    }

    transLabel->setBuddy(d->transVal);

    // --------------------

    QGridLayout* const grid = new QGridLayout(main);
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->addWidget(framesLabel,     0, 0, 1, 1);
    grid->addWidget(d->framesVal,    0, 1, 1, 1);
    grid->addWidget(typeLabel,       1, 0, 1, 1);
    grid->addWidget(d->typeVal,      1, 1, 1, 1);
    grid->addWidget(bitrateLabel,    2, 0, 1, 1);
    grid->addWidget(d->bitrateVal,   2, 1, 1, 1);
    grid->addWidget(transLabel,      3, 0, 1, 1);
    grid->addWidget(d->transVal,     3, 1, 1, 1);
    grid->setRowStretch(4, 10);

    setPageWidget(main);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("video-mp4")));
}

VidSlideVideoPage::~VidSlideVideoPage()
{
    delete d;
}

void VidSlideVideoPage::initializePage()
{
    d->framesVal->setValue(d->settings->aframes);
    d->typeVal->setCurrentIndex(d->settings->outputType);
    d->bitrateVal->setCurrentIndex(d->settings->vbitRate);
    d->transVal->setCurrentIndex(d->settings->transition);
}

bool VidSlideVideoPage::validatePage()
{
    d->settings->aframes    = d->framesVal->value();
    d->settings->outputType = (VidSlideSettings::VidType)d->typeVal->currentIndex();
    d->settings->vbitRate   = (VidSlideSettings::VidBitRate)d->bitrateVal->currentIndex();
    d->settings->transition = (TransitionMngr::TransType)d->transVal->currentIndex();

    return true;
}

} // namespace Digikam
