/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-31
 * Description : Auto-Color correction tool.
 *
 * Copyright (C) 2005-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "autocorrectiontool.moc"

// Qt includes

#include <QGridLayout>
#include <QListWidget>

// KDE includes

#include <kapplication.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "autolevelsfilter.h"
#include "equalizefilter.h"
#include "stretchfilter.h"
#include "normalizefilter.h"
#include "autoexpofilter.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "previewlist.h"

using namespace Digikam;

namespace DigikamColorImagePlugin
{

class AutoCorrectionTool::AutoCorrectionToolPriv
{
public:

    AutoCorrectionToolPriv() :
        destinationPreviewData(0),
        correctionTools(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;
    static const QString configAutoCorrectionFilterEntry;

    uchar*               destinationPreviewData;

    PreviewList*         correctionTools;

    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};
const QString AutoCorrectionTool::AutoCorrectionToolPriv::configGroupName("autocorrection Tool");
const QString AutoCorrectionTool::AutoCorrectionToolPriv::configHistogramChannelEntry("Histogram Channel");
const QString AutoCorrectionTool::AutoCorrectionToolPriv::configHistogramScaleEntry("Histogram Scale");
const QString AutoCorrectionTool::AutoCorrectionToolPriv::configAutoCorrectionFilterEntry("Auto Correction Filter");

// --------------------------------------------------------

AutoCorrectionTool::AutoCorrectionTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new AutoCorrectionToolPriv)
{
    setObjectName("autocorrection");
    setToolName(i18n("Auto Correction"));
    setToolVersion(1);
    setToolIcon(SmallIcon("autocorrection"));
    setToolHelp("autocolorcorrectiontool.anchor");
    setToolCategory(FilterAction::ReproducibleFilter);

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    ImageIface iface(0, 0);
    DImg thumbImage       = iface.getOriginal()->smoothScale(128, 128, Qt::KeepAspectRatio);
    PreviewListItem* item = 0;
    d->gboxSettings       = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBC);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);
    //                                EditorToolSettings::Try);

    // -------------------------------------------------------------

    d->correctionTools = new PreviewList(this);

    item = d->correctionTools->addItem(new AutoLevelsFilter(&thumbImage, iface.getOriginal()),
                                       i18n("Auto Levels"), AutoLevelsCorrection);
    item->setWhatsThis(i18n("<b>Auto Levels</b>:"
                            "<p>This option maximizes the tonal range in the Red, "
                            "Green, and Blue channels. It searches the image shadow and highlight "
                            "limit values and adjusts the Red, Green, and Blue channels "
                            "to a full histogram range.</p>"));

    item = d->correctionTools->addItem(new NormalizeFilter(&thumbImage, iface.getOriginal()),
                                       i18n("Normalize"), NormalizeCorrection);
    item->setWhatsThis(i18n("<b>Normalize</b>:"
                            "<p>This option scales brightness values across the active "
                            "image so that the darkest point becomes black, and the "
                            "brightest point becomes as bright as possible without "
                            "altering its hue. This is often a \"magic fix\" for "
                            "images that are dim or washed out.</p>"));


    item = d->correctionTools->addItem(new EqualizeFilter(&thumbImage, iface.getOriginal()),
                                       i18n("Equalize"), EqualizeCorrection);
    item->setWhatsThis(i18n("<b>Equalize</b>:"
                            "<p>This option adjusts the brightness of colors across the "
                            "active image so that the histogram for the value channel "
                            "is as nearly as possible flat, that is, so that each possible "
                            "brightness value appears at about the same number of pixels "
                            "as each other value. Sometimes Equalize works wonderfully at "
                            "enhancing the contrasts in an image. Other times it gives "
                            "garbage. It is a very powerful operation, which can either work "
                            "miracles on an image or destroy it.</p>"));

    item = d->correctionTools->addItem(new StretchFilter(&thumbImage, iface.getOriginal()),
                                       i18n("Stretch Contrast"), StretchContrastCorrection);
    item->setWhatsThis(i18n("<b>Stretch Contrast</b>:"
                            "<p>This option enhances the contrast and brightness "
                            "of the RGB values of an image by stretching the lowest "
                            "and highest values to their fullest range, adjusting "
                            "everything in between.</p>"));

    item = d->correctionTools->addItem(new AutoExpoFilter(&thumbImage, iface.getOriginal()),
                                       i18n("Auto Exposure"), AutoExposureCorrection);
    item->setWhatsThis(i18n("<b>Auto Exposure</b>:"
                            "<p>This option enhances the contrast and brightness "
                            "of the RGB values of an image to calculate optimal "
                            "exposition and black level using image histogram "
                            "properties.</p>"));

    d->correctionTools->setFocus();

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout();
    mainLayout->addWidget(d->correctionTools, 0, 0, 1, 5);
    mainLayout->setRowStretch(0, 10);
    mainLayout->setMargin(0);
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->correctionTools, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotEffect()));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotTimer()));
}

AutoCorrectionTool::~AutoCorrectionTool()
{
    if (d->destinationPreviewData)
    {
        delete [] d->destinationPreviewData;
    }

    delete d->correctionTools;
    delete d;
}

void AutoCorrectionTool::slotInit()
{
    EditorToolThreaded::slotInit();
    d->correctionTools->startFilters();
}

void AutoCorrectionTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry, (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,  (int)LogScaleHistogram));
    d->correctionTools->setCurrentId(group.readEntry(d->configAutoCorrectionFilterEntry, (int)AutoLevelsCorrection));
}

void AutoCorrectionTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry,     (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,       (int)d->gboxSettings->histogramBox()->scale());
    group.writeEntry(d->configAutoCorrectionFilterEntry, d->correctionTools->currentId());
    config->sync();
}

void AutoCorrectionTool::slotResetSettings()
{
    d->correctionTools->blockSignals(true);
    d->correctionTools->setCurrentId(AutoLevelsCorrection);
    d->correctionTools->blockSignals(false);

    slotEffect();
}

void AutoCorrectionTool::prepareEffect()
{
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    ImageIface iface(0, 0);
    DImg preview = d->previewWidget->getOriginalRegionImage(true);

    autoCorrection(&preview, iface.getOriginal(), d->correctionTools->currentId());
}

void AutoCorrectionTool::putPreviewData()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    if (d->destinationPreviewData)
    {
        delete [] d->destinationPreviewData;
    }

    d->destinationPreviewData = preview.copyBits();
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData,
            preview.width(), preview.height(), preview.sixteenBit(),
            0, 0, 0, false);
}

void AutoCorrectionTool::prepareFinal()
{
    int type = d->correctionTools->currentId();
    ImageIface iface(0, 0);
    autoCorrection(iface.getOriginal(), iface.getOriginal(), type);
}

void AutoCorrectionTool::putFinalData()
{
    int type = d->correctionTools->currentId();
    QString name;

    switch (type)
    {
        case AutoLevelsCorrection:
            name = i18n("Auto Levels");
            break;

        case NormalizeCorrection:
            name = i18n("Normalize");
            break;

        case EqualizeCorrection:
            name = i18n("Equalize");
            break;

        case StretchContrastCorrection:
            name = i18n("Stretch Contrast");
            break;

        case AutoExposureCorrection:
            name = i18n("Auto Exposure");
            break;
    }

    ImageIface iface(0, 0);
    iface.putOriginalImage(name, filter()->filterAction(), filter()->getTargetImage().bits());
}

void AutoCorrectionTool::autoCorrection(DImg* img, DImg* ref, int type)
{
    switch (type)
    {
        case AutoLevelsCorrection:
        {
            setFilter(new AutoLevelsFilter(img, ref, this));
            break;
        }
        case NormalizeCorrection:
        {
            setFilter(new NormalizeFilter(img, ref, this));
            break;
        }
        case EqualizeCorrection:
        {
            setFilter(new EqualizeFilter(img, ref, this));
            break;
        }
        case StretchContrastCorrection:
        {
            setFilter(new StretchFilter(img, ref, this));
            break;
        }
        case AutoExposureCorrection:
        {
            setFilter(new AutoExpoFilter(img, ref, this));
            break;
        }
    }
}

}  // namespace DigikamColorImagePlugin
