/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : Black and White conversion tool.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "bwsepiatool.moc"

// Qt includes

#include <QGridLayout>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "colorgradientwidget.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "bwsepiafilter.h"
#include "bwsepiasettings.h"

using namespace KDcrawIface;

namespace DigikamColorImagePlugin
{

class BWSepiaTool::BWSepiaToolPriv
{

public:

    BWSepiaToolPriv() :
        destinationPreviewData(0),
        bwsepiaSettings(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    uchar*               destinationPreviewData;

    BWSepiaSettings*     bwsepiaSettings;

    ImageRegionWidget*   previewWidget;

    EditorToolSettings*  gboxSettings;
};
const QString BWSepiaTool::BWSepiaToolPriv::configGroupName("convertbw Tool");
const QString BWSepiaTool::BWSepiaToolPriv::configHistogramChannelEntry("Histogram Channel");
const QString BWSepiaTool::BWSepiaToolPriv::configHistogramScaleEntry("Histogram Scale");

// -----------------------------------------------------------------------------------

BWSepiaTool::BWSepiaTool(QObject* parent)
    : EditorToolThreaded(parent), d(new BWSepiaToolPriv)
{
    setObjectName("convertbw");
    setToolName(i18n("Black and White"));
    setToolIcon(SmallIcon("bwtonal"));
    setToolHelp("blackandwhitetool.anchor");

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBC);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs);

    ImageIface iface(0, 0);
    d->bwsepiaSettings = new BWSepiaSettings(d->gboxSettings->plainPage(), iface.getOriginalImg());

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->bwsepiaSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    slotTimer();
}

BWSepiaTool::~BWSepiaTool()
{
    if (d->destinationPreviewData)
    {
        delete [] d->destinationPreviewData;
    }

    delete d;
}

void BWSepiaTool::slotInit()
{
    EditorToolThreaded::slotInit();
    d->bwsepiaSettings->startPreviewFilters();
}

void BWSepiaTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);


    // we need to call the set methods here, otherwise the curve will not be updated correctly
    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    d->bwsepiaSettings->readSettings(group);
    slotScaleChanged();
}

void BWSepiaTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());

    d->bwsepiaSettings->writeSettings(group);

    group.sync();
}

void BWSepiaTool::slotResetSettings()
{
    d->bwsepiaSettings->resetToDefault();
    d->gboxSettings->histogramBox()->histogram()->reset();
    slotEffect();
}

void BWSepiaTool::prepareEffect()
{
    BWSepiaContainer settings = d->bwsepiaSettings->settings();

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new BWSepiaFilter(&preview, this, settings));
}

void BWSepiaTool::putPreviewData()
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

void BWSepiaTool::prepareFinal()
{
    BWSepiaContainer settings = d->bwsepiaSettings->settings();

    ImageIface iface(0, 0);
    setFilter(new BWSepiaFilter(iface.getOriginalImg(), this, settings));
}

void BWSepiaTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("Convert to Black and White"), filter()->filterAction(), filter()->getTargetImage().bits());
}

void BWSepiaTool::slotLoadSettings()
{
    d->bwsepiaSettings->loadSettings();
    d->gboxSettings->histogramBox()->histogram()->reset();
    slotEffect();
}

void BWSepiaTool::slotSaveAsSettings()
{
    d->bwsepiaSettings->saveAsSettings();
}

void BWSepiaTool::slotScaleChanged()
{
    d->bwsepiaSettings->setScaleType(d->gboxSettings->histogramBox()->scale());
}

}  // namespace DigikamColorImagePlugin
