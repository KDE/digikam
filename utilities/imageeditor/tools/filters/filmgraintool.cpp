/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-26
 * Description : a digiKam image editor tool for add film
 *               grain on an image.
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "filmgraintool.h"

// Qt includes

#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QIcon>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "filmgrainfilter.h"
#include "filmgrainsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace Digikam
{

class FilmGrainTool::Private
{
public:

    Private() :
        configGroupName(QLatin1String("filmgrain Tool")),
        settingsView(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    const QString       configGroupName;

    FilmGrainSettings*  settingsView;

    ImageRegionWidget*  previewWidget;
    EditorToolSettings* gboxSettings;
};

FilmGrainTool::FilmGrainTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d(new Private)
{
    setObjectName(QLatin1String("filmgrain"));
    setToolName(i18n("Film Grain"));
    setToolIcon(QIcon::fromTheme(QLatin1String("filmgrain")));
    setInitPreview(true);

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

    // -------------------------------------------------------------


    connect(d->settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotTimer()));
}

FilmGrainTool::~FilmGrainTool()
{
    delete d;
}

void FilmGrainTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->readSettings(group);
}

void FilmGrainTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->settingsView->writeSettings(group);
    config->sync();
}

void FilmGrainTool::slotResetSettings()
{
    d->settingsView->resetToDefault();
    slotPreview();
}

void FilmGrainTool::preparePreview()
{
    FilmGrainContainer prm = d->settingsView->settings();
    DImg image             = d->previewWidget->getOriginalRegionImage();

    setFilter(new FilmGrainFilter(&image, this, prm));
}

void FilmGrainTool::prepareFinal()
{
    FilmGrainContainer prm = d->settingsView->settings();

    ImageIface iface;
    setFilter(new FilmGrainFilter(iface.original(), this, prm));
}

void FilmGrainTool::setPreviewImage()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void FilmGrainTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Film Grain"), filter()->filterAction(), filter()->getTargetImage());
}

void FilmGrainTool::renderingFinished()
{
    toolSettings()->enableButton(EditorToolSettings::Ok, d->settingsView->settings().isDirty());
}

}  // namespace Digikam
