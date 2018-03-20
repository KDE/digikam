/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-26
 * Description : image channels mixer.
 *
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

#include "channelmixertool.h"

// C++ includes

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cerrno>

// Qt includes

#include <QVBoxLayout>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "colorgradientwidget.h"
#include "dimg.h"
#include "mixerfilter.h"
#include "mixersettings.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "dnuminput.h"

namespace Digikam
{

class ChannelMixerTool::Private
{
public:

    Private() :
        settingsView(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    MixerSettings*       settingsView;

    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};

const QString ChannelMixerTool::Private::configGroupName(QLatin1String("channelmixer Tool"));
const QString ChannelMixerTool::Private::configHistogramChannelEntry(QLatin1String("Histogram Channel"));
const QString ChannelMixerTool::Private::configHistogramScaleEntry(QLatin1String("Histogram Scale"));

// --------------------------------------------------------

ChannelMixerTool::ChannelMixerTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("channelmixer"));
    setToolName(i18n("Channel Mixer"));
    setToolIcon(QIcon::fromTheme(QLatin1String("channelmixer")));

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBC);

    // -------------------------------------------------------------

    QVBoxLayout* vbox = new QVBoxLayout(d->gboxSettings->plainPage());
    d->settingsView   = new MixerSettings(d->gboxSettings->plainPage());
    vbox->addWidget(d->settingsView);
    vbox->addStretch(10);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(0);

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));

    connect(d->settingsView, SIGNAL(signalOutChannelChanged()),
            this, SLOT(slotOutChannelChanged()));
}

ChannelMixerTool::~ChannelMixerTool()
{
    delete d;
}

void ChannelMixerTool::slotOutChannelChanged()
{
    if (d->settingsView->settings().bMonochrome)
    {
        d->gboxSettings->histogramBox()->setGradientColors(QColor("black"), QColor("white"));
    }

//    d->settingsView->setCurrentChannel(d->gboxSettings->histogramBox()->channel());
}

void ChannelMixerTool::preparePreview()
{
    MixerContainer settings = d->settingsView->settings();

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new MixerFilter(&preview, this, settings));
}

void ChannelMixerTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void ChannelMixerTool::prepareFinal()
{
    MixerContainer settings = d->settingsView->settings();

    ImageIface iface;
    setFilter(new MixerFilter(iface.original(), this, settings));
}

void ChannelMixerTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Channel Mixer"), filter()->filterAction(), filter()->getTargetImage());
}

void ChannelMixerTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->readSettings(group);

    // we need to call these methods here, otherwise the histogram will not be updated correctly
    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)RedChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    slotPreview();
}

void ChannelMixerTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->writeSettings(group);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());

    config->sync();
}

void ChannelMixerTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
}

void ChannelMixerTool::slotLoadSettings()
{
    d->settingsView->loadSettings();
    d->gboxSettings->histogramBox()->setChannel((ChannelType)d->settingsView->currentChannel());
}

void ChannelMixerTool::slotSaveAsSettings()
{
    d->settingsView->saveAsSettings();
}

}  // namespace Digikam
