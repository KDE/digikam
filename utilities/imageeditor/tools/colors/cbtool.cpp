/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-11
 * Description : digiKam image editor Color Balance tool.
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

#include "cbtool.h"

// Qt includes

#include <QLabel>
#include <QIcon>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "cbsettings.h"
#include "cbfilter.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imageregionwidget.h"



namespace Digikam
{

class CBTool::Private
{
public:

    Private() :
        cbSettings(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    CBSettings*          cbSettings;
    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};

const QString CBTool::Private::configGroupName(QLatin1String("colorbalance Tool"));
const QString CBTool::Private::configHistogramChannelEntry(QLatin1String("Histogram Channel"));
const QString CBTool::Private::configHistogramScaleEntry(QLatin1String("Histogram Scale"));

// --------------------------------------------------------

CBTool::CBTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("colorbalance"));
    setToolName(i18n("Color Balance"));
    setToolIcon(QIcon::fromTheme(QLatin1String("adjustrgb")));
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

    d->cbSettings = new CBSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->cbSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

CBTool::~CBTool()
{
    delete d;
}

void CBTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry, (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,  (int)LogScaleHistogram));

    d->cbSettings->readSettings(group);
}

void CBTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());
    d->cbSettings->writeSettings(group);

    group.sync();
}

void CBTool::slotResetSettings()
{
    d->cbSettings->resetToDefault();
    slotPreview();
}

void CBTool::preparePreview()
{
    CBContainer settings = d->cbSettings->settings();
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new CBFilter(&preview, this, settings));
}

void CBTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void CBTool::prepareFinal()
{
    CBContainer settings = d->cbSettings->settings();

    ImageIface iface;
    setFilter(new CBFilter(iface.original(), this, settings));
}

void CBTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Color Balance"), filter()->filterAction(), filter()->getTargetImage());
}

}  // namespace Digikam
