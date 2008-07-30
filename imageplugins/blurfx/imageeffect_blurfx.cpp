/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-09
 * Description : a plugin to apply Blur FX to images
 *
 * Copyright 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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
#include <QSlider>
#include <QImage>
#include <QComboBox>
#include <QDateTime>
#include <QGridLayout>

// KDE includes.

#include <kconfig.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <knuminput.h>
#include <kglobal.h>

// Local includes.

#include "version.h"
#include "daboutdata.h"
#include "ddebug.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "blurfx.h"
#include "imageeffect_blurfx.h"
#include "imageeffect_blurfx.moc"

namespace DigikamBlurFXImagesPlugin
{

ImageEffect_BlurFX::ImageEffect_BlurFX(QWidget* parent)
                  : Digikam::CtrlPanelDlg(parent, i18n("Assortment of blurring special effects"),
                                          "blurfx", false, false, true,
                                          Digikam::ImagePannelWidget::SeparateViewAll)
{
    QString whatsThis;

    KAboutData* about = new KAboutData("digikam", 0,
                                       ki18n("Blur Effects"),
                                       digiKamVersion().toAscii(),
                                       ki18n("A digiKam image plugin to apply blurring special effects "
                                       "to an image."),
                                       KAboutData::License_GPL,
                                       ki18n("(c) 2005, Gilles Caulier\n"
                                       "(c) 2006-2008, Gilles Caulier and Marcel Wiesweg"),
                                       KLocalizedString(),
                                       Digikam::webProjectUrl().url().toUtf8());

    about->addAuthor(ki18n("Gilles Caulier"), ki18n("Author and maintainer"),
                     "caulier dot gilles at gmail dot com");

    about->addAuthor(ki18n("Pieter Z. Voloshyn"), ki18n("Blurring algorithms"),
                     "pieter dot voloshyn at gmail dot com");

    about->addAuthor(ki18n("Marcel Wiesweg"), ki18n("Developer"),
                     "marcel dot wiesweg at gmx dot de");

    setAboutData(about);

    // -------------------------------------------------------------

    QWidget *gboxSettings     = new QWidget(m_imagePreviewWidget);
    QGridLayout* gridSettings = new QGridLayout( gboxSettings );

    m_effectTypeLabel = new QLabel(i18n("Type:"), gboxSettings);

    m_effectType      = new QComboBox( gboxSettings );
    m_effectType->addItem( i18n("Zoom Blur") );
    m_effectType->addItem( i18n("Radial Blur") );
    m_effectType->addItem( i18n("Far Blur") );
    m_effectType->addItem( i18n("Motion Blur") );
    m_effectType->addItem( i18n("Softener Blur") );
    m_effectType->addItem( i18n("Skake Blur") );
    m_effectType->addItem( i18n("Focus Blur") );
    m_effectType->addItem( i18n("Smart Blur") );
    m_effectType->addItem( i18n("Frost Glass") );
    m_effectType->addItem( i18n("Mosaic") );
    m_effectType->setWhatsThis( i18n("<p>Select the blurring effect to apply to image.<p>"
                                     "<b>Zoom Blur</b>:  blurs the image along radial lines starting from "
                                     "a specified center point. This simulates the blur of a zooming camera.<p>"
                                     "<b>Radial Blur</b>: blurs the image by rotating the pixels around "
                                     "the specified center point. This simulates the blur of a rotating camera.<p>"
                                     "<b>Far Blur</b>: blurs the image by using far pixels. This simulates the blur "
                                     "of an unfocalized camera lens.<p>"
                                     "<b>Motion Blur</b>: blurs the image by moving the pixels horizontally. "
                                     "This simulates the blur of a linear moving camera.<p>"
                                     "<b>Softener Blur</b>: blurs the image softly in dark tones and hardly in light "
                                     "tones. This gives images a dreamy and glossy soft focus effect. It's ideal "
                                     "for creating romantic portraits, glamour photographs, or giving images a warm "
                                     "and subtle glow.<p>"
                                     "<b>Skake Blur</b>: blurs the image by skaking randomly the pixels. "
                                     "This simulates the blur of a random moving camera.<p>"
                                     "<b>Focus Blur</b>: blurs the image corners to reproduce the astigmatism distortion "
                                     "of a lens.<p>"
                                     "<b>Smart Blur</b>: finds the edges of color in your image and blurs them without "
                                     "muddying the rest of the image.<p>"
                                     "<b>Frost Glass</b>: blurs the image by randomly disperse light coming through "
                                     "a frosted glass.<p>"
                                     "<b>Mosaic</b>: divides the photograph into rectangular cells and then "
                                     "recreates it by filling those cells with average pixel value."));

    m_distanceLabel = new QLabel(i18n("Distance:"), gboxSettings);
    m_distanceInput = new KIntNumInput(gboxSettings);
    m_distanceInput->setRange(0, 100, 1);
    m_distanceInput->setSliderEnabled(true);
    m_distanceInput->setWhatsThis( i18n("<p>Set here the blur distance in pixels."));

    m_levelLabel = new QLabel(i18n("Level:"), gboxSettings);
    m_levelInput = new KIntNumInput(gboxSettings);
    m_levelInput->setRange(0, 360, 1);
    m_levelInput->setSliderEnabled(true);
    m_levelInput->setWhatsThis( i18n("<p>This value controls the level to use with the current effect."));

    // -------------------------------------------------------------

    gridSettings->addWidget(m_effectTypeLabel, 0, 0, 1, 2 );
    gridSettings->addWidget(m_effectType, 1, 0, 1, 2 );
    gridSettings->addWidget(m_distanceLabel, 2, 0, 1, 2 );
    gridSettings->addWidget(m_distanceInput, 3, 0, 1, 2 );
    gridSettings->addWidget(m_levelLabel, 4, 0, 1, 2 );
    gridSettings->addWidget(m_levelInput, 5, 0, 1, 2 );
    gridSettings->setMargin(spacingHint());
    gridSettings->setSpacing(spacingHint());

    m_imagePreviewWidget->setUserAreaWidget(gboxSettings);

    // -------------------------------------------------------------

    connect(m_effectType, SIGNAL(activated(int)),
            this, SLOT(slotEffectTypeChanged(int)));

    connect(m_distanceInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_levelInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

ImageEffect_BlurFX::~ImageEffect_BlurFX()
{
}

void ImageEffect_BlurFX::renderingFinished(void)
{

    m_effectTypeLabel->setEnabled(true);
    m_effectType->setEnabled(true);
    m_distanceInput->setEnabled(true);
    m_distanceLabel->setEnabled(true);

    switch (m_effectType->currentIndex())
    {
       case BlurFX::ZoomBlur:
       case BlurFX::RadialBlur:
       case BlurFX::FarBlur:
       case BlurFX::ShakeBlur:
       case BlurFX::FrostGlass:
       case BlurFX::Mosaic:
          break;

       case BlurFX::MotionBlur:
       case BlurFX::FocusBlur:
       case BlurFX::SmartBlur:
          m_levelInput->setEnabled(true);
          m_levelLabel->setEnabled(true);
          break;

       case BlurFX::SoftenerBlur:
          m_distanceInput->setEnabled(false);
          m_distanceLabel->setEnabled(false);
          break;
    }
}

void ImageEffect_BlurFX::readUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("blurfx Tool Dialog");
    m_effectType->blockSignals(true);
    m_distanceInput->blockSignals(true);
    m_levelInput->blockSignals(true);
    m_effectType->setCurrentIndex(group.readEntry("EffectType", (int)BlurFX::ZoomBlur));
    m_distanceInput->setValue(group.readEntry("DistanceAjustment", 3));
    m_levelInput->setValue(group.readEntry("LevelAjustment", 128));
    m_effectType->blockSignals(false);
    m_distanceInput->blockSignals(false);
    m_levelInput->blockSignals(false);
}

void ImageEffect_BlurFX::writeUserSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("blurfx Tool Dialog");
    group.writeEntry("EffectType", m_effectType->currentIndex());
    group.writeEntry("DistanceAjustment", m_distanceInput->value());
    group.writeEntry("LevelAjustment", m_levelInput->value());
    group.sync();
}

void ImageEffect_BlurFX::resetValues()
{
    m_effectType->setCurrentIndex(BlurFX::ZoomBlur);
    slotEffectTypeChanged(BlurFX::ZoomBlur);
}

void ImageEffect_BlurFX::slotEffectTypeChanged(int type)
{
    m_distanceInput->setEnabled(true);
    m_distanceLabel->setEnabled(true);

    m_distanceInput->blockSignals(true);
    m_levelInput->blockSignals(true);
    m_distanceInput->setRange(0, 200, 1);
    m_distanceInput->setSliderEnabled(true);
    m_distanceInput->setValue(100);
    m_levelInput->setRange(0, 360, 1);
    m_levelInput->setSliderEnabled(true);
    m_levelInput->setValue(45);

    m_levelInput->setEnabled(false);
    m_levelLabel->setEnabled(false);

    switch (type)
    {
       case BlurFX::ZoomBlur:
          break;

       case BlurFX::RadialBlur:
       case BlurFX::FrostGlass:
          m_distanceInput->setRange(0, 10, 1);
          m_distanceInput->setSliderEnabled(true);
          m_distanceInput->setValue(3);
          break;

       case BlurFX::FarBlur:
          m_distanceInput->setRange(0, 20, 1);
          m_distanceInput->setSliderEnabled(true);
          m_distanceInput->setMaximum(20);
          m_distanceInput->setValue(10);
          break;

       case BlurFX::MotionBlur:
       case BlurFX::FocusBlur:
          m_distanceInput->setRange(0, 100, 1);
          m_distanceInput->setSliderEnabled(true);
          m_distanceInput->setValue(20);
          m_levelInput->setEnabled(true);
          m_levelLabel->setEnabled(true);
          break;

       case BlurFX::SoftenerBlur:
          m_distanceInput->setEnabled(false);
          m_distanceLabel->setEnabled(false);
          break;

       case BlurFX::ShakeBlur:
          m_distanceInput->setRange(0, 100, 1);
          m_distanceInput->setSliderEnabled(true);
          m_distanceInput->setValue(20);
          break;

       case BlurFX::SmartBlur:
          m_distanceInput->setRange(0, 20, 1);
          m_distanceInput->setSliderEnabled(true);
          m_distanceInput->setValue(3);
          m_levelInput->setEnabled(true);
          m_levelLabel->setEnabled(true);
          m_levelInput->setRange(0, 255, 1);
          m_levelInput->setSliderEnabled(true);
          m_levelInput->setValue(128);
          break;

       case BlurFX::Mosaic:
          m_distanceInput->setRange(0, 50, 1);
          m_distanceInput->setSliderEnabled(true);
          m_distanceInput->setValue(3);
          break;
    }

    m_distanceInput->blockSignals(false);
    m_levelInput->blockSignals(false);

    slotEffect();
}

void ImageEffect_BlurFX::prepareEffect()
{
    m_effectTypeLabel->setEnabled(false);
    m_effectType->setEnabled(false);
    m_distanceInput->setEnabled(false);
    m_distanceLabel->setEnabled(false);
    m_levelInput->setEnabled(false);
    m_levelLabel->setEnabled(false);

    Digikam::DImg image;

    switch (m_effectType->currentIndex())
    {
       case BlurFX::ZoomBlur:
       case BlurFX::RadialBlur:
       case BlurFX::FocusBlur:
       {
            Digikam::ImageIface iface(0, 0);
            image = *iface.getOriginalImg();
            break;
       }

       case BlurFX::FarBlur:
       case BlurFX::MotionBlur:
       case BlurFX::SoftenerBlur:
       case BlurFX::ShakeBlur:
       case BlurFX::SmartBlur:
       case BlurFX::FrostGlass:
       case BlurFX::Mosaic:
           image = m_imagePreviewWidget->getOriginalRegionImage();
           break;
    }

    int t = m_effectType->currentIndex();
    int d = m_distanceInput->value();
    int l = m_levelInput->value();

    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new BlurFX(&image, this, t, d, l));
}

void ImageEffect_BlurFX::prepareFinal()
{
    m_effectTypeLabel->setEnabled(false);
    m_effectType->setEnabled(false);
    m_distanceInput->setEnabled(false);
    m_distanceLabel->setEnabled(false);
    m_levelInput->setEnabled(false);
    m_levelLabel->setEnabled(false);

    int t = m_effectType->currentIndex();
    int d = m_distanceInput->value();
    int l = m_levelInput->value();

    Digikam::ImageIface iface(0, 0);
    m_threadedFilter = dynamic_cast<Digikam::DImgThreadedFilter *>(new BlurFX(iface.getOriginalImg(), this, t, d, l));
}

void ImageEffect_BlurFX::putPreviewData(void)
{
    switch (m_effectType->currentIndex())
    {
        case BlurFX::ZoomBlur:
        case BlurFX::RadialBlur:
        case BlurFX::FocusBlur:
        {
            QRect pRect    = m_imagePreviewWidget->getOriginalImageRegionToRender();
            Digikam::DImg destImg = m_threadedFilter->getTargetImage().copy(pRect);
            m_imagePreviewWidget->setPreviewImage(destImg);
            break;
        }
        case BlurFX::FarBlur:
        case BlurFX::MotionBlur:
        case BlurFX::SoftenerBlur:
        case BlurFX::ShakeBlur:
        case BlurFX::SmartBlur:
        case BlurFX::FrostGlass:
        case BlurFX::Mosaic:
            m_imagePreviewWidget->setPreviewImage(m_threadedFilter->getTargetImage());
            break;
    }
}

void ImageEffect_BlurFX::putFinalData(void)
{
    Digikam::ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Blur Effects"),
                           m_threadedFilter->getTargetImage().bits());
}

}  // NameSpace DigikamBlurFXImagesPlugin
