/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor plugin for add film
 *               grain on an image.
 *
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include <QLabel>
#include <QLCDNumber>
#include <QSlider>
#include <QImage>
#include <QGridLayout>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kglobal.h>

// Local includes.

#include "version.h"
#include "daboutdata.h"
#include "ddebug.h"
#include "dimg.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "filmgrain.h"
#include "imageeffect_filmgrain.h"
#include "imageeffect_filmgrain.moc"

namespace DigikamFilmGrainImagesPlugin
{

ImageEffect_FilmGrain::ImageEffect_FilmGrain(QWidget* parent)
                     : Digikam::CtrlPanelDlg(parent, i18n("Add Film Grain to Photograph"),
                                             "filmgrain", false, false, true,
                                             Digikam::ImagePannelWidget::SeparateViewAll)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Film Grain"),
                                       digiKamVersion().toAscii(),
                                       ki18n("A digiKam image plugin to apply a film grain "
                                                 "effect to an image."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2004-2005, Gilles Caulier\n"
                                       "(c) 2006-2007, Gilles Caulier and Marcel Wiesweg"),
                                       KLocalizedString(),
                                       Digikam::webProjectUrl().url().toUtf8());

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("Marcel Wiesweg"), ki18n("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);

    // -------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings );

    QLabel *label1      = new QLabel(i18n("Sensitivity (ISO):"), gboxSettings);

    m_sensibilitySlider = new QSlider(Qt::Horizontal, gboxSettings);
    m_sensibilitySlider->setMinimum(2);
    m_sensibilitySlider->setMaximum(30);
    m_sensibilitySlider->setPageStep(1);
    m_sensibilitySlider->setValue(12);
    m_sensibilitySlider->setTracking ( false );
    m_sensibilitySlider->setTickInterval(1);
    m_sensibilitySlider->setTickPosition(QSlider::TicksBelow);

    m_sensibilityLCDValue = new QLCDNumber(4, gboxSettings);
    m_sensibilityLCDValue->setSegmentStyle( QLCDNumber::Flat );
    m_sensibilityLCDValue->display( QString::number(2400) );
    whatsThis = i18n("<p>Set here the film ISO-sensitivity to use for "
                     "simulating the film graininess.");

    m_sensibilityLCDValue->setWhatsThis( whatsThis);
    m_sensibilitySlider->setWhatsThis( whatsThis);

    // -------------------------------------------------------------

    gridSettings->addWidget(label1, 0, 0, 1, 2 );
    gridSettings->addWidget(m_sensibilitySlider, 1, 0, 1, 1);
    gridSettings->addWidget(m_sensibilityLCDValue, 1, 1, 1, 1);
    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(spacingHint());

    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect( m_sensibilitySlider, SIGNAL(valueChanged(int)),
             this, SLOT(slotTimer()) );

    // this connection is necessary to change the LCD display when
    // the value is changed by single clicking on the slider
    connect( m_sensibilitySlider, SIGNAL(valueChanged(int)),
             this, SLOT(slotSliderMoved(int)) );

    connect( m_sensibilitySlider, SIGNAL(sliderMoved(int)),
             this, SLOT(slotSliderMoved(int)) );
}

ImageEffect_FilmGrain::~ImageEffect_FilmGrain()
{
}

void ImageEffect_FilmGrain::renderingFinished()
{
    m_sensibilitySlider->setEnabled(true);
}

void ImageEffect_FilmGrain::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("filmgrain Tool Dialog");
    m_sensibilitySlider->blockSignals(true);
    m_sensibilitySlider->setValue(group.readEntry("SensitivityAjustment", 12));
    m_sensibilitySlider->blockSignals(false);
    slotSliderMoved(m_sensibilitySlider->value());
}

void ImageEffect_FilmGrain::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("filmgrain Tool Dialog");
    group.writeEntry("SensitivityAjustment", m_sensibilitySlider->value());
    config->sync();
}

void ImageEffect_FilmGrain::resetValues()
{
    m_sensibilitySlider->blockSignals(true);
    m_sensibilitySlider->setValue(12);
    m_sensibilitySlider->blockSignals(false);
}

void ImageEffect_FilmGrain::slotSliderMoved(int v)
{
    m_sensibilityLCDValue->display( QString::number(400+200*v) );
}

void ImageEffect_FilmGrain::prepareEffect()
{
    m_sensibilitySlider->setEnabled(false);

    Digikam::DImg image = m_imagePreviewWidget->getOriginalRegionImage();
    int s = 400 + 200 * m_sensibilitySlider->value();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>
                                   (new FilmGrain(&image, this, s));
}

void ImageEffect_FilmGrain::prepareFinal()
{
    m_sensibilitySlider->setEnabled(false);

    int s = 400 + 200 * m_sensibilitySlider->value();

    Digikam::ImageIface iface(0, 0);

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>
                                   (new FilmGrain(iface.getOriginalImg(), this, s));
}

void ImageEffect_FilmGrain::putPreviewData(void)
{
    m_imagePreviewWidget->setPreviewImage(m_threadedFilter->getTargetImage());
}

void ImageEffect_FilmGrain::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Film Grain"), m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamFilmGrainImagesPlugin
