/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : a digiKam image plugin to apply special effects.
 *
 * Copyright (C) 2004-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_fxfilters.h"

// Qt includes

#include <QKeySequence>
#include <QAction>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>
#include <kactioncollection.h>

// Local includes

#include "digikam_debug.h"
#include "colorfxtool.h"
#include "charcoaltool.h"
#include "embosstool.h"
#include "oilpainttool.h"
#include "blurfxtool.h"
#include "distortionfxtool.h"
#include "raindroptool.h"
#include "filmgraintool.h"

namespace DigikamFxFiltersImagePlugin
{

K_PLUGIN_FACTORY( FxFiltersFactory, registerPlugin<ImagePlugin_FxFilters>(); )

class ImagePlugin_FxFilters::Private
{
public:

    Private() :
        filmgrainAction(0),
        raindropAction(0),
        distortionfxAction(0),
        blurfxAction(0),
        oilpaintAction(0),
        embossAction(0),
        charcoalAction(0),
        colorEffectsAction(0)
    {
    }

    QAction* filmgrainAction;
    QAction* raindropAction;
    QAction* distortionfxAction;
    QAction* blurfxAction;
    QAction* oilpaintAction;
    QAction* embossAction;
    QAction* charcoalAction;
    QAction* colorEffectsAction;
};

ImagePlugin_FxFilters::ImagePlugin_FxFilters(QObject* const parent, const QVariantList&)
    : ImagePlugin(parent, QLatin1String("ImagePlugin_FxFilters")),
      d(new Private)
{
    // to load the rc file from digikam's installation path
    setComponentName(QLatin1String("digikam"), i18nc("to be displayed in shortcuts dialog", "Filter plugins"));

    d->colorEffectsAction = new QAction(QIcon::fromTheme(QLatin1String("colorfx")), i18n("Color Effects..."), this);
    actionCollection()->addAction(QLatin1String("imageplugin_colorfx"), d->colorEffectsAction);
    connect(d->colorEffectsAction, SIGNAL(triggered(bool)),
            this, SLOT(slotColorEffects()));

    d->charcoalAction = new QAction(QIcon::fromTheme(QLatin1String("charcoaltool")), i18n("Charcoal Drawing..."), this);
    actionCollection()->addAction(QLatin1String("imageplugin_charcoal"), d->charcoalAction);
    connect(d->charcoalAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCharcoal()));

    d->embossAction = new QAction(QIcon::fromTheme(QLatin1String("embosstool")), i18n("Emboss..."), this);
    actionCollection()->addAction(QLatin1String("imageplugin_emboss"), d->embossAction);
    connect(d->embossAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEmboss()));

    d->oilpaintAction = new QAction(QIcon::fromTheme(QLatin1String("oilpaint")), i18n("Oil Paint..."), this);
    actionCollection()->addAction(QLatin1String("imageplugin_oilpaint"), d->oilpaintAction);
    connect(d->oilpaintAction, SIGNAL(triggered(bool)),
            this ,SLOT(slotOilPaint()));

    d->blurfxAction = new QAction(QIcon::fromTheme(QLatin1String("blurfx")), i18n("Blur Effects..."), this);
    actionCollection()->addAction(QLatin1String("imageplugin_blurfx"), d->blurfxAction);
    connect(d->blurfxAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBlurFX()));

    d->distortionfxAction = new QAction(QIcon::fromTheme(QLatin1String("draw-spiral")), i18n("Distortion Effects..."), this);
    actionCollection()->addAction(QLatin1String("imageplugin_distortionfx"), d->distortionfxAction );
    connect(d->distortionfxAction, SIGNAL(triggered(bool)),
            this, SLOT(slotDistortionFX()));

    d->raindropAction = new QAction(QIcon::fromTheme(QLatin1String("raindrop")), i18n("Raindrops..."), this);
    actionCollection()->addAction(QLatin1String("imageplugin_raindrop"), d->raindropAction);
    connect(d->raindropAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRainDrop()));

    d->filmgrainAction  = new QAction(QIcon::fromTheme(QLatin1String("filmgrain")), i18n("Add Film Grain..."), this);
    actionCollection()->addAction(QLatin1String("imageplugin_filmgrain"), d->filmgrainAction);
    connect(d->filmgrainAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFilmGrain()));

    setActionCategory(i18n("Effects"));
    setXMLFile(QLatin1String("digikamimageplugin_fxfilters_ui.rc"));

    qCDebug(DIGIKAM_IMAGEPLUGINS_LOG) << "ImagePlugin_FxFilters plugin loaded";
}

ImagePlugin_FxFilters::~ImagePlugin_FxFilters()
{
    delete d;
}

void ImagePlugin_FxFilters::setEnabledActions(bool b)
{
    d->charcoalAction->setEnabled(b);
    d->colorEffectsAction->setEnabled(b);
    d->embossAction->setEnabled(b);
    d->oilpaintAction->setEnabled(b);
    d->blurfxAction->setEnabled(b);
    d->distortionfxAction->setEnabled(b);
    d->raindropAction->setEnabled(b);
    d->filmgrainAction->setEnabled(b);
}

void ImagePlugin_FxFilters::slotColorEffects()
{
    loadTool(new ColorFxTool(this));
}

void ImagePlugin_FxFilters::slotCharcoal()
{
    loadTool(new CharcoalTool(this));
}

void ImagePlugin_FxFilters::slotEmboss()
{
    loadTool(new EmbossTool(this));
}

void ImagePlugin_FxFilters::slotOilPaint()
{
    loadTool(new OilPaintTool(this));
}

void ImagePlugin_FxFilters::slotBlurFX()
{
    loadTool(new BlurFXTool(this));
}

void ImagePlugin_FxFilters::slotDistortionFX()
{
    loadTool(new DistortionFXTool(this));
}

void ImagePlugin_FxFilters::slotRainDrop()
{
    loadTool(new RainDropTool(this));
}

void ImagePlugin_FxFilters::slotFilmGrain()
{
    loadTool(new FilmGrainTool(this));
}

} // namespace DigikamFxFiltersImagePlugin

#include "imageplugin_fxfilters.moc"
