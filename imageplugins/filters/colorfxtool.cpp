/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : a digiKam image plugin for to apply a color
 *               effect to an image.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "colorfxtool.moc"

// Qt includes

#include <QButtonGroup>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kglobal.h>
#include <khelpmenu.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kvbox.h>

// Local includes

#include "dimg.h"
#include "colorfxfilter.h"
#include "colorfxsettings.h"
#include "editortoolsettings.h"
#include "histogrambox.h"
#include "histogramwidget.h"
#include "imagehistogram.h"
#include "imageiface.h"
#include "imageregionwidget.h"

using namespace KDcrawIface;

namespace DigikamFxFiltersImagePlugin
{

class ColorFxTool::Private
{

public:

    Private() :
        previewWidget(0),
        gboxSettings(0),
        settingsView(0)
    {
    }

    static const QString configGroupName;
    static const QString configHistogramChannelEntry;
    static const QString configHistogramScaleEntry;

    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
    ColorFXSettings*     settingsView;
};

const QString ColorFxTool::Private::configGroupName("coloreffect Tool");
const QString ColorFxTool::Private::configHistogramChannelEntry("Histogram Channel");
const QString ColorFxTool::Private::configHistogramScaleEntry("Histogram Scale");

// --------------------------------------------------------

ColorFxTool::ColorFxTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName("coloreffects");
    setToolName(i18n("Color Effects"));
    setToolIcon(SmallIcon("colorfx"));

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;
    d->previewWidget->setWhatsThis(i18n("This is the color effects preview"));
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBC);

    // -------------------------------------------------------------

    d->settingsView = new ColorFXSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(spotPositionChangedFromTarget(Digikam::DColor,QPoint)),
            this, SLOT(slotColorSelectedFromTarget(Digikam::DColor)));

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotPreview()));

    connect(d->settingsView, SIGNAL(signalLevelOrIterationChanged()),
            this, SLOT(slotTimer()));
}

ColorFxTool::~ColorFxTool()
{
    delete d;
}

void ColorFxTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry,
            (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,
            (int)LogScaleHistogram));

    d->settingsView->readSettings(group);
}

void ColorFxTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry,    (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,      (int)d->gboxSettings->histogramBox()->scale());

    d->settingsView->writeSettings(group);

    group.sync();
}

void ColorFxTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    slotPreview();
}

void ColorFxTool::slotColorSelectedFromTarget(const DColor& color)
{
    d->gboxSettings->histogramBox()->histogram()->setHistogramGuideByColor(color);
}

void ColorFxTool::preparePreview()
{
    d->settingsView->disable();
    ColorFXContainer prm = d->settingsView->settings();

    DImg preview = d->previewWidget->getOriginalRegionImage(true);

    setFilter(new ColorFXFilter(&preview, this, prm));
}

void ColorFxTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview, DImg(), false);
}

void ColorFxTool::prepareFinal()
{
    d->settingsView->disable();
    ColorFXContainer prm = d->settingsView->settings();

    ImageIface iface;

    setFilter(new ColorFXFilter(iface.original(), this, prm));
}

void ColorFxTool::setFinalImage()
{
    ImageIface iface;

    QString name;

    switch (d->settingsView->settings().colorFXType)
    {
        case ColorFXFilter::Solarize:
            name = i18n("Solarize");
            break;

        case ColorFXFilter::Vivid:
            name = i18n("Vivid");
            break;

        case ColorFXFilter::Neon:
            name = i18n("Neon");
            break;

        case ColorFXFilter::FindEdges:
            name = i18n("Find Edges");
            break;
    }

    iface.setOriginal(name, filter()->filterAction(), filter()->getTargetImage());
}

void ColorFxTool::renderingFinished()
{
    d->settingsView->enable();
}

}  // namespace DigikamFxFiltersImagePlugin
