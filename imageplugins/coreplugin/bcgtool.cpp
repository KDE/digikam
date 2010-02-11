/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-05
 * Description : digiKam image editor to adjust Brightness,
                 Contrast, and Gamma of picture.
 *
 * Copyright (C) 2004 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "bcgtool.moc"

// Qt includes

#include <QButtonGroup>
#include <QCheckBox>
#include <QColor>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>

// KDE includes

#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "bcgsettings.h"
#include "colorgradientwidget.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imageregionwidget.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamImagesPluginCore
{

class BCGToolPriv
{
public:

    BCGToolPriv() :
        configGroupName("bcgadjust Tool"),
        configHistogramChannelEntry("Histogram Channel"),
        configHistogramScaleEntry("Histogram Scale"),

        destinationPreviewData(0),
        previewWidget(0),
        gboxSettings(0)
        {}

    const QString       configGroupName;
    const QString       configHistogramChannelEntry;
    const QString       configHistogramScaleEntry;

    uchar*              destinationPreviewData;

    BCGSettings*        bcgSettings;
    ImageRegionWidget*  previewWidget;
    EditorToolSettings* gboxSettings;
};

BCGTool::BCGTool(QObject* parent)
       : EditorToolThreaded(parent),
         d(new BCGToolPriv)
{
    setObjectName("bcgadjust");
    setToolName(i18n("Brightness / Contrast / Gamma"));
    setToolIcon(SmallIcon("contrast"));
    setToolHelp("bcgadjusttool.anchor");

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Try);

    // -------------------------------------------------------------

    d->bcgSettings = new BCGSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->bcgSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------

    d->gboxSettings->enableButton(EditorToolSettings::Ok, false);
}

BCGTool::~BCGTool()
{
    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    delete d;
}

void BCGTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry, (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,  (int)LogScaleHistogram));
    d->bcgSettings->readSettings(group);
}

void BCGTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());
    d->bcgSettings->writeSettings(group);

    config->sync();
}

void BCGTool::slotResetSettings()
{
    d->bcgSettings->resetToDefault();
    slotEffect();
}

void BCGTool::prepareEffect()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    d->bcgSettings->setEnabled(false);
    toolView()->setEnabled(false);

    BCGContainer settings = d->bcgSettings->settings();

    d->gboxSettings->enableButton(EditorToolSettings::Ok, (settings.brightness != 0.0 ||
                                                           settings.contrast != 1.0   ||
                                                           settings.gamma != 1.0));
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage();
    setFilter(dynamic_cast<DImgThreadedFilter*>(new BCGFilter(&preview, this, settings)));
}

void BCGTool::putPreviewData()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    if (d->destinationPreviewData)
       delete [] d->destinationPreviewData;

    d->destinationPreviewData = preview.copyBits();
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData,
                                                             preview.width(), preview.height(), preview.sixteenBit(),
                                                             0, 0, 0, false);
}

void BCGTool::prepareFinal()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    d->bcgSettings->setEnabled(false);
    toolView()->setEnabled(false);

    BCGContainer settings = d->bcgSettings->settings();

    ImageIface iface(0, 0);
    setFilter(dynamic_cast<DImgThreadedFilter*>(new BCGFilter(iface.getOriginalImg(), this, settings)));
}

void BCGTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Brightness / Contrast / Gamma"), filter()->getTargetImage().bits());
}

void BCGTool::renderingFinished()
{
    QApplication::restoreOverrideCursor();
    d->bcgSettings->setEnabled(true);
    toolView()->setEnabled(true);
}

}  // namespace DigikamImagesPluginCore
