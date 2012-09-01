/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-11
 * Description : digiKam image editor Color Balance tool.
 *
 * Copyright (C) 2004-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "cbtool.moc"

// Qt includes

#include <QLabel>

// KDE includes

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

// Local includes

#include "cbsettings.h"
#include "cbfilter.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imageregionwidget.h"

using namespace KDcrawIface;

namespace DigikamColorImagePlugin
{

class CBTool::CBToolPriv
{
public:

    CBToolPriv() :
        destinationPreviewData(0),
        cbSettings(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    uchar*               destinationPreviewData;

    CBSettings*          cbSettings;
    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};
const QString CBTool::CBToolPriv::configGroupName("colorbalance Tool");
const QString CBTool::CBToolPriv::configHistogramChannelEntry("Histogram Channel");
const QString CBTool::CBToolPriv::configHistogramScaleEntry("Histogram Scale");

// --------------------------------------------------------

CBTool::CBTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new CBToolPriv)
{
    setObjectName("colorbalance");
    setToolName(i18n("Color Balance"));
    setToolIcon(SmallIcon("adjustrgb"));

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBC);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    d->cbSettings = new CBSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->cbSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    slotTimer();
}

CBTool::~CBTool()
{
    if (d->destinationPreviewData)
    {
        delete [] d->destinationPreviewData;
    }

    delete d;
}

void CBTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry, (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,  (int)LogScaleHistogram));

    d->cbSettings->readSettings(group);
}

void CBTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());
    d->cbSettings->writeSettings(group);

    group.sync();
}

void CBTool::slotResetSettings()
{
    d->cbSettings->resetToDefault();
    slotEffect();
}

void CBTool::prepareEffect()
{
    CBContainer settings = d->cbSettings->settings();
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new CBFilter(&preview, this, settings));
}

void CBTool::putPreviewData()
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

void CBTool::prepareFinal()
{
    CBContainer settings = d->cbSettings->settings();

    ImageIface iface(0, 0);
    setFilter(new CBFilter(iface.getOriginal(), this, settings));
}

void CBTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginal(i18n("Color Balance"), filter()->filterAction(), filter()->getTargetImage().bits());
}

}  // namespace DigikamColorImagePlugin
