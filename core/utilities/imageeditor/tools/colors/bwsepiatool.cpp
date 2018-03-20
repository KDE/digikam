/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : Black and White conversion tool.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "bwsepiatool.h"

// Qt includes

#include <QGridLayout>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dnuminput.h"
#include "colorgradientwidget.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "bwsepiafilter.h"
#include "bwsepiasettings.h"

namespace Digikam
{

class BWSepiaTool::Private
{

public:

    Private() :
        bwsepiaSettings(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    static const QString configGroupName;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    BWSepiaSettings*     bwsepiaSettings;

    ImageRegionWidget*   previewWidget;

    EditorToolSettings*  gboxSettings;
};

const QString BWSepiaTool::Private::configGroupName(QLatin1String("convertbw Tool"));
const QString BWSepiaTool::Private::configHistogramChannelEntry(QLatin1String("Histogram Channel"));
const QString BWSepiaTool::Private::configHistogramScaleEntry(QLatin1String("Histogram Scale"));

// -----------------------------------------------------------------------------------

BWSepiaTool::BWSepiaTool(QObject* const parent)
    : EditorToolThreaded(parent), d(new Private)
{
    setObjectName(QLatin1String("convertbw"));
    setToolName(i18n("Black and White"));
    setToolIcon(QIcon::fromTheme(QLatin1String("bwtonal")));
    setToolHelp(QLatin1String("blackandwhitetool.anchor"));
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
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs);

    ImageIface iface;
    d->bwsepiaSettings = new BWSepiaSettings(d->gboxSettings->plainPage(), iface.original());

    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->bwsepiaSettings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

BWSepiaTool::~BWSepiaTool()
{
    delete d;
}

void BWSepiaTool::slotInit()
{
    EditorToolThreaded::slotInit();
    d->bwsepiaSettings->startPreviewFilters();
}

void BWSepiaTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
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
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
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
    slotPreview();
}

void BWSepiaTool::preparePreview()
{
    BWSepiaContainer settings = d->bwsepiaSettings->settings();

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new BWSepiaFilter(&preview, this, settings));
}

void BWSepiaTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void BWSepiaTool::prepareFinal()
{
    BWSepiaContainer settings = d->bwsepiaSettings->settings();

    ImageIface iface;
    setFilter(new BWSepiaFilter(iface.original(), this, settings));
}

void BWSepiaTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Convert to Black and White"), filter()->filterAction(), filter()->getTargetImage());
}

void BWSepiaTool::slotLoadSettings()
{
    d->bwsepiaSettings->loadSettings();
    d->gboxSettings->histogramBox()->histogram()->reset();
    slotPreview();
}

void BWSepiaTool::slotSaveAsSettings()
{
    d->bwsepiaSettings->saveAsSettings();
}

void BWSepiaTool::slotScaleChanged()
{
    d->bwsepiaSettings->setScaleType(d->gboxSettings->histogramBox()->scale());
}

}  // namespace Digikam
