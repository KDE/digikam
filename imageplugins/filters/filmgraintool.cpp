/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor plugin for add film
 *               grain on an image.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "filmgraintool.moc"

// Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLabel>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "dimg.h"
#include "editortoolsettings.h"
#include "filmgrainfilter.h"
#include "filmgrainsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace DigikamFxFiltersImagePlugin
{

class FilmGrainTool::FilmGrainToolPriv
{
public:

    FilmGrainToolPriv() :
        configGroupName("filmgrain Tool"),
        settingsView(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    const QString       configGroupName;

    FilmGrainSettings*  settingsView;

    ImageRegionWidget*  previewWidget;
    EditorToolSettings* gboxSettings;
};

FilmGrainTool::FilmGrainTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new FilmGrainToolPriv)
{
    setObjectName("filmgrain");
    setToolName(i18n("Film Grain"));
    setToolIcon(SmallIcon("filmgrain"));

    d->previewWidget = new ImageRegionWidget;
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings  = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Try);

    d->settingsView = new FilmGrainSettings(d->gboxSettings->plainPage());
    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------


    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));

    connect(d->previewWidget, SIGNAL(signalResized()),
            this, SLOT(slotEffect()));

    // -------------------------------------------------------------

    slotTimer();
}

FilmGrainTool::~FilmGrainTool()
{
    delete d;
}

void FilmGrainTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->readSettings(group);
}

void FilmGrainTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->writeSettings(group);
    config->sync();
}

void FilmGrainTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    slotEffect();
}

void FilmGrainTool::prepareEffect()
{
    FilmGrainContainer prm = d->settingsView->settings();
    DImg image             = d->previewWidget->getOriginalRegionImage();

    setFilter(new FilmGrainFilter(&image, this, prm));
}

void FilmGrainTool::prepareFinal()
{
    FilmGrainContainer prm = d->settingsView->settings();

    ImageIface iface(0, 0);
    setFilter(new FilmGrainFilter(iface.getOriginal(), this, prm));
}

void FilmGrainTool::putPreviewData()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void FilmGrainTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginal(i18n("Film Grain"), filter()->filterAction(), filter()->getTargetImage());
}

void FilmGrainTool::renderingFinished()
{
    toolSettings()->enableButton(EditorToolSettings::Ok, d->settingsView->settings().isDirty());
}

}  // namespace DigikamFxFiltersImagePlugin
