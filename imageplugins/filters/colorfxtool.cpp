/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : a digiKam image plugin for to apply a color
 *               effect to an image.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "colorfxtool.moc"

// Qt includes

#include <QButtonGroup>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kglobal.h>
#include <khelpmenu.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>

// Local includes

#include "dimg.h"
#include "colorfxfilter.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imageguidewidget.h"

using namespace KDcrawIface;

namespace DigikamFxFiltersImagePlugin
{

class ColorFxTool::ColorFxToolPriv
{

public:

    ColorFxToolPriv() :
        destinationPreviewData(0),
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
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;
    static const QString configEffectTypeEntry;
    static const QString configLevelAdjustmentEntry;
    static const QString configIterationAdjustmentEntry;

    uchar*               destinationPreviewData;

    QLabel*              effectTypeLabel;
    QLabel*              levelLabel;
    QLabel*              iterationLabel;

    RComboBox*           effectType;

    RIntNumInput*        levelInput;
    RIntNumInput*        iterationInput;

    ImageGuideWidget*    previewWidget;
    EditorToolSettings*  gboxSettings;
};
const QString ColorFxTool::ColorFxToolPriv::configGroupName("coloreffect Tool");
const QString ColorFxTool::ColorFxToolPriv::configHistogramChannelEntry("Histogram Channel");
const QString ColorFxTool::ColorFxToolPriv::configHistogramScaleEntry("Histogram Scale");
const QString ColorFxTool::ColorFxToolPriv::configEffectTypeEntry("EffectType");
const QString ColorFxTool::ColorFxToolPriv::configLevelAdjustmentEntry("LevelAdjustment");
const QString ColorFxTool::ColorFxToolPriv::configIterationAdjustmentEntry("IterationAdjustment");

// --------------------------------------------------------

ColorFxTool::ColorFxTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new ColorFxToolPriv)
{
    setObjectName("coloreffects");
    setToolName(i18n("Color Effects"));
    setToolIcon(SmallIcon("colorfx"));

    // -------------------------------------------------------------

    d->previewWidget = new ImageGuideWidget;
    d->previewWidget->setWhatsThis(i18n("This is the color effects preview"));
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBC);
    
    // -------------------------------------------------------------

    d->effectTypeLabel = new QLabel(i18n("Type:"));
    d->effectType      = new RComboBox();
    d->effectType->addItem(i18n("Solarize"));
    d->effectType->addItem(i18n("Vivid"));
    d->effectType->addItem(i18n("Neon"));
    d->effectType->addItem(i18n("Find Edges"));
    d->effectType->setDefaultIndex(ColorFXFilter::Solarize);
    d->effectType->setWhatsThis(i18n("<p>Select the effect type to apply to the image here.</p>"
                                     "<p><b>Solarize</b>: simulates solarization of photograph.</p>"
                                     "<p><b>Vivid</b>: simulates the Velvia(tm) slide film colors.</p>"
                                     "<p><b>Neon</b>: coloring the edges in a photograph to "
                                     "reproduce a fluorescent light effect.</p>"
                                     "<p><b>Find Edges</b>: detects the edges in a photograph "
                                     "and their strength.</p>"));

    d->levelLabel = new QLabel(i18nc("level of the effect", "Level:"));
    d->levelInput = new RIntNumInput();
    d->levelInput->setRange(0, 100, 1);
    d->levelInput->setSliderEnabled(true);
    d->levelInput->setDefaultValue(0);
    d->levelInput->setWhatsThis( i18n("Set here the level of the effect."));

    d->iterationLabel = new QLabel(i18n("Iteration:"));
    d->iterationInput = new RIntNumInput();
    d->iterationInput->setRange(0, 100, 1);
    d->iterationInput->setSliderEnabled(true);
    d->iterationInput->setDefaultValue(0);
    d->iterationInput->setWhatsThis( i18n("This value controls the number of iterations "
                                          "to use with the Neon and Find Edges effects."));

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->addWidget(d->effectTypeLabel, 0, 0, 1, 5);
    mainLayout->addWidget(d->effectType,      1, 0, 1, 5);
    mainLayout->addWidget(d->levelLabel,      2, 0, 1, 5);
    mainLayout->addWidget(d->levelInput,      3, 0, 1, 5);
    mainLayout->addWidget(d->iterationLabel,  4, 0, 1, 5);
    mainLayout->addWidget(d->iterationInput,  5, 0, 1, 5);
    mainLayout->setRowStretch(6, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget(Digikam::DColor,QPoint)),
            this, SLOT(slotColorSelectedFromTarget(Digikam::DColor)));

    connect(d->levelInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->iterationInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotTimer()));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    connect(d->effectType, SIGNAL(activated(int)),
            this, SLOT(slotEffectTypeChanged(int)));
}

ColorFxTool::~ColorFxTool()
{
    if (d->destinationPreviewData)
    {
        delete [] d->destinationPreviewData;
    }

    delete d;
}

void ColorFxTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    d->effectType->setCurrentIndex(group.readEntry(d->configEffectTypeEntry,       d->effectType->defaultIndex()));
    d->levelInput->setValue(group.readEntry(d->configLevelAdjustmentEntry,         d->levelInput->defaultValue()));
    d->iterationInput->setValue(group.readEntry(d->configIterationAdjustmentEntry, d->iterationInput->defaultValue()));
    slotEffectTypeChanged(d->effectType->currentIndex());  //check for enable/disable of iteration
}

void ColorFxTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry,    (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,      (int)d->gboxSettings->histogramBox()->scale());

    group.writeEntry(d->configEffectTypeEntry,          d->effectType->currentIndex());
    group.writeEntry(d->configLevelAdjustmentEntry,     d->levelInput->value());
    group.writeEntry(d->configIterationAdjustmentEntry, d->iterationInput->value());

    group.sync();
}

void ColorFxTool::slotResetSettings()
{
    d->effectType->blockSignals(true);
    d->levelInput->blockSignals(true);
    d->iterationInput->blockSignals(true);

    d->effectType->slotReset();
    d->levelInput->slotReset();
    d->iterationInput->slotReset();

    d->effectType->blockSignals(false);
    d->levelInput->blockSignals(false);
    d->iterationInput->blockSignals(false);

    slotEffect();
}

void ColorFxTool::slotColorSelectedFromTarget(const DColor& color)
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void ColorFxTool::slotEffectTypeChanged(int type)
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
        case ColorFXFilter::Solarize:
            d->levelInput->setRange(0, 100, 1);
            d->levelInput->setSliderEnabled(true);
            d->levelInput->setValue(0);
            d->iterationInput->setEnabled(false);
            d->iterationLabel->setEnabled(false);
            break;

        case ColorFXFilter::Vivid:
            d->levelInput->setRange(0, 50, 1);
            d->levelInput->setSliderEnabled(true);
            d->levelInput->setValue(5);
            d->iterationInput->setEnabled(false);
            d->iterationLabel->setEnabled(false);
            break;

        case ColorFXFilter::Neon:
        case ColorFXFilter::FindEdges:
            d->levelInput->setRange(0, 5, 1);
            d->levelInput->setSliderEnabled(true);
            d->levelInput->setValue(3);
            d->iterationInput->setEnabled(true);
            d->iterationLabel->setEnabled(true);
            d->iterationInput->setRange(0, 5, 1);
            d->iterationInput->setSliderEnabled(true);
            d->iterationInput->setValue(2);
            break;
    }

    d->levelInput->blockSignals(false);
    d->iterationInput->blockSignals(false);

    slotEffect();
}

void ColorFxTool::prepareEffect()
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
    DImg image        = iface->getPreviewImg();

    setFilter(new ColorFXFilter(&image, this, e, l, f));
}

void ColorFxTool::prepareFinal()
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

    setFilter(new ColorFXFilter(iface.getOriginalImg(), this, e, l, f));
}

void ColorFxTool::putPreviewData()
{
    ImageIface* iface = d->previewWidget->imageIface();
    DImg preview = filter()->getTargetImage();
    DImg imDest  = preview.smoothScale(iface->previewWidth(), iface->previewHeight());
    iface->putPreviewImage(imDest.bits());
    d->gboxSettings->histogramBox()->histogram()->updateData(preview.bits(), preview.width(), preview.height(),
            preview.sixteenBit(), 0, 0, 0, false);

    d->previewWidget->updatePreview();
}

void ColorFxTool::putFinalData()
{
    ImageIface iface(0, 0);

    QString name;

    switch (d->effectType->currentIndex())
    {
        case ColorFXFilter::Solarize:
            name = i18n("Solarize");
            break;

        case ColorFXFilter::Vivid:
            name = i18n("Vivid");
            break;

        case ColorFXFilter::Neon:
            name = i18n("Neon");
            break;

        case ColorFXFilter::FindEdges:
            name = i18n("Find Edges");
            break;
    }

    iface.putOriginalImage(name, filter()->filterAction(), filter()->getTargetImage().bits());
}

void ColorFxTool::renderingFinished()
{
    d->effectTypeLabel->setEnabled(true);
    d->effectType->setEnabled(true);
    d->levelInput->setEnabled(true);
    d->levelLabel->setEnabled(true);
    d->iterationInput->setEnabled(true);
    d->iterationLabel->setEnabled(true);

    switch (d->effectType->currentIndex())
    {
        case ColorFXFilter::Solarize:
        case ColorFXFilter::Vivid:
            d->iterationInput->setEnabled(false);
            d->iterationLabel->setEnabled(false);
            break;

        case ColorFXFilter::Neon:
        case ColorFXFilter::FindEdges:
            d->iterationInput->setEnabled(true);
            d->iterationLabel->setEnabled(true);
            break;
    }
}

}  // namespace DigikamFxFiltersImagePlugin

