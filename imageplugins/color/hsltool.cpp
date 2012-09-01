/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-16
 * Description : digiKam image editor to adjust Hue, Saturation,
 *               and Lightness of picture.
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

#include "hsltool.moc"

// Qt includes

#include <QLabel>

// KDE includes

#include <kapplication.h>
#include <kcolordialog.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// Local includes

#include "dimg.h"
#include "hslsettings.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imageregionwidget.h"

using namespace KDcrawIface;

namespace DigikamColorImagePlugin
{

class HSLTool::HSLToolPriv
{
public:

    HSLToolPriv() :
        destinationPreviewData(0),
        hslSettings(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    uchar*               destinationPreviewData;

    HSLSettings*         hslSettings;
    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};
const QString HSLTool::HSLToolPriv::configGroupName("hsladjust Tool");
const QString HSLTool::HSLToolPriv::configHistogramChannelEntry("Histogram Channel");
const QString HSLTool::HSLToolPriv::configHistogramScaleEntry("Histogram Scale");

// --------------------------------------------------------

HSLTool::HSLTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new HSLToolPriv)
{
    setObjectName("adjusthsl");
    setToolName(i18n("Hue / Saturation / Lightness"));
    setToolIcon(SmallIcon("adjusthsl"));
    setToolHelp("hsladjusttool.anchor");

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
    init();

    // -------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    connect(d->hslSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));

    slotTimer();
}

HSLTool::~HSLTool()
{
    if (d->destinationPreviewData)
    {
        delete [] d->destinationPreviewData;
    }

    delete d;
}

void HSLTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry, (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,  (int)LogScaleHistogram));
    d->hslSettings->readSettings(group);
}

void HSLTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());
    d->hslSettings->writeSettings(group);

    config->sync();
}

void HSLTool::slotResetSettings()
{
    d->hslSettings->resetToDefault();
    slotEffect();
}

void HSLTool::prepareEffect()
{
    HSLContainer settings = d->hslSettings->settings();
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new HSLFilter(&preview, this, settings));
}

void HSLTool::putPreviewData()
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

void HSLTool::prepareFinal()
{
    HSLContainer settings = d->hslSettings->settings();

    ImageIface iface(0, 0);
    setFilter(new HSLFilter(iface.getOriginal(), this, settings));
}

void HSLTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginal(i18n("HSL Adjustments"), filter()->filterAction(), filter()->getTargetImage());
}

}  // namespace DigikamColorImagePlugin
