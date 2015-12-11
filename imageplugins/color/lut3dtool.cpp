/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-10-10
 * Description : Lut3D color adjustment tool.
 *
 * Copyright (C) 2015 by Andrej Krutak <dev at andree dot sk>
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

#include "lut3dtool.h"

// Qt includes

#include <QApplication>
#include <QFileInfo>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "lut3dsettings.h"
#include "lut3dfilter.h"
#include "editortoolsettings.h"
#include "imageregionwidget.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imageiface.h"
#include "dnuminput.h"

using namespace Digikam;

namespace DigikamColorImagePlugin
{

class Lut3DTool::Private
{
public:

    Private() :
        previewWidget(0),
        gboxSettings(0)
    {
    }

    static const QString configGroupName;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    ImageRegionWidget*   previewWidget;
    Lut3DSettings*       settingsView;
    EditorToolSettings*  gboxSettings;
};

const QString Lut3DTool::Private::configGroupName(QLatin1String("lut3d Tool"));
const QString Lut3DTool::Private::configHistogramChannelEntry(QLatin1String("Histogram Channel"));
const QString Lut3DTool::Private::configHistogramScaleEntry(QLatin1String("Histogram Scale"));

// --------------------------------------------------------

Lut3DTool::Lut3DTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("lut3d"));
    setToolName(i18n("Lut3D Color Correction"));
    setToolVersion(1);
    setToolIcon(QIcon::fromTheme(QLatin1String("lut3d")));
    setToolHelp(QLatin1String("autolut3dtool.anchor"));
    setToolCategory(FilterAction::ReproducibleFilter);

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings  = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBC);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    // -------------------------------------------------------------
    d->settingsView  = new Lut3DSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);

    // ----------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

Lut3DTool::~Lut3DTool()
{
    delete d;
}

void Lut3DTool::slotInit()
{
    EditorToolThreaded::slotInit();
    d->settingsView->startPreviewFilters();
}

void Lut3DTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry, (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry, (int)LogScaleHistogram));
    d->settingsView->readSettings(group);
}

void Lut3DTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());
    d->settingsView->writeSettings(group);
    config->sync();
}

void Lut3DTool::slotResetSettings()
{
    d->settingsView->resetToDefault();

    slotPreview();
}

void Lut3DTool::preparePreview()
{
    Lut3DContainer settings = d->settingsView->settings();
    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    ImageIface iface;

    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    applyCorrection(&preview, settings);
}

void Lut3DTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void Lut3DTool::prepareFinal()
{
    Lut3DContainer settings = d->settingsView->settings();
    ImageIface iface;
    applyCorrection(iface.original(), settings);
}

void Lut3DTool::setFinalImage()
{
    Lut3DContainer settings = d->settingsView->settings();

    QString name;
    QFileInfo fi(settings.path);

    name = QLatin1String("Lut3D ") + fi.baseName() + QLatin1String(" / ") +
           QString::number(settings.intensity) + QLatin1Char('%');

    ImageIface iface;
    iface.setOriginal(name, filter()->filterAction(), filter()->getTargetImage());
}

void Lut3DTool::applyCorrection(DImg* const img, const Lut3DContainer &settings)
{
    setFilter(new Lut3DFilter(img, settings));
}

}  // namespace DigikamColorImagePlugin
