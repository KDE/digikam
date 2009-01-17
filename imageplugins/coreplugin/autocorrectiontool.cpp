/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-31
 * Description : Auto-Color correction tool.
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "autocorrectiontool.h"
#include "autocorrectiontool.moc"

// Qt includes.

#include <QCheckBox>
#include <QColor>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QTimer>
#include <QToolButton>

// KDE includes.

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// Local includes.

#include "colorgradientwidget.h"
#include "dimg.h"
#include "dimgimagefilters.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imageiface.h"
#include "imagewidget.h"
#include "whitebalance.h"


using namespace Digikam;

namespace DigikamImagesPluginCore
{

AutoCorrectionTool::AutoCorrectionTool(QObject* parent)
                  : EditorTool(parent)
{
    setObjectName("autocorrection");
    setToolName(i18n("Auto Correction"));
    setToolIcon(SmallIcon("autocorrection"));
    setToolHelp("autocolorcorrectiontool.anchor");

    m_destinationPreviewData = 0;

    // -------------------------------------------------------------

    m_previewWidget = new ImageWidget("autocorrection Tool", 0,
                                      i18n("Here you can see the auto-color correction tool "
                                           "preview. You can pick color on image "
                                           "to see the color level corresponding on histogram."));
    setToolView(m_previewWidget);

    // -------------------------------------------------------------

    ImageIface iface(0, 0);
    m_thumbnailImage = iface.getOriginalImg()->smoothScale(128, 128, Qt::ScaleMin);

    m_gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                            EditorToolSettings::Ok|
                                            EditorToolSettings::Cancel,
                                            EditorToolSettings::Histogram);

    QGridLayout* gridSettings = new QGridLayout(m_gboxSettings->plainPage());

    // -------------------------------------------------------------

    m_correctionTools = new QListWidget(m_gboxSettings->plainPage());
    m_correctionTools->setIconSize(QSize(128, 128));

    QPixmap pix = getThumbnailForEffect(AutoLevelsCorrection);
    QListWidgetItem *item = new QListWidgetItem(QIcon(pix), i18n("Auto Levels"));
    item->setWhatsThis(i18n("<b>Auto Levels</b>:"
                            "<p>This option maximizes the tonal range in the Red, "
                            "Green, and Blue channels. It searches the image shadow and highlight "
                            "limit values and adjusts the Red, Green, and Blue channels "
                            "to a full histogram range.</p>"));
    m_correctionTools->insertItem(AutoLevelsCorrection, item);

    pix = getThumbnailForEffect(NormalizeCorrection);
    item = new QListWidgetItem(QIcon(pix), i18n("Normalize"));
    item->setWhatsThis(i18n("<b>Normalize</b>:"
                            "<p>This option scales brightness values across the active "
                            "image so that the darkest point becomes black, and the "
                            "brightest point becomes as bright as possible without "
                            "altering its hue. This is often a \"magic fix\" for "
                            "images that are dim or washed out.</p>"));
    m_correctionTools->insertItem(NormalizeCorrection, item);

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
    m_correctionTools->insertItem(EqualizeCorrection, item);

    pix = getThumbnailForEffect(StretchContrastCorrection);
    item = new QListWidgetItem(QIcon(pix), i18n("Stretch Contrast"));
    item->setWhatsThis(i18n("<b>Stretch Contrast</b>:"
                            "<p>This option enhances the contrast and brightness "
                            "of the RGB values of an image by stretching the lowest "
                            "and highest values to their fullest range, adjusting "
                            "everything in between.</p>"));
    m_correctionTools->insertItem(StretchContrastCorrection, item);

    pix = getThumbnailForEffect(AutoExposureCorrection);
    item = new QListWidgetItem(QIcon(pix), i18n("Auto Exposure"));
    item->setWhatsThis(i18n("<b>Auto Exposure</b>:"
                            "<p>This option enhances the contrast and brightness "
                            "of the RGB values of an image to calculate optimal "
                            "exposition and black level using image histogram "
                            "properties.</p>"));
    m_correctionTools->insertItem(AutoExposureCorrection, item);

    // -------------------------------------------------------------

    m_correctionTools->setFocus();

    gridSettings->addWidget(m_correctionTools, 0, 0, 1, 5);
    gridSettings->setRowStretch(0, 10);
    gridSettings->setMargin(m_gboxSettings->spacingHint());
    gridSettings->setSpacing(m_gboxSettings->spacingHint());

    setToolSettings(m_gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(m_previewWidget, SIGNAL(spotPositionChangedFromTarget( const Digikam::DColor &, const QPoint & )),
            this, SLOT(slotColorSelectedFromTarget( const Digikam::DColor & )));

    connect(m_correctionTools, SIGNAL(itemSelectionChanged()),
            this, SLOT(slotEffect()));

    connect(m_previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));
}

AutoCorrectionTool::~AutoCorrectionTool()
{
    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;
}

void AutoCorrectionTool::slotColorSelectedFromTarget(const DColor &color)
{
    m_gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void AutoCorrectionTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("autocorrection Tool");

    m_gboxSettings->histogramBox()->setChannel(group.readEntry("Histogram Channel",
                        (int)EditorToolSettings::LuminosityChannel));
    m_gboxSettings->histogramBox()->setScale(group.readEntry("Histogram Scale",
                        (int)HistogramWidget::LogScaleHistogram));

    m_correctionTools->setCurrentRow(group.readEntry("Auto Correction Filter", (int)AutoLevelsCorrection));
}

void AutoCorrectionTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("autocorrection Tool");
    group.writeEntry("Histogram Channel", m_gboxSettings->histogramBox()->channel());
    group.writeEntry("Histogram Scale", m_gboxSettings->histogramBox()->scale());
    group.writeEntry("Auto Correction Filter", m_correctionTools->currentRow());
    config->sync();
}

void AutoCorrectionTool::slotResetSettings()
{
    m_correctionTools->blockSignals(true);
    m_correctionTools->setCurrentRow(AutoLevelsCorrection);
    m_correctionTools->blockSignals(false);

    slotEffect();
}

void AutoCorrectionTool::slotEffect()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    m_gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    if (m_destinationPreviewData)
       delete [] m_destinationPreviewData;

    ImageIface* iface        = m_previewWidget->imageIface();
    m_destinationPreviewData = iface->getPreviewImage();
    int w                    = iface->previewWidth();
    int h                    = iface->previewHeight();
    bool sb                  = iface->previewSixteenBit();

    autoCorrection(m_destinationPreviewData, w, h, sb, m_correctionTools->currentRow());

    iface->putPreviewImage(m_destinationPreviewData);
    m_previewWidget->updatePreview();

    // Update histogram.

    m_gboxSettings->histogramBox()->histogram()->updateData(m_destinationPreviewData, w, h, sb, 0, 0, 0, false);

    kapp->restoreOverrideCursor();
}

QPixmap AutoCorrectionTool::getThumbnailForEffect(AutoCorrectionType type)
{
    DImg thumb = m_thumbnailImage.copy();
    autoCorrection(thumb.bits(), thumb.width(), thumb.height(), thumb.sixteenBit(), type);
    return (thumb.convertToPixmap());
}


void AutoCorrectionTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );
    ImageIface* iface = m_previewWidget->imageIface();
    uchar *data                = iface->getOriginalImage();
    int w                      = iface->originalWidth();
    int h                      = iface->originalHeight();
    bool sb                    = iface->originalSixteenBit();

    if (data)
    {
       int type = m_correctionTools->currentRow();
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

