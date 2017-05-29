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

#include "vidslideoutputpage.h"

// Qt includes

#include <QIcon>
#include <QLabel>
#include <QSpinBox>
#include <QUrl>
#include <QWidget>
#include <QApplication>
#include <QStyle>
#include <QCheckBox>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "vidslidewizard.h"
#include "dfileselector.h"
#include "dlayoutbox.h"

namespace Digikam
{

class VidSlideOutputPage::Private
{
public:

    Private(QWizard* const dialog)
      : destUrl(0),
        framesVal(0),
        typeVal(0),
        transVal(0),
        openInPlayer(0),
        wizard(0),
        settings(0)
    {
        wizard = dynamic_cast<VidSlideWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
        }
    }

    DFileSelector*    destUrl;
    QSpinBox*         framesVal;
    QComboBox*        typeVal;
    QComboBox*        transVal;
    QCheckBox*        openInPlayer;
    VidSlideWizard*   wizard;
    VidSlideSettings* settings;
};

VidSlideOutputPage::VidSlideOutputPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    setObjectName(QLatin1String("OutputPage"));

    DVBox* const vbox         = new DVBox(this);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // --------------------

    DVBox* const hbox1        = new DVBox(vbox);
    hbox1->setContentsMargins(QMargins());
    hbox1->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    QLabel* const framesLabel = new QLabel(hbox1);
    framesLabel->setWordWrap(false);
    framesLabel->setText(i18n("Number of Frames by Image:"));
    d->framesVal              = new QSpinBox(hbox1);
    d->framesVal->setRange(1, 9999);
    framesLabel->setBuddy(d->framesVal);

    // --------------------

    DVBox* const hbox2        = new DVBox(vbox);
    hbox2->setContentsMargins(QMargins());
    hbox2->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    QLabel* const typeLabel = new QLabel(hbox2);
    typeLabel->setWordWrap(false);
    typeLabel->setText(i18n("Video Type:"));
    d->typeVal              = new QComboBox(hbox2);
    d->typeVal->setEditable(false);
    d->typeVal->addItem(i18n("VCD"),      (int)VidSlideSettings::VCD);
    d->typeVal->addItem(i18n("SVCD"),     (int)VidSlideSettings::SVCD);
    d->typeVal->addItem(i18n("DVD"),      (int)VidSlideSettings::DVD);
    d->typeVal->addItem(i18n("HDTV"),     (int)VidSlideSettings::HDTV);
    d->typeVal->addItem(i18n("Blue Ray"), (int)VidSlideSettings::BLUERAY);
    d->typeVal->addItem(i18n("UHD 4K"),   (int)VidSlideSettings::UHD4K);
    typeLabel->setBuddy(d->typeVal);

    // --------------------

    DVBox* const hbox3        = new DVBox(vbox);
    hbox3->setContentsMargins(QMargins());
    hbox3->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    QLabel* const transLabel = new QLabel(hbox3);
    transLabel->setWordWrap(false);
    transLabel->setText(i18n("Transition Type:"));
    d->transVal              = new QComboBox(hbox3);
    d->transVal->setEditable(false);

    QMap<TransitionMngr::TransType, QString> map                = TransitionMngr::transitionNames();
    QMap<TransitionMngr::TransType, QString>::const_iterator it = map.constBegin();

    while (it != map.constEnd())
    {
        d->transVal->addItem(it.value(), (int)it.key());
        ++it;
    }

    transLabel->setBuddy(d->transVal);

    // --------------------

    DVBox* const hbox4        = new DVBox(vbox);
    hbox4->setContentsMargins(QMargins());
    hbox4->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    QLabel* const textLabel1  = new QLabel(hbox4);
    textLabel1->setWordWrap(false);
    textLabel1->setText(i18n("Output video file:"));

    d->destUrl = new DFileSelector(hbox4);
    d->destUrl->setFileDlgMode(QFileDialog::AnyFile);
    d->destUrl->setFileDlgTitle(i18n("Output Video File"));
    textLabel1->setBuddy(d->destUrl);

    // --------------------

    d->openInPlayer           = new QCheckBox(vbox);
    d->openInPlayer->setText(i18n("Open in video player"));

    // --------------------

    QWidget* const spacer    = new QWidget(vbox);
    vbox->setStretchFactor(spacer, 10);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-video")));

    connect(d->destUrl->lineEdit(), SIGNAL(textEdited(QString)),
            this, SIGNAL(completeChanged()));

    connect(d->destUrl, SIGNAL(signalUrlSelected(QUrl)),
            this, SIGNAL(completeChanged()));
}

VidSlideOutputPage::~VidSlideOutputPage()
{
    delete d;
}

void VidSlideOutputPage::initializePage()
{
    d->destUrl->setFileDlgPath(d->settings->outputVideo.toLocalFile());
    d->openInPlayer->setChecked(d->settings->openInPlayer);
    d->framesVal->setValue(d->settings->aframes);
    d->typeVal->setCurrentIndex(d->settings->outputType);
    d->transVal->setCurrentIndex(d->settings->transition);
}

bool VidSlideOutputPage::validatePage()
{
    if (d->destUrl->fileDlgPath().isEmpty())
        return false;

    d->settings->outputVideo  = QUrl::fromLocalFile(d->destUrl->fileDlgPath());
    d->settings->openInPlayer = d->openInPlayer->isChecked();
    d->settings->aframes      = d->framesVal->value();
    d->settings->outputType   = (VidSlideSettings::VidType)d->typeVal->currentIndex();
    d->settings->transition   = (TransitionMngr::TransType)d->transVal->currentIndex();

    return true;
}

bool VidSlideOutputPage::isComplete() const
{
    return (!d->destUrl->fileDlgPath().isEmpty());
}

} // namespace Digikam
