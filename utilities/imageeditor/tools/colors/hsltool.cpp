/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-16
 * Description : digiKam image editor to adjust Hue, Saturation,
 *               and Lightness of picture.
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

#include "hsltool.h"

// Qt includes

#include <QLabel>
#include <QColorDialog>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dimg.h"
#include "hslsettings.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imageregionwidget.h"



namespace Digikam
{

class HSLTool::Private
{
public:

    Private() :
        hslSettings(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    HSLSettings*         hslSettings;
    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};

const QString HSLTool::Private::configGroupName(QLatin1String("hsladjust Tool"));
const QString HSLTool::Private::configHistogramChannelEntry(QLatin1String("Histogram Channel"));
const QString HSLTool::Private::configHistogramScaleEntry(QLatin1String("Histogram Scale"));

// --------------------------------------------------------

HSLTool::HSLTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("adjusthsl"));
    setToolName(i18n("Hue / Saturation / Lightness"));
    setToolIcon(QIcon::fromTheme(QLatin1String("adjusthsl")));
    setToolHelp(QLatin1String("hsladjusttool.anchor"));
    setInitPreview(true);

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

    // -------------------------------------------------------------

    d->hslSettings = new HSLSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->hslSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

HSLTool::~HSLTool()
{
    delete d;
}

void HSLTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry, (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,  (int)LogScaleHistogram));
    d->hslSettings->readSettings(group);
}

void HSLTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());
    d->hslSettings->writeSettings(group);

    config->sync();
}

void HSLTool::slotResetSettings()
{
    d->hslSettings->resetToDefault();
    slotPreview();
}

void HSLTool::preparePreview()
{
    HSLContainer settings = d->hslSettings->settings();
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new HSLFilter(&preview, this, settings));
}

void HSLTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void HSLTool::prepareFinal()
{
    HSLContainer settings = d->hslSettings->settings();

    ImageIface iface;
    setFilter(new HSLFilter(iface.original(), this, settings));
}

void HSLTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("HSL Adjustments"), filter()->filterAction(), filter()->getTargetImage());
}

}  // namespace Digikam
