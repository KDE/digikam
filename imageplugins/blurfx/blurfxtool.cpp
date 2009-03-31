/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-09
 * Description : a plugin to apply Blur FX to images
 *
 * Copyright 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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


#include "blurfxtool.h"
#include "blurfxtool.moc"

// Qt includes.

#include <QDateTime>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QSlider>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

// Local includes.

#include "daboutdata.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "version.h"
#include "blurfx.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamBlurFXImagesPlugin
{

BlurFXTool::BlurFXTool(QObject* parent)
          : EditorToolThreaded(parent)
{
    setObjectName("blurfx");
    setToolName(i18n("Blur FX"));
    setToolIcon(SmallIcon("blurfx"));

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel|
                                            EditorToolSettings::Try,
                                            EditorToolSettings::PanIcon);

    QGridLayout* grid = new QGridLayout(m_gboxSettings->plainPage());

    m_effectTypeLabel = new QLabel(i18n("Type:"), m_gboxSettings->plainPage());

    m_effectType      = new RComboBox(m_gboxSettings->plainPage());
    m_effectType->addItem(i18n("Zoom Blur"));
    m_effectType->addItem(i18n("Radial Blur"));
    m_effectType->addItem(i18n("Far Blur"));
    m_effectType->addItem(i18n("Motion Blur"));
    m_effectType->addItem(i18n("Softener Blur"));
    m_effectType->addItem(i18n("Skake Blur"));
    m_effectType->addItem(i18n("Focus Blur"));
    m_effectType->addItem(i18n("Smart Blur"));
    m_effectType->addItem(i18n("Frost Glass"));
    m_effectType->addItem(i18n("Mosaic"));
    m_effectType->setDefaultIndex(BlurFX::ZoomBlur);
    m_effectType->setWhatsThis( i18n("<p>Select the blurring effect to apply to image.</p>"
                                     "<p><b>Zoom Blur</b>:  blurs the image along radial lines starting from "
                                     "a specified center point. This simulates the blur of a zooming camera.</p>"
                                     "<p><b>Radial Blur</b>: blurs the image by rotating the pixels around "
                                     "the specified center point. This simulates the blur of a rotating camera.</p>"
                                     "<p><b>Far Blur</b>: blurs the image by using far pixels. This simulates the blur "
                                     "of an unfocalized camera lens.</p>"
                                     "<p><b>Motion Blur</b>: blurs the image by moving the pixels horizontally. "
                                     "This simulates the blur of a linear moving camera.</p>"
                                     "<p><b>Softener Blur</b>: blurs the image softly in dark tones and hardly in light "
                                     "tones. This gives images a dreamy and glossy soft focus effect. It is ideal "
                                     "for creating romantic portraits, glamour photographs, or giving images a warm "
                                     "and subtle glow.</p>"
                                     "<p><b>Skake Blur</b>: blurs the image by skaking randomly the pixels. "
                                     "This simulates the blur of a random moving camera.</p>"
                                     "<p><b>Focus Blur</b>: blurs the image corners to reproduce the astigmatism distortion "
                                     "of a lens.</p>"
                                     "<p><b>Smart Blur</b>: finds the edges of color in your image and blurs them without "
                                     "muddying the rest of the image.</p>"
                                     "<p><b>Frost Glass</b>: blurs the image by randomly disperse light coming through "
                                     "a frosted glass.</p>"
                                     "<p><b>Mosaic</b>: divides the photograph into rectangular cells and then "
                                     "recreates it by filling those cells with average pixel value.</p>"));

    m_distanceLabel = new QLabel(i18n("Distance:"), m_gboxSettings->plainPage());
    m_distanceInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_distanceInput->setRange(0, 100, 1);
    m_distanceInput->setSliderEnabled(true);
    m_distanceInput->setDefaultValue(3);
    m_distanceInput->setWhatsThis( i18n("Set here the blur distance in pixels."));

    m_levelLabel = new QLabel(i18nc("level to use for the effect", "Level:"), m_gboxSettings->plainPage());
    m_levelInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_levelInput->setRange(0, 360, 1);
    m_levelInput->setSliderEnabled(true);
    m_levelInput->setDefaultValue(128);
    m_levelInput->setWhatsThis( i18n("This value controls the level to use with the current effect."));

    // -------------------------------------------------------------

    grid->addWidget(m_effectTypeLabel, 0, 0, 1, 2);
    grid->addWidget(m_effectType,      1, 0, 1, 2);
    grid->addWidget(m_distanceLabel,   2, 0, 1, 2);
    grid->addWidget(m_distanceInput,   3, 0, 1, 2);
    grid->addWidget(m_levelLabel,      4, 0, 1, 2);
    grid->addWidget(m_levelInput,      5, 0, 1, 2);
    grid->setRowStretch(6, 10);
    grid->setMargin(m_gboxSettings->spacingHint());
    grid->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);

    // -------------------------------------------------------------

    m_previewWidget = new ImagePanelWidget(470, 350, "blurfx Tool", m_gboxSettings->panIconView());

    setToolView(m_previewWidget);
    init();

    // -------------------------------------------------------------

    //    connect(m_effectType, SIGNAL(activated(int)),
    //            this, SLOT(slotEffectTypeChanged(int)));
    //
    //    connect(m_distanceInput, SIGNAL(valueChanged(int)),
    //            this, SLOT(slotTimer()));
    //
    //    connect(m_levelInput, SIGNAL(valueChanged(int)),
    //            this, SLOT(slotTimer()));
}

BlurFXTool::~BlurFXTool()
{
}

void BlurFXTool::renderingFinished()
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

void BlurFXTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("blurfx Tool");

    blockWidgetSignals(true);

    m_effectType->setCurrentIndex(group.readEntry("EffectType", m_effectType->defaultIndex()));
    m_distanceInput->setValue(group.readEntry("DistanceAdjustment", m_distanceInput->defaultValue()));
    m_levelInput->setValue(group.readEntry("LevelAdjustment",m_levelInput->defaultValue()));

    blockWidgetSignals(false);
}

void BlurFXTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("blurfx Tool");
    group.writeEntry("EffectType", m_effectType->currentIndex());
    group.writeEntry("DistanceAdjustment", m_distanceInput->value());
    group.writeEntry("LevelAdjustment", m_levelInput->value());
    m_previewWidget->writeSettings();
    group.sync();
}

void BlurFXTool::slotResetSettings()
{
    blockWidgetSignals(true);

    m_effectType->slotReset();
    m_distanceInput->slotReset();
    m_levelInput->slotReset();

    blockWidgetSignals(false);

    slotEffectTypeChanged(m_effectType->defaultIndex());
}

void BlurFXTool::slotEffectTypeChanged(int type)
{
    m_distanceInput->setEnabled(true);
    m_distanceLabel->setEnabled(true);

    blockWidgetSignals(true);

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
          m_distanceInput->input()->setMaximum(20);
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

    blockWidgetSignals(false);

    slotEffect();
}

void BlurFXTool::prepareEffect()
{
    m_effectTypeLabel->setEnabled(false);
    m_effectType->setEnabled(false);
    m_distanceInput->setEnabled(false);
    m_distanceLabel->setEnabled(false);
    m_levelInput->setEnabled(false);
    m_levelLabel->setEnabled(false);

    DImg image;

    switch (m_effectType->currentIndex())
    {
       case BlurFX::ZoomBlur:
       case BlurFX::RadialBlur:
       case BlurFX::FocusBlur:
       {
            ImageIface iface(0, 0);
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
           image = m_previewWidget->getOriginalRegionImage();
           break;
    }

    int t = m_effectType->currentIndex();
    int d = m_distanceInput->value();
    int l = m_levelInput->value();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new BlurFX(&image, this, t, d, l)));
}

void BlurFXTool::prepareFinal()
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

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter *>(new BlurFX(iface.getOriginalImg(), this, t, d, l)));
}

void BlurFXTool::putPreviewData()
{
    switch (m_effectType->currentIndex())
    {
        case BlurFX::ZoomBlur:
        case BlurFX::RadialBlur:
        case BlurFX::FocusBlur:
        {
            QRect pRect    = m_previewWidget->getOriginalImageRegionToRender();
            DImg destImg = filter()->getTargetImage().copy(pRect);
            m_previewWidget->setPreviewImage(destImg);
            break;
        }
        case BlurFX::FarBlur:
        case BlurFX::MotionBlur:
        case BlurFX::SoftenerBlur:
        case BlurFX::ShakeBlur:
        case BlurFX::SmartBlur:
        case BlurFX::FrostGlass:
        case BlurFX::Mosaic:
            m_previewWidget->setPreviewImage(filter()->getTargetImage());
            break;
    }
}

void BlurFXTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Blur Effects"), filter()->getTargetImage().bits());
}

void BlurFXTool::blockWidgetSignals(bool b)
{
    m_effectType->blockSignals(b);
    m_distanceInput->blockSignals(b);
    m_levelInput->blockSignals(b);
}

}  // namespace DigikamBlurFXImagesPlugin
