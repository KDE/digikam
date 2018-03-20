/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-11
 * Description : a digiKam image editor tool to correct
 *               image white balance
 *
 * Copyright (C) 2008-2009 by Guillaume Castagnino <casta at xwing dot info>
 * Copyright (C) 2005-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "whitebalancetool.h"

// Qt includes

#include <QString>
#include <QIcon>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

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

namespace Digikam
{

class WhiteBalanceTool::Private
{
public:

    Private() :
        settingsView(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    static const QString configGroupName;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    WBSettings*          settingsView;

    ImageRegionWidget*   previewWidget;

    EditorToolSettings*  gboxSettings;
};

const QString WhiteBalanceTool::Private::configGroupName(QLatin1String("whitebalance Tool"));
const QString WhiteBalanceTool::Private::configHistogramChannelEntry(QLatin1String("Histogram Chanel"));
const QString WhiteBalanceTool::Private::configHistogramScaleEntry(QLatin1String("Histogram Scale"));

// --------------------------------------------------------

WhiteBalanceTool::WhiteBalanceTool(QObject* const parent)
    : EditorToolThreaded(parent), d(new Private)
{
    setObjectName(QLatin1String("whitebalance"));
    setToolName(i18n("White Balance"));
    setToolIcon(QIcon::fromTheme(QLatin1String("bordertool")));
    setInitPreview(true);

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

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));

    connect(d->settingsView, SIGNAL(signalAutoAdjustExposure()),
            this, SLOT(slotAutoAdjustExposure()));

    connect(d->settingsView, SIGNAL(signalPickerColorButtonActived()),
            this, SLOT(slotPickerColorButtonActived()));

    connect(d->previewWidget, SIGNAL(signalCapturedPointFromOriginal(Digikam::DColor,QPoint)),
            this, SLOT(slotColorSelectedFromOriginal(Digikam::DColor)));
/*
    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget(Digikam::DColor,QPoint)),
            this, SLOT(slotColorSelectedFromTarget(Digikam::DColor)));
*/
}

WhiteBalanceTool::~WhiteBalanceTool()
{
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
    qApp->activeWindow()->setCursor(Qt::WaitCursor);

    ImageIface iface;
    DImg* const img      = iface.original();
    WBContainer settings = d->settingsView->settings();
    WBFilter::autoExposureAdjustement(img, settings.black, settings.expositionMain);
    d->settingsView->setSettings(settings);

    qApp->activeWindow()->unsetCursor();
    slotTimer();
}

void WhiteBalanceTool::preparePreview()
{
    ImageIface iface;
    DImg* const img      = iface.original();
    WBContainer settings = d->settingsView->settings();
    WBFilter::findChanelsMax(img, settings.maxr, settings.maxg, settings.maxb);

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new WBFilter(&preview, this, settings));
}

void WhiteBalanceTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void WhiteBalanceTool::prepareFinal()
{
    WBContainer settings = d->settingsView->settings();

    ImageIface iface;
    setFilter(new WBFilter(iface.original(), this, settings));
}

void WhiteBalanceTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("White Balance"), filter()->filterAction(), filter()->getTargetImage());
}

void WhiteBalanceTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    d->gboxSettings->histogramBox()->setChannel(LuminosityChannel);
    d->gboxSettings->histogramBox()->histogram()->reset();

    slotPreview();
}

void WhiteBalanceTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    d->settingsView->readSettings(group);
}

void WhiteBalanceTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
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
    slotPreview();
}

void WhiteBalanceTool::slotSaveAsSettings()
{
    d->settingsView->saveAsSettings();
}

}  // namespace Digikam
