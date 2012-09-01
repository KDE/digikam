/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-11
 * Description : a plugin to apply Distortion FX to an image.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "distortionfxtool.moc"

// Qt includes

#include <QFrame>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QSpinBox>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kglobal.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>

// Local includes

#include "dimg.h"
#include "distortionfxfilter.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"

using namespace KDcrawIface;

namespace DigikamFxFiltersImagePlugin
{

class DistortionFXTool::DistortionFXToolPriv
{
public:

    DistortionFXToolPriv() :
        effectTypeLabel(0),
        levelLabel(0),
        iterationLabel(0),
        effectType(0),
        levelInput(0),
        iterationInput(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configEffectTypeEntry;
    static const QString configIterationAdjustmentEntry;
    static const QString configLevelAdjustmentEntry;

    QLabel*              effectTypeLabel;
    QLabel*              levelLabel;
    QLabel*              iterationLabel;

    RComboBox*           effectType;

    RIntNumInput*        levelInput;
    RIntNumInput*        iterationInput;

    ImageGuideWidget*    previewWidget;
    EditorToolSettings*  gboxSettings;
};
const QString DistortionFXTool::DistortionFXToolPriv::configGroupName("distortionfx Tool");
const QString DistortionFXTool::DistortionFXToolPriv::configEffectTypeEntry("EffectType");
const QString DistortionFXTool::DistortionFXToolPriv::configIterationAdjustmentEntry("IterationAdjustment");
const QString DistortionFXTool::DistortionFXToolPriv::configLevelAdjustmentEntry("LevelAdjustment");

// --------------------------------------------------------

DistortionFXTool::DistortionFXTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new DistortionFXToolPriv)
{
    setObjectName("distortionfx");
    setToolName(i18n("Distortion Effects"));
    setToolIcon(SmallIcon("distortionfx"));

    d->previewWidget = new ImageGuideWidget(0, false, ImageGuideWidget::HVGuideMode);
    d->previewWidget->setWhatsThis(i18n("This is the preview of the distortion effect "
                                        "applied to the photograph."));
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::ColorGuide);

    // -------------------------------------------------------------

    d->effectTypeLabel = new QLabel(i18n("Type:"));
    d->effectType      = new RComboBox();
    d->effectType->addItem(i18n("Fish Eyes"));
    d->effectType->addItem(i18n("Twirl"));
    d->effectType->addItem(i18n("Cylindrical Hor."));
    d->effectType->addItem(i18n("Cylindrical Vert."));
    d->effectType->addItem(i18n("Cylindrical H/V."));
    d->effectType->addItem(i18n("Caricature"));
    d->effectType->addItem(i18n("Multiple Corners"));
    d->effectType->addItem(i18n("Waves Hor."));
    d->effectType->addItem(i18n("Waves Vert."));
    d->effectType->addItem(i18n("Block Waves 1"));
    d->effectType->addItem(i18n("Block Waves 2"));
    d->effectType->addItem(i18n("Circular Waves 1"));
    d->effectType->addItem(i18n("Circular Waves 2"));
    d->effectType->addItem(i18n("Polar Coordinates"));
    d->effectType->addItem(i18n("Unpolar Coordinates"));
    d->effectType->addItem(i18n("Tile"));
    d->effectType->setDefaultIndex(DistortionFXFilter::FishEye);
    d->effectType->setWhatsThis(i18n("<p>Here, select the type of effect to apply to an image.</p>"
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

    d->levelLabel = new QLabel(i18nc("level of the effect", "Level:"));
    d->levelInput = new RIntNumInput();
    d->levelInput->setRange(0, 100, 1);
    d->levelInput->setSliderEnabled(true);
    d->levelInput->setDefaultValue(50);
    d->levelInput->setWhatsThis( i18n("Set here the level of the effect."));

    d->iterationLabel = new QLabel(i18n("Iteration:"));
    d->iterationInput = new RIntNumInput();
    d->iterationInput->setRange(0, 100, 1);
    d->iterationInput->setSliderEnabled(true);
    d->iterationInput->setDefaultValue(10);
    d->iterationInput->setWhatsThis( i18n("This value controls the iterations to use for Waves, "
                                          "Tile, and Neon effects."));

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->addWidget(d->effectTypeLabel,  0, 0, 1, 3);
    mainLayout->addWidget(d->effectType,       1, 0, 1, 3);
    mainLayout->addWidget(d->levelLabel,       2, 0, 1, 3);
    mainLayout->addWidget(d->levelInput,       3, 0, 1, 3);
    mainLayout->addWidget(d->iterationLabel,   4, 0, 1, 3);
    mainLayout->addWidget(d->iterationInput,   5, 0, 1, 3);
    mainLayout->setRowStretch(6, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->effectType, SIGNAL(activated(int)),
            this, SLOT(slotEffectTypeChanged(int)));

    connect(d->levelInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->iterationInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));
}

DistortionFXTool::~DistortionFXTool()
{
    delete d;
}

void DistortionFXTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->effectType->blockSignals(true);
    d->iterationInput->blockSignals(true);
    d->levelInput->blockSignals(true);

    d->effectType->setCurrentIndex(group.readEntry(d->configEffectTypeEntry,       (int)DistortionFXFilter::FishEye));
    d->iterationInput->setValue(group.readEntry(d->configIterationAdjustmentEntry, 10));
    d->levelInput->setValue(group.readEntry(d->configLevelAdjustmentEntry,         50));

    d->effectType->blockSignals(false);
    d->iterationInput->blockSignals(false);
    d->levelInput->blockSignals(false);

    slotEffect();
}

void DistortionFXTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configEffectTypeEntry,          d->effectType->currentIndex());
    group.writeEntry(d->configIterationAdjustmentEntry, d->iterationInput->value());
    group.writeEntry(d->configLevelAdjustmentEntry,     d->levelInput->value());

    config->sync();
}

void DistortionFXTool::slotResetSettings()
{
    d->effectType->blockSignals(true);
    d->iterationInput->blockSignals(true);
    d->levelInput->blockSignals(true);

    d->effectType->slotReset();
    d->iterationInput->slotReset();
    d->levelInput->slotReset();
    slotEffectTypeChanged(d->effectType->defaultIndex());

    d->effectType->blockSignals(false);
    d->iterationInput->blockSignals(false);
    d->levelInput->blockSignals(false);
}

void DistortionFXTool::slotEffectTypeChanged(int type)
{
    d->levelInput->setEnabled(true);
    d->levelLabel->setEnabled(true);

    d->levelInput->blockSignals(true);
    d->iterationInput->blockSignals(true);
    d->levelInput->setRange(0, 100, 1);
    d->levelInput->setSliderEnabled(true);
    d->levelInput->setValue(25);

    switch (type)
    {
        case DistortionFXFilter::Twirl:
            d->levelInput->setRange(-50, 50, 1);
            d->levelInput->setSliderEnabled(true);
            d->levelInput->setValue(10);
            break;

        case DistortionFXFilter::FishEye:
        case DistortionFXFilter::CilindricalHor:
        case DistortionFXFilter::CilindricalVert:
        case DistortionFXFilter::CilindricalHV:
        case DistortionFXFilter::Caricature:
            d->levelInput->setRange(0, 200, 1);
            d->levelInput->setSliderEnabled(true);
            d->levelInput->setValue(50);
            break;

        case DistortionFXFilter::MultipleCorners:
            d->levelInput->setRange(1, 10, 1);
            d->levelInput->setSliderEnabled(true);
            d->levelInput->setValue(4);
            break;

        case DistortionFXFilter::WavesHorizontal:
        case DistortionFXFilter::WavesVertical:
        case DistortionFXFilter::BlockWaves1:
        case DistortionFXFilter::BlockWaves2:
        case DistortionFXFilter::CircularWaves1:
        case DistortionFXFilter::CircularWaves2:
        case DistortionFXFilter::Tile:
            d->iterationInput->setEnabled(true);
            d->iterationLabel->setEnabled(true);
            d->iterationInput->setRange(0, 200, 1);
            d->iterationInput->setSliderEnabled(true);
            d->iterationInput->setValue(10);
            break;

        case DistortionFXFilter::PolarCoordinates:
        case DistortionFXFilter::UnpolarCoordinates:
            d->levelInput->setEnabled(false);
            d->levelLabel->setEnabled(false);
            break;
    }

    d->levelInput->blockSignals(false);
    d->iterationInput->blockSignals(false);

    slotEffect();
}

void DistortionFXTool::prepareEffect()
{
    d->effectTypeLabel->setEnabled(false);
    d->effectType->setEnabled(false);
    d->levelInput->setEnabled(false);
    d->levelLabel->setEnabled(false);
    d->iterationInput->setEnabled(false);
    d->iterationLabel->setEnabled(false);

    int l = d->levelInput->value();
    int f = d->iterationInput->value();
    int e = d->effectType->currentIndex();

    ImageIface* iface = d->previewWidget->imageIface();
    DImg image        = iface->getPreview();

    setFilter(new DistortionFXFilter(&image, this, e, l, f));
}

void DistortionFXTool::prepareFinal()
{
    d->effectTypeLabel->setEnabled(false);
    d->effectType->setEnabled(false);
    d->levelInput->setEnabled(false);
    d->levelLabel->setEnabled(false);
    d->iterationInput->setEnabled(false);
    d->iterationLabel->setEnabled(false);

    int l = d->levelInput->value();
    int f = d->iterationInput->value();
    int e = d->effectType->currentIndex();

    ImageIface iface(0, 0);

    setFilter(new DistortionFXFilter(iface.getOriginal(), this, e, l, f));
}

void DistortionFXTool::putPreviewData()
{
    ImageIface* iface = d->previewWidget->imageIface();
    DImg imDest       = filter()->getTargetImage().smoothScale(iface->previewWidth(), iface->previewHeight());
    iface->putPreview(imDest);

    d->previewWidget->updatePreview();
}

void DistortionFXTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginal(i18n("Distortion Effects"), filter()->filterAction(), filter()->getTargetImage());
}

void DistortionFXTool::renderingFinished()
{
    d->effectTypeLabel->setEnabled(true);
    d->effectType->setEnabled(true);
    d->levelInput->setEnabled(true);
    d->levelLabel->setEnabled(true);
    d->iterationInput->setEnabled(true);
    d->iterationLabel->setEnabled(true);

    switch (d->effectType->currentIndex())
    {
        case DistortionFXFilter::FishEye:
        case DistortionFXFilter::Twirl:
        case DistortionFXFilter::CilindricalHor:
        case DistortionFXFilter::CilindricalVert:
        case DistortionFXFilter::CilindricalHV:
        case DistortionFXFilter::Caricature:
        case DistortionFXFilter::MultipleCorners:
            break;

        case DistortionFXFilter::PolarCoordinates:
        case DistortionFXFilter::UnpolarCoordinates:
            d->levelInput->setEnabled(false);
            d->levelLabel->setEnabled(false);
            break;

        case DistortionFXFilter::WavesHorizontal:
        case DistortionFXFilter::WavesVertical:
        case DistortionFXFilter::BlockWaves1:
        case DistortionFXFilter::BlockWaves2:
        case DistortionFXFilter::CircularWaves1:
        case DistortionFXFilter::CircularWaves2:
        case DistortionFXFilter::Tile:
            d->iterationInput->setEnabled(true);
            d->iterationLabel->setEnabled(true);
            break;
    }
}

}  // namespace DigikamFxFiltersImagePlugin
