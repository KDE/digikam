/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-06
 * Description : Red eyes correction tool for image editor
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
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

#include "redeyetool.h"

// Qt includes

#include <QColor>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QToolButton>
#include <QApplication>
#include <QColorDialog>
#include <QIcon>

// KDE includes

#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "editortoolsettings.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "imageiface.h"
#include "redeyecorrectionfilter.h"
#include "redeyecorrectionsettings.h"
#include "imageregionwidget.h"

namespace Digikam
{

class RedEyeTool::Private
{

public:

    Private() :
        settingsView(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    static const QString      configGroupName;
    static const QString      configHistogramChannelEntry;
    static const QString      configHistogramScaleEntry;

    RedEyeCorrectionSettings* settingsView;
    ImageRegionWidget*        previewWidget;
    EditorToolSettings*       gboxSettings;
};

const QString RedEyeTool::Private::configGroupName(QLatin1String("redeye Tool"));
const QString RedEyeTool::Private::configHistogramChannelEntry(QLatin1String("Histogram Channel"));
const QString RedEyeTool::Private::configHistogramScaleEntry(QLatin1String("Histogram Scale"));

// --------------------------------------------------------

RedEyeTool::RedEyeTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("redeye"));
    setToolName(i18n("Red Eye"));
    setToolIcon(QIcon::fromTheme(QLatin1String("redeyes")));
    setToolHelp(QLatin1String("redeyecorrectiontool.anchor"));

    d->previewWidget = new ImageRegionWidget;
    d->previewWidget->setToolTip(i18n("Here you can see the image selection preview with "
                                      "red eye reduction applied."));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBC);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    d->settingsView = new RedEyeCorrectionSettings(d->gboxSettings->plainPage());

    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);
    setToolSettings(d->gboxSettings);

    // -------------------------------------------------------------

    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

RedEyeTool::~RedEyeTool()
{
    delete d;
}

void RedEyeTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry, (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,  (int)LogScaleHistogram));
    d->settingsView->readSettings(group);
}

void RedEyeTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());
    d->settingsView->writeSettings(group);

    config->sync();
}

void RedEyeTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    slotPreview();
}

void RedEyeTool::preparePreview()
{
    RedEyeCorrectionContainer settings = d->settingsView->settings();

    d->gboxSettings->histogramBox()->histogram()->stopHistogramComputation();

    DImg original = d->previewWidget->getOriginalImage();
    setFilter(new RedEyeCorrectionFilter(&original, this, settings));
}

void RedEyeTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage().copy(d->previewWidget->getOriginalImageRegionToRender());
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void RedEyeTool::prepareFinal()
{
    RedEyeCorrectionContainer settings = d->settingsView->settings();

    ImageIface iface;

    setFilter(new RedEyeCorrectionFilter(iface.original(), this, settings));
}

void RedEyeTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Red Eyes Correction"), filter()->filterAction(), filter()->getTargetImage());
}

}  // namespace Digikam
