/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-09
 * Description : a plugin to apply Blur FX to images
 *
 * Copyright 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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


// #include "blurfxtool.h"
#include "blurfxtool.moc"

// Qt includes

#include <QDateTime>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QSlider>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>

// Local includes

#include "blurfx.h"
#include "daboutdata.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagepanelwidget.h"
#include "version.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamBlurFXImagesPlugin
{

class BlurFXToolPriv
{
public:

    BlurFXToolPriv() :
        configGroupName("blurfx Tool"),
        configEffectTypeEntry("EffectType"),
        configDistanceAdjustmentEntry("DistanceAdjustment"),
        configLevelAdjustmentEntry("LevelAdjustment"),

        effectTypeLabel(0),
        distanceLabel(0),
        levelLabel(0),
        effectType(0),
        distanceInput(0),
        levelInput(0),
        previewWidget(0),
        gboxSettings(0)
        {}


    const QString       configGroupName;
    const QString       configEffectTypeEntry;
    const QString       configDistanceAdjustmentEntry;
    const QString       configLevelAdjustmentEntry;

    QLabel*             effectTypeLabel;
    QLabel*             distanceLabel;
    QLabel*             levelLabel;

    RComboBox*          effectType;

    RIntNumInput*       distanceInput;
    RIntNumInput*       levelInput;

    ImagePanelWidget*   previewWidget;

    EditorToolSettings* gboxSettings;
};

BlurFXTool::BlurFXTool(QObject* parent)
          : EditorToolThreaded(parent),
            d(new BlurFXToolPriv)
{
    setObjectName("blurfx");
    setToolName(i18n("Blur FX"));
    setToolIcon(SmallIcon("blurfx"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Try);

    d->gboxSettings->setTools(EditorToolSettings::PanIcon);

    d->previewWidget = new ImagePanelWidget(470, 350, "blurfx Tool", d->gboxSettings->panIconView());

    // -------------------------------------------------------------

    d->effectTypeLabel = new QLabel(i18n("Type:"));
    d->effectType      = new RComboBox;
    d->effectType->addItem(i18n("Zoom Blur"));
    d->effectType->addItem(i18n("Radial Blur"));
    d->effectType->addItem(i18n("Far Blur"));
    d->effectType->addItem(i18n("Motion Blur"));
    d->effectType->addItem(i18n("Softener Blur"));
    d->effectType->addItem(i18n("Shake Blur"));
    d->effectType->addItem(i18n("Focus Blur"));
    d->effectType->addItem(i18n("Smart Blur"));
    d->effectType->addItem(i18n("Frost Glass"));
    d->effectType->addItem(i18n("Mosaic"));
    d->effectType->setDefaultIndex(BlurFX::ZoomBlur);
    d->effectType->setWhatsThis(i18n("<p>Select the blurring effect to apply to image.</p>"
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
                                     "<p><b>Shake Blur</b>: blurs the image by shaking randomly the pixels. "
                                     "This simulates the blur of a random moving camera.</p>"
                                     "<p><b>Focus Blur</b>: blurs the image corners to reproduce the astigmatism distortion "
                                     "of a lens.</p>"
                                     "<p><b>Smart Blur</b>: finds the edges of color in your image and blurs them without "
                                     "muddying the rest of the image.</p>"
                                     "<p><b>Frost Glass</b>: blurs the image by randomly disperse light coming through "
                                     "a frosted glass.</p>"
                                     "<p><b>Mosaic</b>: divides the photograph into rectangular cells and then "
                                     "recreates it by filling those cells with average pixel value.</p>"));

    d->distanceLabel = new QLabel(i18n("Distance:"));
    d->distanceInput = new RIntNumInput;
    d->distanceInput->setRange(0, 100, 1);
    d->distanceInput->setSliderEnabled(true);
    d->distanceInput->setDefaultValue(3);
    d->distanceInput->setWhatsThis( i18n("Set here the blur distance in pixels."));

    d->levelLabel = new QLabel(i18nc("level to use for the effect", "Level:"));
    d->levelInput = new RIntNumInput;
    d->levelInput->setRange(0, 360, 1);
    d->levelInput->setSliderEnabled(true);
    d->levelInput->setDefaultValue(128);
    d->levelInput->setWhatsThis( i18n("This value controls the level to use with the current effect."));

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(d->effectTypeLabel, 0, 0, 1, 2);
    mainLayout->addWidget(d->effectType,      1, 0, 1, 2);
    mainLayout->addWidget(d->distanceLabel,   2, 0, 1, 2);
    mainLayout->addWidget(d->distanceInput,   3, 0, 1, 2);
    mainLayout->addWidget(d->levelLabel,      4, 0, 1, 2);
    mainLayout->addWidget(d->levelInput,      5, 0, 1, 2);
    mainLayout->setRowStretch(6, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    init();
}

BlurFXTool::~BlurFXTool()
{
    delete d;
}

void BlurFXTool::renderingFinished()
{

    d->effectTypeLabel->setEnabled(true);
    d->effectType->setEnabled(true);
    d->distanceInput->setEnabled(true);
    d->distanceLabel->setEnabled(true);

    switch (d->effectType->currentIndex())
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
          d->levelInput->setEnabled(true);
          d->levelLabel->setEnabled(true);
          break;

       case BlurFX::SoftenerBlur:
          d->distanceInput->setEnabled(false);
          d->distanceLabel->setEnabled(false);
          break;
    }
}

void BlurFXTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    blockWidgetSignals(true);

    d->effectType->setCurrentIndex(group.readEntry(d->configEffectTypeEntry,     d->effectType->defaultIndex()));
    d->distanceInput->setValue(group.readEntry(d->configDistanceAdjustmentEntry, d->distanceInput->defaultValue()));
    d->levelInput->setValue(group.readEntry(d->configLevelAdjustmentEntry,       d->levelInput->defaultValue()));

    blockWidgetSignals(false);
}

void BlurFXTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configEffectTypeEntry,         d->effectType->currentIndex());
    group.writeEntry(d->configDistanceAdjustmentEntry, d->distanceInput->value());
    group.writeEntry(d->configLevelAdjustmentEntry,    d->levelInput->value());
    d->previewWidget->writeSettings();
    group.sync();
}

void BlurFXTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->effectType->slotReset();
    d->distanceInput->slotReset();
    d->levelInput->slotReset();

    blockWidgetSignals(false);

    slotEffectTypeChanged(d->effectType->defaultIndex());
}

void BlurFXTool::slotEffectTypeChanged(int type)
{
    d->distanceInput->setEnabled(true);
    d->distanceLabel->setEnabled(true);

    blockWidgetSignals(true);

    d->distanceInput->setRange(0, 200, 1);
    d->distanceInput->setSliderEnabled(true);
    d->distanceInput->setValue(100);
    d->levelInput->setRange(0, 360, 1);
    d->levelInput->setSliderEnabled(true);
    d->levelInput->setValue(45);

    d->levelInput->setEnabled(false);
    d->levelLabel->setEnabled(false);

    switch (type)
    {
       case BlurFX::ZoomBlur:
          break;

       case BlurFX::RadialBlur:
       case BlurFX::FrostGlass:
          d->distanceInput->setRange(0, 10, 1);
          d->distanceInput->setSliderEnabled(true);
          d->distanceInput->setValue(3);
          break;

       case BlurFX::FarBlur:
          d->distanceInput->setRange(0, 20, 1);
          d->distanceInput->setSliderEnabled(true);
          d->distanceInput->input()->setMaximum(20);
          d->distanceInput->setValue(10);
          break;

       case BlurFX::MotionBlur:
       case BlurFX::FocusBlur:
          d->distanceInput->setRange(0, 100, 1);
          d->distanceInput->setSliderEnabled(true);
          d->distanceInput->setValue(20);
          d->levelInput->setEnabled(true);
          d->levelLabel->setEnabled(true);
          break;

       case BlurFX::SoftenerBlur:
          d->distanceInput->setEnabled(false);
          d->distanceLabel->setEnabled(false);
          break;

       case BlurFX::ShakeBlur:
          d->distanceInput->setRange(0, 100, 1);
          d->distanceInput->setSliderEnabled(true);
          d->distanceInput->setValue(20);
          break;

       case BlurFX::SmartBlur:
          d->distanceInput->setRange(0, 20, 1);
          d->distanceInput->setSliderEnabled(true);
          d->distanceInput->setValue(3);
          d->levelInput->setEnabled(true);
          d->levelLabel->setEnabled(true);
          d->levelInput->setRange(0, 255, 1);
          d->levelInput->setSliderEnabled(true);
          d->levelInput->setValue(128);
          break;

       case BlurFX::Mosaic:
          d->distanceInput->setRange(0, 50, 1);
          d->distanceInput->setSliderEnabled(true);
          d->distanceInput->setValue(3);
          break;
    }

    blockWidgetSignals(false);

    slotEffect();
}

void BlurFXTool::prepareEffect()
{
    d->effectTypeLabel->setEnabled(false);
    d->effectType->setEnabled(false);
    d->distanceInput->setEnabled(false);
    d->distanceLabel->setEnabled(false);
    d->levelInput->setEnabled(false);
    d->levelLabel->setEnabled(false);

    DImg image;

    switch (d->effectType->currentIndex())
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
           image = d->previewWidget->getOriginalRegionImage();
           break;
    }

    int type  = d->effectType->currentIndex();
    int dist  = d->distanceInput->value();
    int level = d->levelInput->value();

    setFilter(dynamic_cast<DImgThreadedFilter*>(new BlurFX(&image, this, type, dist, level)));
}

void BlurFXTool::prepareFinal()
{
    d->effectTypeLabel->setEnabled(false);
    d->effectType->setEnabled(false);
    d->distanceInput->setEnabled(false);
    d->distanceLabel->setEnabled(false);
    d->levelInput->setEnabled(false);
    d->levelLabel->setEnabled(false);

    int type  = d->effectType->currentIndex();
    int dist  = d->distanceInput->value();
    int level = d->levelInput->value();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter *>(new BlurFX(iface.getOriginalImg(), this, type, dist, level)));
}

void BlurFXTool::putPreviewData()
{
    switch (d->effectType->currentIndex())
    {
        case BlurFX::ZoomBlur:
        case BlurFX::RadialBlur:
        case BlurFX::FocusBlur:
        {
            QRect pRect    = d->previewWidget->getOriginalImageRegionToRender();
            DImg destImg = filter()->getTargetImage().copy(pRect);
            d->previewWidget->setPreviewImage(destImg);
            break;
        }
        case BlurFX::FarBlur:
        case BlurFX::MotionBlur:
        case BlurFX::SoftenerBlur:
        case BlurFX::ShakeBlur:
        case BlurFX::SmartBlur:
        case BlurFX::FrostGlass:
        case BlurFX::Mosaic:
            d->previewWidget->setPreviewImage(filter()->getTargetImage());
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
    d->effectType->blockSignals(b);
    d->distanceInput->blockSignals(b);
    d->levelInput->blockSignals(b);
}

}  // namespace DigikamBlurFXImagesPlugin
