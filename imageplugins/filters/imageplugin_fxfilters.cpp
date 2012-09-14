/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : a digiKam image plugin to apply special effects.
 *
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageplugin_fxfilters.moc"

// KDE includes

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kdebug.h>

// Local includes

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
K_EXPORT_PLUGIN ( FxFiltersFactory("digikamimageplugin_fxfilters") )

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

    KAction* filmgrainAction;
    KAction* raindropAction;
    KAction* distortionfxAction;
    KAction* blurfxAction;
    KAction* oilpaintAction;
    KAction* embossAction;
    KAction* charcoalAction;
    KAction* colorEffectsAction;
};

ImagePlugin_FxFilters::ImagePlugin_FxFilters(QObject* const parent, const QVariantList&)
    : ImagePlugin(parent, "ImagePlugin_FxFilters"),
      d(new Private)
{
    d->colorEffectsAction = new KAction(KIcon("colorfx"), i18n("Color Effects..."), this);
    actionCollection()->addAction("imageplugin_colorfx", d->colorEffectsAction);
    connect(d->colorEffectsAction, SIGNAL(triggered(bool)),
            this, SLOT(slotColorEffects()));

    d->charcoalAction = new KAction(KIcon("charcoaltool"), i18n("Charcoal Drawing..."), this);
    actionCollection()->addAction("imageplugin_charcoal", d->charcoalAction);
    connect(d->charcoalAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCharcoal()));

    d->embossAction = new KAction(KIcon("embosstool"), i18n("Emboss..."), this);
    actionCollection()->addAction("imageplugin_emboss", d->embossAction);
    connect(d->embossAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEmboss()));

    d->oilpaintAction = new KAction(KIcon("oilpaint"), i18n("Oil Paint..."), this);
    actionCollection()->addAction("imageplugin_oilpaint", d->oilpaintAction);
    connect(d->oilpaintAction, SIGNAL(triggered(bool)),
            this ,SLOT(slotOilPaint()));

    d->blurfxAction = new KAction(KIcon("blurfx"), i18n("Blur Effects..."), this);
    actionCollection()->addAction("imageplugin_blurfx", d->blurfxAction);
    connect(d->blurfxAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBlurFX()));

    d->distortionfxAction = new KAction(KIcon("distortionfx"), i18n("Distortion Effects..."), this);
    actionCollection()->addAction("imageplugin_distortionfx", d->distortionfxAction );
    connect(d->distortionfxAction, SIGNAL(triggered(bool)),
            this, SLOT(slotDistortionFX()));

    d->raindropAction = new KAction(KIcon("raindrop"), i18n("Raindrops..."), this);
    actionCollection()->addAction("imageplugin_raindrop", d->raindropAction);
    connect(d->raindropAction, SIGNAL(triggered(bool)),
            this, SLOT(slotRainDrop()));

    d->filmgrainAction  = new KAction(KIcon("filmgrain"), i18n("Add Film Grain..."), this);
    actionCollection()->addAction("imageplugin_filmgrain", d->filmgrainAction);
    connect(d->filmgrainAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFilmGrain()));

    setActionCategory(i18n("Effects"));
    setXMLFile( "digikamimageplugin_fxfilters_ui.rc" );

    kDebug() << "ImagePlugin_FxFilters plugin loaded";
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
