/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-11
 * Description : a digiKam image editor plugin to correct
 *               image white balance
 *
 * Copyright (C) 2008-2009 by Guillaume Castagnino <casta at xwing dot info>
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

#include "whitebalancetool.moc"

// Qt includes

#include <QString>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kvbox.h>
#include <kiconloader.h>

// Local includes

#include "colorgradientwidget.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "wbfilter.h"
#include "wbsettings.h"

using namespace KDcrawIface;
using namespace Digikam;

namespace DigikamColorImagePlugin
{

class WhiteBalanceTool::WhiteBalanceToolPriv
{
public:

    WhiteBalanceToolPriv() :
        destinationPreviewData(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    uchar*               destinationPreviewData;

    WBSettings*          settingsView;

    ImageRegionWidget*   previewWidget;

    EditorToolSettings*  gboxSettings;
};
const QString WhiteBalanceTool::WhiteBalanceToolPriv::configGroupName("whitebalance Tool");
const QString WhiteBalanceTool::WhiteBalanceToolPriv::configHistogramChannelEntry("Histogram Chanel");
const QString WhiteBalanceTool::WhiteBalanceToolPriv::configHistogramScaleEntry("Histogram Scale");

// --------------------------------------------------------

WhiteBalanceTool::WhiteBalanceTool(QObject* parent)
    : EditorToolThreaded(parent), d(new WhiteBalanceToolPriv)
{
    setObjectName("whitebalance");
    setToolName(i18n("White Balance"));
    setToolIcon(SmallIcon("whitebalance"));

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBC);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    // -------------------------------------------------------------

    d->settingsView = new WBSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));

    connect(d->settingsView, SIGNAL(signalAutoAdjustExposure()),
            this, SLOT(slotAutoAdjustExposure()));

    connect(d->settingsView, SIGNAL(signalPickerColorButtonActived()),
            this, SLOT(slotPickerColorButtonActived()));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    connect(d->previewWidget, SIGNAL(signalCapturedPointFromOriginal(Digikam::DColor,QPoint)),
            this, SLOT(slotColorSelectedFromOriginal(Digikam::DColor)));

    /*
        connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget(Digikam::DColor,QPoint)),
                this, SLOT(slotColorSelectedFromTarget(Digikam::DColor)));
    */

    slotTimer();
}

WhiteBalanceTool::~WhiteBalanceTool()
{
    if (d->destinationPreviewData)
    {
        delete [] d->destinationPreviewData;
    }

    delete d;
}

void WhiteBalanceTool::slotPickerColorButtonActived()
{
    d->previewWidget->setCapturePointMode(true);
}

void WhiteBalanceTool::slotColorSelectedFromOriginal(const DColor& color)
{
    if ( d->settingsView->pickTemperatureIsOn() )
    {
        WBContainer settings = d->settingsView->settings();
        DColor dc            = color;
        QColor tc            = dc.getQColor();

        WBFilter::autoWBAdjustementFromColor(tc, settings.temperature, settings.green);
        d->settingsView->setSettings(settings);

        d->settingsView->setOnPickTemperature(false);
    }
    else
    {
        return;
    }

    d->previewWidget->setCapturePointMode(false);
    slotTimer();
}

void WhiteBalanceTool::slotColorSelectedFromTarget(const DColor& color)
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void WhiteBalanceTool::slotAutoAdjustExposure()
{
    kapp->activeWindow()->setCursor(Qt::WaitCursor);

    ImageIface iface(0, 0);
    DImg* img            = iface.getOriginal();
    WBContainer settings = d->settingsView->settings();
    WBFilter::autoExposureAdjustement(img, settings.black, settings.exposition);
    d->settingsView->setSettings(settings);

    kapp->activeWindow()->unsetCursor();
    slotTimer();
}

void WhiteBalanceTool::prepareEffect()
{
    ImageIface iface(0, 0);
    DImg* img            = iface.getOriginal();
    WBContainer settings = d->settingsView->settings();
    WBFilter::findChanelsMax(img, settings.maxr, settings.maxg, settings.maxb);

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new WBFilter(&preview, this, settings));
}

void WhiteBalanceTool::putPreviewData()
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

void WhiteBalanceTool::prepareFinal()
{
    WBContainer settings = d->settingsView->settings();

    ImageIface iface(0, 0);
    setFilter(new WBFilter(iface.getOriginal(), this, settings));
}

void WhiteBalanceTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginal(i18n("White Balance"), filter()->filterAction(), filter()->getTargetImage().bits());
}

void WhiteBalanceTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    d->gboxSettings->histogramBox()->setChannel(LuminosityChannel);
    d->gboxSettings->histogramBox()->histogram()->reset();

    slotEffect();
}

void WhiteBalanceTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    d->settingsView->readSettings(group);
}

void WhiteBalanceTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());

    d->settingsView->writeSettings(group);

    config->sync();
}

void WhiteBalanceTool::slotLoadSettings()
{
    d->settingsView->loadSettings();
    d->gboxSettings->histogramBox()->histogram()->reset();
    slotEffect();
}

void WhiteBalanceTool::slotSaveAsSettings()
{
    d->settingsView->saveAsSettings();
}

}  // namespace DigikamColorImagePlugin
