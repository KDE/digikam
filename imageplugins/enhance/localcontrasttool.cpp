/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-09
 * Description : a plugin to enhance image with local contrasts (as human eye does).
 *
 * Copyright (C) 2009      by Julien Pontabry <julien dot pontabry at gmail dot com>
 * Copyright (C) 2009-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "localcontrasttool.moc"

// Qt includes

#include <QCheckBox>
#include <QFile>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QString>
#include <QTextStream>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>

// Local includes

#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "histogramwidget.h"
#include "imageregionwidget.h"
#include "localcontrastfilter.h"
#include "localcontrastsettings.h"
#include "localcontrastcontainer.h"

namespace DigikamEnhanceImagePlugin
{

class LocalContrastTool::Private
{
public:

    Private() :
        settingsView(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    static const QString   configGroupName;
    static const QString   configHistogramChannelEntry;
    static const QString   configHistogramScaleEntry;

    LocalContrastSettings* settingsView;
    ImageRegionWidget*     previewWidget;
    EditorToolSettings*    gboxSettings;
};
const QString LocalContrastTool::Private::configGroupName("localcontrast Tool");
const QString LocalContrastTool::Private::configHistogramChannelEntry("Histogram Channel");
const QString LocalContrastTool::Private::configHistogramScaleEntry("Histogram Scale");

// --------------------------------------------------------

LocalContrastTool::LocalContrastTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName("localcontrast");
    setToolName(i18n("Local Contrast"));
    setToolIcon(SmallIcon("tonemap"));

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
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Try);

    // -------------------------------------------------------------

    d->settingsView = new LocalContrastSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);
}

LocalContrastTool::~LocalContrastTool()
{
    delete d;
}

void LocalContrastTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->gboxSettings->histogramBox()->setChannel((ChannelType)group.readEntry(d->configHistogramChannelEntry, (int)LuminosityChannel));
    d->gboxSettings->histogramBox()->setScale((HistogramScale)group.readEntry(d->configHistogramScaleEntry,  (int)LogScaleHistogram));
    d->settingsView->readSettings(group);
}

void LocalContrastTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configHistogramChannelEntry, (int)d->gboxSettings->histogramBox()->channel());
    group.writeEntry(d->configHistogramScaleEntry,   (int)d->gboxSettings->histogramBox()->scale());
    d->settingsView->writeSettings(group);
    group.sync();
}

void LocalContrastTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
}

void LocalContrastTool::preparePreview()
{
    // See bug #235601 : we cannot use downscaled image to render preview. It will differs than final rendering.
    DImg image = d->previewWidget->getOriginalRegionImage(false);
    setFilter(new LocalContrastFilter(&image, this, d->settingsView->settings()));
}

void LocalContrastTool::prepareFinal()
{
    ImageIface iface;
    setFilter(new LocalContrastFilter(iface.original(), this, d->settingsView->settings()));
}

void LocalContrastTool::setPreviewImage()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    d->gboxSettings->histogramBox()->histogram()->updateData(preview.copy(), DImg(), false);
}

void LocalContrastTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Local Contrast"), filter()->filterAction(), filter()->getTargetImage());
}

void LocalContrastTool::slotLoadSettings()
{
    d->settingsView->loadSettings();
    d->gboxSettings->histogramBox()->histogram()->reset();
    slotPreview();
}

void LocalContrastTool::slotSaveAsSettings()
{
    d->settingsView->saveAsSettings();
}

} // namespace DigikamEnhanceImagePlugin
