/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-26
 * Description : image channels mixer.
 *
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

#include "channelmixertool.moc"

// C++ includes

#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cerrno>

// Qt includes

#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kvbox.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

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

using namespace KDcrawIface;

namespace DigikamColorImagePlugin
{

class ChannelMixerTool::ChannelMixerToolPriv
{
public:

    ChannelMixerToolPriv() :
        destinationPreviewData(0),
        settingsView(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    uchar*               destinationPreviewData;

    MixerSettings*       settingsView;

    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};
const QString ChannelMixerTool::ChannelMixerToolPriv::configGroupName("channelmixer Tool");
const QString ChannelMixerTool::ChannelMixerToolPriv::configHistogramChannelEntry("Histogram Channel");
const QString ChannelMixerTool::ChannelMixerToolPriv::configHistogramScaleEntry("Histogram Scale");

// --------------------------------------------------------

ChannelMixerTool::ChannelMixerTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new ChannelMixerToolPriv)
{
    setObjectName("channelmixer");
    setToolName(i18n("Channel Mixer"));
    setToolIcon(SmallIcon("channelmixer"));

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
    d->gboxSettings->setHistogramType(RGB);

    // -------------------------------------------------------------

    QVBoxLayout* vbox = new QVBoxLayout(d->gboxSettings->plainPage());
    d->settingsView   = new MixerSettings(d->gboxSettings->plainPage());
    vbox->addWidget(d->settingsView);
    vbox->addStretch(10);
    vbox->setMargin(0);
    vbox->setSpacing(0);

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    connect(d->gboxSettings, SIGNAL(signalChannelChanged()),
            this, SLOT(slotChannelChanged()));

    connect(d->settingsView, SIGNAL(signalMonochromeActived(bool)),
            this, SLOT(slotMonochromeActived(bool)));
}

ChannelMixerTool::~ChannelMixerTool()
{
    if (d->destinationPreviewData)
    {
        delete [] d->destinationPreviewData;
    }

    delete d;
}

void ChannelMixerTool::slotMonochromeActived(bool mono)
{
    d->gboxSettings->histogramBox()->setChannelEnabled(!mono);
    d->gboxSettings->histogramBox()->setChannel(RedChannel);
}

void ChannelMixerTool::slotChannelChanged()
{
    if (d->settingsView->settings().bMonochrome)
    {
        d->gboxSettings->histogramBox()->setGradientColors(QColor("black"), QColor("white"));
    }

    d->settingsView->setCurrentChannel(d->gboxSettings->histogramBox()->channel());
}

void ChannelMixerTool::prepareEffect()
{
    MixerContainer settings = d->settingsView->settings();

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new MixerFilter(&preview, this, settings));
}

void ChannelMixerTool::putPreviewData()
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

void ChannelMixerTool::prepareFinal()
{
    MixerContainer settings = d->settingsView->settings();

    ImageIface iface(0, 0);
    setFilter(new MixerFilter(iface.getOriginalImg(), this, settings));
}

void ChannelMixerTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Channel Mixer"), filter()->filterAction(), filter()->getTargetImage().bits());
}

void ChannelMixerTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->readSettings(group);

    // we need to call these methods here, otherwise the histogram will not be updated correctly
    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    slotEffect();
}

void ChannelMixerTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
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

}  // namespace DigikamColorImagePlugin
