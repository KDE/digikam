/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-11
 * Description : a plugin to apply Distortion FX to an image.
 *
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * Original Distortion algorithms copyrighted 2004-2005 by
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
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


#include "distortionfxtool.h"
#include "distortionfxtool.moc"

// Qt includes.

#include <QFrame>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QSpinBox>

// KDE includes.

#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kstandarddirs.h>

// LibKDcraw includes.

#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rcombobox.h>

// Local includes.

#include "daboutdata.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "version.h"
#include "distortionfx.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamDistortionFXImagesPlugin
{

DistortionFXTool::DistortionFXTool(QObject* parent)
                : EditorToolThreaded(parent)
{
    setObjectName("distortionfx");
    setToolName(i18n("Distortion Effects"));
    setToolIcon(SmallIcon("distortionfx"));

    m_previewWidget = new ImageWidget("distortionfx Tool", 0,
                                      i18n("This is the preview of the distortion effect "
                                           "applied to the photograph."),
                                      false, ImageGuideWidget::HVGuideMode);

    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel,
                                            EditorToolSettings::ColorGuide);


    QGridLayout* gridSettings = new QGridLayout(m_gboxSettings->plainPage());

    m_effectTypeLabel = new QLabel(i18n("Type:"), m_gboxSettings->plainPage());

    m_effectType = new RComboBox(m_gboxSettings->plainPage());
    m_effectType->addItem(i18n("Fish Eyes"));
    m_effectType->addItem(i18n("Twirl"));
    m_effectType->addItem(i18n("Cylindrical Hor."));
    m_effectType->addItem(i18n("Cylindrical Vert."));
    m_effectType->addItem(i18n("Cylindrical H/V."));
    m_effectType->addItem(i18n("Caricature"));
    m_effectType->addItem(i18n("Multiple Corners"));
    m_effectType->addItem(i18n("Waves Hor."));
    m_effectType->addItem(i18n("Waves Vert."));
    m_effectType->addItem(i18n("Block Waves 1"));
    m_effectType->addItem(i18n("Block Waves 2"));
    m_effectType->addItem(i18n("Circular Waves 1"));
    m_effectType->addItem(i18n("Circular Waves 2"));
    m_effectType->addItem(i18n("Polar Coordinates"));
    m_effectType->addItem(i18n("Unpolar Coordinates"));
    m_effectType->addItem(i18n("Tile"));
    m_effectType->setDefaultIndex(DistortionFX::FishEye);
    m_effectType->setWhatsThis( i18n("<p>Here, select the type of effect to apply to an image.</p>"
                                     "<p><b>Fish Eyes</b>: warps the photograph around a 3D spherical shape to "
                                            "reproduce the common photograph 'Fish Eyes' effect.</p>"
                                     "<p><b>Twirl</b>: spins the photograph to produce a Twirl pattern.</p>"
                                     "<p><b>Cylinder Hor.</b>: warps the photograph around a horizontal cylinder.</p>"
                                     "<p><b>Cylinder Vert.</b>: warps the photograph around a vertical cylinder.</p>"
                                     "<p><b>Cylinder H/V.</b>: warps the photograph around 2 cylinders, vertical "
                                            "and horizontal.</p>"
                                     "<p><b>Caricature</b>: distorts the photograph with the 'Fish Eyes' effect inverted.</p>"
                                     "<p><b>Multiple Corners</b>: splits the photograph like a multiple corners pattern.</p>"
                                     "<p><b>Waves Horizontal</b>: distorts the photograph with horizontal waves.</p>"
                                     "<p><b>Waves Vertical</b>: distorts the photograph with vertical waves.</p>"
                                     "<p><b>Block Waves 1</b>: divides the image into cells and makes it look as "
                                            "if it is being viewed through glass blocks.</p>"
                                     "<p><b>Block Waves 2</b>: like Block Waves 1 but with another version "
                                            "of glass blocks distortion.</p>"
                                     "<p><b>Circular Waves 1</b>: distorts the photograph with circular waves.</p>"
                                     "<p><b>Circular Waves 2</b>: another variation of the Circular Waves effect.</p>"
                                     "<p><b>Polar Coordinates</b>: converts the photograph from rectangular "
                                            "to polar coordinates.</p>"
                                     "<p><b>Unpolar Coordinates</b>: the Polar Coordinate effect inverted.</p>"
                                     "<p><b>Tile</b>: splits the photograph into square blocks and moves "
                                            "them randomly inside the image.</p>"
                                     ));

    m_levelLabel = new QLabel(i18nc("level of the effect", "Level:"), m_gboxSettings->plainPage());
    m_levelInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_levelInput->setRange(0, 100, 1);
    m_levelInput->setSliderEnabled(true);
    m_levelInput->setDefaultValue(50);
    m_levelInput->setWhatsThis( i18n("Set here the level of the effect."));

    m_iterationLabel = new QLabel(i18n("Iteration:"), m_gboxSettings->plainPage());
    m_iterationInput = new RIntNumInput(m_gboxSettings->plainPage());
    m_iterationInput->setRange(0, 100, 1);
    m_iterationInput->setSliderEnabled(true);
    m_iterationInput->setDefaultValue(10);
    m_iterationInput->setWhatsThis( i18n("This value controls the iterations to use for Waves, "
                                         "Tile, and Neon effects."));

    // -------------------------------------------------------------

    gridSettings->addWidget(m_effectTypeLabel,  0, 0, 1, 3);
    gridSettings->addWidget(m_effectType,       1, 0, 1, 3);
    gridSettings->addWidget(m_levelLabel,       2, 0, 1, 3);
    gridSettings->addWidget(m_levelInput,       3, 0, 1, 3);
    gridSettings->addWidget(m_iterationLabel,   4, 0, 1, 3);
    gridSettings->addWidget(m_iterationInput,   5, 0, 1, 3);
    gridSettings->setRowStretch(6, 10);
    gridSettings->setMargin(m_gboxSettings->spacingHint());
    gridSettings->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_effectType, SIGNAL(activated(int)),
            this, SLOT(slotEffectTypeChanged(int)));

    connect(m_levelInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(m_iterationInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

DistortionFXTool::~DistortionFXTool()
{
}

void DistortionFXTool::renderingFinished()
{
    m_effectTypeLabel->setEnabled(true);
    m_effectType->setEnabled(true);
    m_levelInput->setEnabled(true);
    m_levelLabel->setEnabled(true);
    m_iterationInput->setEnabled(true);
    m_iterationLabel->setEnabled(true);

    switch (m_effectType->currentIndex())
    {
        case DistortionFX::FishEye:
        case DistortionFX::Twirl:
        case DistortionFX::CilindricalHor:
        case DistortionFX::CilindricalVert:
        case DistortionFX::CilindricalHV:
        case DistortionFX::Caricature:
        case DistortionFX::MultipleCorners:
            break;

        case DistortionFX::PolarCoordinates:
        case DistortionFX::UnpolarCoordinates:
            m_levelInput->setEnabled(false);
            m_levelLabel->setEnabled(false);
            break;

        case DistortionFX::WavesHorizontal:
        case DistortionFX::WavesVertical:
        case DistortionFX::BlockWaves1:
        case DistortionFX::BlockWaves2:
        case DistortionFX::CircularWaves1:
        case DistortionFX::CircularWaves2:
        case DistortionFX::Tile:
            m_iterationInput->setEnabled(true);
            m_iterationLabel->setEnabled(true);
            break;
    }
}

void DistortionFXTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("distortionfx Tool");

    m_effectType->blockSignals(true);
    m_iterationInput->blockSignals(true);
    m_levelInput->blockSignals(true);

    m_effectType->setCurrentIndex(group.readEntry("EffectType", (int)DistortionFX::FishEye));
    m_iterationInput->setValue(group.readEntry("IterationAdjustment", 10));
    m_levelInput->setValue(group.readEntry("LevelAdjustment", 50));

    m_effectType->blockSignals(false);
    m_iterationInput->blockSignals(false);
    m_levelInput->blockSignals(false);

    slotEffect();
}

void DistortionFXTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("distortionfx Tool");
    group.writeEntry("EffectType", m_effectType->currentIndex());
    group.writeEntry("IterationAdjustment", m_iterationInput->value());
    group.writeEntry("LevelAdjustment", m_levelInput->value());
    m_previewWidget->writeSettings();
    config->sync();
}

void DistortionFXTool::slotResetSettings()
{
    m_effectType->blockSignals(true);
    m_iterationInput->blockSignals(true);
    m_levelInput->blockSignals(true);

    m_effectType->slotReset();
    m_iterationInput->slotReset();
    m_levelInput->slotReset();
    slotEffectTypeChanged(m_effectType->defaultIndex());

    m_effectType->blockSignals(false);
    m_iterationInput->blockSignals(false);
    m_levelInput->blockSignals(false);
}

void DistortionFXTool::slotEffectTypeChanged(int type)
{
    m_levelInput->setEnabled(true);
    m_levelLabel->setEnabled(true);

    m_levelInput->blockSignals(true);
    m_iterationInput->blockSignals(true);
    m_levelInput->setRange(0, 100, 1);
    m_levelInput->setSliderEnabled(true);
    m_levelInput->setValue(25);

    switch (type)
    {
        case DistortionFX::Twirl:
            m_levelInput->setRange(-50, 50, 1);
            m_levelInput->setSliderEnabled(true);
            m_levelInput->setValue(10);
            break;

        case DistortionFX::FishEye:
        case DistortionFX::CilindricalHor:
        case DistortionFX::CilindricalVert:
        case DistortionFX::CilindricalHV:
        case DistortionFX::Caricature:
            m_levelInput->setRange(0, 200, 1);
            m_levelInput->setSliderEnabled(true);
            m_levelInput->setValue(50);
            break;

        case DistortionFX::MultipleCorners:
            m_levelInput->setRange(1, 10, 1);
            m_levelInput->setSliderEnabled(true);
            m_levelInput->setValue(4);
            break;

        case DistortionFX::WavesHorizontal:
        case DistortionFX::WavesVertical:
        case DistortionFX::BlockWaves1:
        case DistortionFX::BlockWaves2:
        case DistortionFX::CircularWaves1:
        case DistortionFX::CircularWaves2:
        case DistortionFX::Tile:
            m_iterationInput->setEnabled(true);
            m_iterationLabel->setEnabled(true);
            m_iterationInput->setRange(0, 200, 1);
            m_iterationInput->setSliderEnabled(true);
            m_iterationInput->setValue(10);
            break;

        case DistortionFX::PolarCoordinates:
        case DistortionFX::UnpolarCoordinates:
            m_levelInput->setEnabled(false);
            m_levelLabel->setEnabled(false);
            break;
    }

    m_levelInput->blockSignals(false);
    m_iterationInput->blockSignals(false);

    slotEffect();
}

void DistortionFXTool::prepareEffect()
{
    m_effectTypeLabel->setEnabled(false);
    m_effectType->setEnabled(false);
    m_levelInput->setEnabled(false);
    m_levelLabel->setEnabled(false);
    m_iterationInput->setEnabled(false);
    m_iterationLabel->setEnabled(false);

    int l = m_levelInput->value();
    int f = m_iterationInput->value();
    int e = m_effectType->currentIndex();

    ImageIface* iface = m_previewWidget->imageIface();

    uchar *data = iface->getPreviewImage();
    DImg image(iface->previewWidth(), iface->previewHeight(), iface->previewSixteenBit(),
                        iface->previewHasAlpha(), data);
    delete [] data;

    setFilter(dynamic_cast<DImgThreadedFilter *>(
                       new DistortionFX(&image, this, e, l, f)));
}

void DistortionFXTool::prepareFinal()
{
    m_effectTypeLabel->setEnabled(false);
    m_effectType->setEnabled(false);
    m_levelInput->setEnabled(false);
    m_levelLabel->setEnabled(false);
    m_iterationInput->setEnabled(false);
    m_iterationLabel->setEnabled(false);

    int l = m_levelInput->value();
    int f = m_iterationInput->value();
    int e = m_effectType->currentIndex();

    ImageIface iface(0, 0);

    setFilter(dynamic_cast<DImgThreadedFilter *>(
                       new DistortionFX(iface.getOriginalImg(), this, e, l, f)));
}

void DistortionFXTool::putPreviewData(void)
{
    ImageIface* iface = m_previewWidget->imageIface();

    DImg imDest = filter()->getTargetImage()
            .smoothScale(iface->previewWidth(), iface->previewHeight());
    iface->putPreviewImage(imDest.bits());

    m_previewWidget->updatePreview();
}

void DistortionFXTool::putFinalData(void)
{
    ImageIface iface(0, 0);

    iface.putOriginalImage(i18n("Distortion Effects"),
                           filter()->getTargetImage().bits());
}

}  // namespace DigikamDistortionFXImagesPlugin
