/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-01
 * Description : image histogram adjust curves.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "adjustcurvestool.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dimg.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "curvesfilter.h"
#include "curvessettings.h"
#include "imagehistogram.h"
#include "imagecurves.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace Digikam
{

class AdjustCurvesTool::Private
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

    CurvesSettings*      settingsView;
    ImageRegionWidget*   previewWidget;

    EditorToolSettings*  gboxSettings;
};

const QString AdjustCurvesTool::Private::configGroupName(QLatin1String("adjustcurves Tool"));
const QString AdjustCurvesTool::Private::configHistogramChannelEntry(QLatin1String("Histogram Channel"));
const QString AdjustCurvesTool::Private::configHistogramScaleEntry(QLatin1String("Histogram Scale"));

// --------------------------------------------------------

AdjustCurvesTool::AdjustCurvesTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("adjustcurves"));
    setToolName(i18n("Adjust Curves"));
    setToolIcon(QIcon::fromTheme(QLatin1String("adjustcurves")));

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(Digikam::LRGBA);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    // we don't need to use the Gradient widget in this tool
    d->gboxSettings->histogramBox()->setGradientVisible(false);

    // -------------------------------------------------------------

    ImageIface iface;
    QVBoxLayout* vbox = new QVBoxLayout(d->gboxSettings->plainPage());
    d->settingsView   = new CurvesSettings(d->gboxSettings->plainPage(), iface.original());
    d->gboxSettings->histogramBox()->setContentsMargins(d->settingsView->curvesLeftOffset(), 0, 0, 0);
    vbox->addWidget(d->settingsView);
    vbox->addStretch(10);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(0);

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));

    connect(d->gboxSettings, SIGNAL(signalChannelChanged()),
            this, SLOT(slotChannelChanged()));

    connect(d->gboxSettings, SIGNAL(signalScaleChanged()),
            this, SLOT(slotScaleChanged()));

    connect(d->previewWidget, SIGNAL(signalCapturedPointFromOriginal(Digikam::DColor,QPoint)),
            d->settingsView, SLOT(slotSpotColorChanged(Digikam::DColor)));

    connect(d->settingsView, SIGNAL(signalSpotColorChanged()),
            this, SLOT(slotSpotColorChanged()));

    connect(d->settingsView, SIGNAL(signalChannelReset(int)),
            this, SLOT(slotResetCurrentChannel()));

    connect(d->settingsView, SIGNAL(signalPickerChanged(int)),
            this, SLOT(slotPickerColorButtonActived(int)));
/*
    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget(Digikam::DColor,QPoint)),
            this, SLOT(slotColorSelectedFromTarget(Digikam::DColor)));
*/
}

AdjustCurvesTool::~AdjustCurvesTool()
{
    delete d;
}

void AdjustCurvesTool::slotPickerColorButtonActived(int type)
{
    if (type == CurvesBox::NoPicker)
    {
        return;
    }

    d->previewWidget->setCapturePointMode(true);
}

void AdjustCurvesTool::slotSpotColorChanged()
{
    d->previewWidget->setCapturePointMode(false);
    slotPreview();
}

void AdjustCurvesTool::slotColorSelectedFromTarget(const DColor& color)
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void AdjustCurvesTool::slotResetCurrentChannel()
{
    d->gboxSettings->histogramBox()->histogram()->reset();
    slotPreview();
}

void AdjustCurvesTool::slotChannelChanged()
{
    d->settingsView->setCurrentChannel(d->gboxSettings->histogramBox()->channel());
}

void AdjustCurvesTool::slotScaleChanged()
{
    d->settingsView->setScale(d->gboxSettings->histogramBox()->scale());
}

void AdjustCurvesTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    // we need to call the set methods here, otherwise the curve will not be updated correctly
    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    d->settingsView->readSettings(group);

    slotScaleChanged();
    slotChannelChanged();

    slotPreview();
}

void AdjustCurvesTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());

    d->settingsView->writeSettings(group);

    config->sync();
}

void AdjustCurvesTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    d->gboxSettings->histogramBox()->histogram()->reset();
    slotPreview();
}

void AdjustCurvesTool::preparePreview()
{
    CurvesContainer settings = d->settingsView->settings();

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new CurvesFilter(&preview, this, settings));
}

void AdjustCurvesTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void AdjustCurvesTool::prepareFinal()
{
    CurvesContainer settings = d->settingsView->settings();

    ImageIface iface;
    setFilter(new CurvesFilter(iface.original(), this, settings));
}

void AdjustCurvesTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Adjust Curve"), filter()->filterAction(), filter()->getTargetImage());
}

void AdjustCurvesTool::slotLoadSettings()
{
    d->settingsView->loadSettings();

    // Refresh the current curves config.
    slotChannelChanged();
    slotPreview();
}

void AdjustCurvesTool::slotSaveAsSettings()
{
    d->settingsView->saveAsSettings();
}

}  // namespace Digikam
