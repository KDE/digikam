/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-31
 * Description : Auto-Color correction tool.
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QPixmap>

// KDE includes

#include <kapplication.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "dimg.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "whitebalance.h"

using namespace Digikam;

namespace DigikamImagesPluginCore
{

class AutoCorrectionToolPriv
{
public:

    AutoCorrectionToolPriv() :
        configGroupName("autocorrection Tool"),
        configHistogramChannelEntry("Histogram Channel"),
        configHistogramScaleEntry("Histogram Scale"),
        configAutoCorrectionFilterEntry("Auto Correction Filter"),

        destinationPreviewData(0),
        correctionTools(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString       configGroupName;
    const QString       configHistogramChannelEntry;
    const QString       configHistogramScaleEntry;
    const QString       configAutoCorrectionFilterEntry;

    uchar*              destinationPreviewData;

    QListWidget*        correctionTools;

    ImageWidget*        previewWidget;
    EditorToolSettings* gboxSettings;
    DImg                thumbnailImage;
};

AutoCorrectionTool::AutoCorrectionTool(QObject* parent)
                  : EditorTool(parent),
                    d(new AutoCorrectionToolPriv)
{
    setObjectName("autocorrection");
    setToolName(i18n("Auto Correction"));
    setToolIcon(SmallIcon("autocorrection"));
    setToolHelp("autocolorcorrectiontool.anchor");

    d->destinationPreviewData = 0;

    // -------------------------------------------------------------

    d->previewWidget = new ImageWidget("autocorrection Tool", 0,
                                      i18n("The auto-color correction tool preview is shown "
                                           "here. Picking a color on the image will show "
                                           "the corresponding color level on the histogram."));
    setToolView(d->previewWidget);

    // -------------------------------------------------------------

    ImageIface iface(0, 0);
    d->thumbnailImage = iface.getOriginalImg()->smoothScale(128, 128, Qt::KeepAspectRatio);

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);

    // -------------------------------------------------------------

    d->correctionTools = new QListWidget();
    d->correctionTools->setIconSize(QSize(128, 128));

    QPixmap pix = getThumbnailForEffect(AutoLevelsCorrection);
    QListWidgetItem *item = new QListWidgetItem(QIcon(pix), i18n("Auto Levels"));
    item->setWhatsThis(i18n("<b>Auto Levels</b>:"
                            "<p>This option maximizes the tonal range in the Red, "
                            "Green, and Blue channels. It searches the image shadow and highlight "
                            "limit values and adjusts the Red, Green, and Blue channels "
                            "to a full histogram range.</p>"));
    d->correctionTools->insertItem(AutoLevelsCorrection, item);

    pix = getThumbnailForEffect(NormalizeCorrection);
    item = new QListWidgetItem(QIcon(pix), i18n("Normalize"));
    item->setWhatsThis(i18n("<b>Normalize</b>:"
                            "<p>This option scales brightness values across the active "
                            "image so that the darkest point becomes black, and the "
                            "brightest point becomes as bright as possible without "
                            "altering its hue. This is often a \"magic fix\" for "
                            "images that are dim or washed out.</p>"));
    d->correctionTools->insertItem(NormalizeCorrection, item);

    pix = getThumbnailForEffect(EqualizeCorrection);
    item = new QListWidgetItem(QIcon(pix), i18n("Equalize"));
    item->setWhatsThis(i18n("<b>Equalize</b>:"
                            "<p>This option adjusts the brightness of colors across the "
                            "active image so that the histogram for the value channel "
                            "is as nearly as possible flat, that is, so that each possible "
                            "brightness value appears at about the same number of pixels "
                            "as each other value. Sometimes Equalize works wonderfully at "
                            "enhancing the contrasts in an image. Other times it gives "
                            "garbage. It is a very powerful operation, which can either work "
                            "miracles on an image or destroy it.</p>"));
    d->correctionTools->insertItem(EqualizeCorrection, item);

    pix = getThumbnailForEffect(StretchContrastCorrection);
    item = new QListWidgetItem(QIcon(pix), i18n("Stretch Contrast"));
    item->setWhatsThis(i18n("<b>Stretch Contrast</b>:"
                            "<p>This option enhances the contrast and brightness "
                            "of the RGB values of an image by stretching the lowest "
                            "and highest values to their fullest range, adjusting "
                            "everything in between.</p>"));
    d->correctionTools->insertItem(StretchContrastCorrection, item);

    pix = getThumbnailForEffect(AutoExposureCorrection);
    item = new QListWidgetItem(QIcon(pix), i18n("Auto Exposure"));
    item->setWhatsThis(i18n("<b>Auto Exposure</b>:"
                            "<p>This option enhances the contrast and brightness "
                            "of the RGB values of an image to calculate optimal "
                            "exposition and black level using image histogram "
                            "properties.</p>"));
    d->correctionTools->insertItem(AutoExposureCorrection, item);

    // -------------------------------------------------------------

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

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(d->correctionTools, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotEffect()));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));
}

AutoCorrectionTool::~AutoCorrectionTool()
{
    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    delete d;
}

void AutoCorrectionTool::slotColorSelectedFromTarget(const DColor& color)
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void AutoCorrectionTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel(group.readEntry(d->configHistogramChannelEntry,
                        (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
                        (int)LogScaleHistogram));

    d->correctionTools->setCurrentRow(group.readEntry(d->configAutoCorrectionFilterEntry,
                                                      (int)AutoLevelsCorrection));
}

void AutoCorrectionTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry,     d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,       (int)d->gboxSettings->histogramBox()->scale());
    group.writeEntry(d->configAutoCorrectionFilterEntry, d->correctionTools->currentRow());
    d->previewWidget->writeSettings();
    config->sync();
}

void AutoCorrectionTool::slotResetSettings()
{
    d->correctionTools->blockSignals(true);
    d->correctionTools->setCurrentRow(AutoLevelsCorrection);
    d->correctionTools->blockSignals(false);

    slotEffect();
}

void AutoCorrectionTool::slotEffect()
{
    kapp->setOverrideCursor(Qt::WaitCursor);

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    ImageIface* iface         = d->previewWidget->imageIface();
    d->destinationPreviewData = iface->getPreviewImage();
    int w                     = iface->previewWidth();
    int h                     = iface->previewHeight();
    bool sb                   = iface->previewSixteenBit();

    autoCorrection(d->destinationPreviewData, w, h, sb, d->correctionTools->currentRow());

    iface->putPreviewImage(d->destinationPreviewData);
    d->previewWidget->updatePreview();

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

QPixmap AutoCorrectionTool::getThumbnailForEffect(AutoCorrectionType type)
{
    DImg thumb = d->thumbnailImage.copy();
    autoCorrection(thumb.bits(), thumb.width(), thumb.height(), thumb.sixteenBit(), type);
    return (thumb.convertToPixmap());
}


void AutoCorrectionTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    ImageIface* iface = d->previewWidget->imageIface();
    uchar *data       = iface->getOriginalImage();
    int w             = iface->originalWidth();
    int h             = iface->originalHeight();
    bool sb           = iface->originalSixteenBit();

    if (data)
    {
       int type = d->correctionTools->currentRow();
       autoCorrection(data, w, h, sb, type);
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

        iface->putOriginalImage(name, data);
        delete[] data;
    }

    kapp->restoreOverrideCursor();
}

void AutoCorrectionTool::autoCorrection(uchar *data, int w, int h, bool sb, int type)
{
    DImgImageFilters filter;

    switch (type)
    {
        case AutoLevelsCorrection:
            filter.autoLevelsCorrectionImage(data, w, h, sb);
            break;

        case NormalizeCorrection:
            filter.normalizeImage(data, w, h, sb);
            break;

        case EqualizeCorrection:
            filter.equalizeImage(data, w, h, sb);
            break;

        case StretchContrastCorrection:
            filter.stretchContrastImage(data, w, h, sb);
            break;

        case AutoExposureCorrection:
            WhiteBalance wbFilter(sb);
            double blackLevel;
            double exposureLevel;
            wbFilter.autoExposureAdjustement(data, w, h, sb, blackLevel, exposureLevel);
            wbFilter.whiteBalance(data, w, h, sb, blackLevel, exposureLevel);
            break;
    }
}

}  // namespace DigikamImagesPluginCore

