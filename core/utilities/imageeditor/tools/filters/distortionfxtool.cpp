/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-11
 * Description : a tool to apply Distortion FX to an image.
 *
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

// Qt includes

#include <QFrame>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QSpinBox>
#include <QIcon>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "dimg.h"
#include "dcombobox.h"
#include "distortionfxfilter.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace Digikam
{

class DistortionFXTool::Private
{
public:

    Private() :
        effectTypeLabel(0),
        levelLabel(0),
        iterationLabel(0),
        effectType(0),
        levelInput(0),
        iterationInput(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    static const QString configGroupName;
    static const QString configEffectTypeEntry;
    static const QString configIterationAdjustmentEntry;
    static const QString configLevelAdjustmentEntry;

    QLabel*              effectTypeLabel;
    QLabel*              levelLabel;
    QLabel*              iterationLabel;

    DComboBox*           effectType;

    DIntNumInput*        levelInput;
    DIntNumInput*        iterationInput;

    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};

const QString DistortionFXTool::Private::configGroupName(QLatin1String("distortionfx Tool"));
const QString DistortionFXTool::Private::configEffectTypeEntry(QLatin1String("EffectType"));
const QString DistortionFXTool::Private::configIterationAdjustmentEntry(QLatin1String("IterationAdjustment"));
const QString DistortionFXTool::Private::configLevelAdjustmentEntry(QLatin1String("LevelAdjustment"));

// --------------------------------------------------------

DistortionFXTool::DistortionFXTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("distortionfx"));
    setToolName(i18n("Distortion Effects"));
    setToolIcon(QIcon::fromTheme(QLatin1String("draw-spiral")));

    d->previewWidget = new ImageRegionWidget;
    d->previewWidget->setWhatsThis(i18n("This is the preview of the distortion effect "
                                        "applied to the photograph."));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Try);

    // -------------------------------------------------------------

    d->effectTypeLabel = new QLabel(i18n("Type:"));
    d->effectType      = new DComboBox();
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
    d->levelInput = new DIntNumInput();
    d->levelInput->setRange(0, 100, 1);
    d->levelInput->setDefaultValue(50);
    d->levelInput->setWhatsThis( i18n("Set here the level of the effect."));

    d->iterationLabel = new QLabel(i18n("Iteration:"));
    d->iterationInput = new DIntNumInput();
    d->iterationInput->setRange(0, 100, 1);
    d->iterationInput->setDefaultValue(10);
    d->iterationInput->setWhatsThis( i18n("This value controls the iterations to use for Waves, "
                                          "Tile, and Neon effects."));

    connect(d->effectType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotEffectTypeChanged(int)));

    // -------------------------------------------------------------

    const int spacing = d->gboxSettings->spacingHint();

    QGridLayout* const mainLayout = new QGridLayout();
    mainLayout->addWidget(d->effectTypeLabel,  0, 0, 1, 3);
    mainLayout->addWidget(d->effectType,       1, 0, 1, 3);
    mainLayout->addWidget(d->levelLabel,       2, 0, 1, 3);
    mainLayout->addWidget(d->levelInput,       3, 0, 1, 3);
    mainLayout->addWidget(d->iterationLabel,   4, 0, 1, 3);
    mainLayout->addWidget(d->iterationInput,   5, 0, 1, 3);
    mainLayout->setRowStretch(6, 10);
    mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setSpacing(spacing);
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setPreviewModeMask(PreviewToolBar::AllPreviewModes);
    setToolView(d->previewWidget);
    setToolSettings(d->gboxSettings);

    slotEffectTypeChanged(d->effectType->defaultIndex());
}

DistortionFXTool::~DistortionFXTool()
{
    delete d;
}

void DistortionFXTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    blockWidgetSignals(true);

    d->effectType->setCurrentIndex(group.readEntry(d->configEffectTypeEntry,       (int)DistortionFXFilter::FishEye));
    d->iterationInput->setValue(group.readEntry(d->configIterationAdjustmentEntry, 10));
    d->levelInput->setValue(group.readEntry(d->configLevelAdjustmentEntry,         50));
    slotEffectTypeChanged(d->effectType->defaultIndex());

    blockWidgetSignals(false);

    slotPreview();
}

void DistortionFXTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configEffectTypeEntry,          d->effectType->currentIndex());
    group.writeEntry(d->configIterationAdjustmentEntry, d->iterationInput->value());
    group.writeEntry(d->configLevelAdjustmentEntry,     d->levelInput->value());

    config->sync();
}

void DistortionFXTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->effectType->slotReset();
    d->iterationInput->slotReset();
    d->levelInput->slotReset();
    slotEffectTypeChanged(d->effectType->defaultIndex());

    blockWidgetSignals(false);
}

void DistortionFXTool::slotEffectTypeChanged(int type)
{
    d->levelInput->setEnabled(true);
    d->levelLabel->setEnabled(true);
    d->iterationInput->setEnabled(false);
    d->iterationLabel->setEnabled(false);

    blockWidgetSignals(true);

    d->levelInput->setRange(0, 100, 1);
    d->levelInput->setValue(25);

    switch (type)
    {
        case DistortionFXFilter::Twirl:
            d->levelInput->setRange(-50, 50, 1);
            d->levelInput->setValue(10);
            break;

        case DistortionFXFilter::FishEye:
        case DistortionFXFilter::CilindricalHor:
        case DistortionFXFilter::CilindricalVert:
        case DistortionFXFilter::CilindricalHV:
        case DistortionFXFilter::Caricature:
            d->levelInput->setRange(0, 200, 1);
            d->levelInput->setValue(50);
            break;

        case DistortionFXFilter::MultipleCorners:
            d->levelInput->setRange(1, 10, 1);
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
            d->iterationInput->setValue(10);
            break;

        case DistortionFXFilter::PolarCoordinates:
        case DistortionFXFilter::UnpolarCoordinates:
            d->levelInput->setEnabled(false);
            d->levelLabel->setEnabled(false);
            break;
    }

    blockWidgetSignals(false);
}

void DistortionFXTool::preparePreview()
{
    d->gboxSettings->setEnabled(false);

    int l                   = d->levelInput->value();
    int f                   = d->iterationInput->value();
    int e                   = d->effectType->currentIndex();

    ImageIface iface;
    DImg image = *iface.original();

    setFilter(new DistortionFXFilter(&image, this, e, l, f));
}

void DistortionFXTool::prepareFinal()
{
    d->gboxSettings->setEnabled(false);

    int l = d->levelInput->value();
    int f = d->iterationInput->value();
    int e = d->effectType->currentIndex();

    ImageIface iface;

    setFilter(new DistortionFXFilter(iface.original(), this, e, l, f));
}

void DistortionFXTool::setPreviewImage()
{
    QRect pRect  = d->previewWidget->getOriginalImageRegionToRender();
    DImg destImg = filter()->getTargetImage().copy(pRect);
    d->previewWidget->setPreviewImage(destImg);
}

void DistortionFXTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Distortion Effects"), filter()->filterAction(), filter()->getTargetImage());
}

void DistortionFXTool::renderingFinished()
{
    d->gboxSettings->setEnabled(true);
}

void DistortionFXTool::blockWidgetSignals(bool b)
{
    d->effectType->blockSignals(b);
    d->levelInput->blockSignals(b);
    d->iterationInput->blockSignals(b);
}

}  // namespace Digikam
