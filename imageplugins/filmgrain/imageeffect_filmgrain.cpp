/* ============================================================
 * File  : imageeffect_filmgrain.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
           Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date  : 2004-08-26
 * Description : a digiKam image editor plugin for add film 
 *               grain on an image.
 * 
 * Copyright 2004-2005 by Gilles Caulier
 * Copyright 2006 by Gilles Caulier and Marcel Wiesweg
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

#include <qlabel.h>
#include <qwhatsthis.h>
#include <qlcdnumber.h>
#include <qslider.h>
#include <qlayout.h>
#include <qimage.h>

// KDE includes.

#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// Local includes.

#include "version.h"
#include "filmgrain.h"
#include "imageeffect_filmgrain.h"

namespace DigikamFilmGrainImagesPlugin
{

ImageEffect_FilmGrain::ImageEffect_FilmGrain(QWidget* parent, QString title, QFrame* banner)
                     : Digikam::CtrlPanelDlg(parent, title, "filmgrain", false, false, true,
                                             Digikam::ImagePannelWidget::SeparateViewAll, banner)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Film Grain"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to apply a film grain "
                                                 "effect to an image."),
                                       KAboutData::License_GPL,
                                       "(c) 2004-2005, Gilles Caulier\n"
                                       "(c) 2006, Gilles Caulier and Marcel Wiesweg",
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");

    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at kdemail dot net");

    about->addAuthor("Marcel Wiesweg", I18N_NOOP("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);

    // -------------------------------------------------------------

    QWidget *gboxSettings = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings, 1, 1, 0, spacingHint());
    QLabel *label1 = new QLabel(i18n("Sensibility (ISO):"), gboxSettings);

    m_sensibilitySlider = new QSlider(2, 30, 1, 12, Qt::Horizontal, gboxSettings);
    m_sensibilitySlider->setTracking ( false );
    m_sensibilitySlider->setTickInterval(1);
    m_sensibilitySlider->setTickmarks(QSlider::Below);

    m_sensibilityLCDValue = new QLCDNumber (4, gboxSettings);
    m_sensibilityLCDValue->setSegmentStyle ( QLCDNumber::Flat );
    m_sensibilityLCDValue->display( QString::number(2400) );
    whatsThis = i18n("<p>Set here the film ISO-sensitivity to use for simulating the film graininess.");

    QWhatsThis::add( m_sensibilityLCDValue, whatsThis);
    QWhatsThis::add( m_sensibilitySlider, whatsThis);

    gridSettings->addMultiCellWidget(label1, 0, 0, 0, 1);
    gridSettings->addMultiCellWidget(m_sensibilitySlider, 1, 1, 0, 0);
    gridSettings->addMultiCellWidget(m_sensibilityLCDValue, 1, 1, 1, 1);

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

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new FilmGrain(&image, this, s));
}

void ImageEffect_FilmGrain::prepareFinal()
{
    m_sensibilitySlider->setEnabled(false);

    int s = 400 + 200 * m_sensibilitySlider->value();

    Digikam::ImageIface iface(0, 0);

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new FilmGrain(iface.getOriginalImg(), this, s));
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

#include "imageeffect_filmgrain.moc"
