/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : a digiKam image plugin to apply special effects.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

using namespace DigikamFxFiltersImagePlugin;

K_PLUGIN_FACTORY( FxFiltersFactory, registerPlugin<ImagePlugin_FxFilters>(); )
K_EXPORT_PLUGIN ( FxFiltersFactory("digikamimageplugin_fxfilters") )

ImagePlugin_FxFilters::ImagePlugin_FxFilters(QObject* parent, const QVariantList&)
                     : ImagePlugin(parent, "ImagePlugin_FxFilters")
{
    m_colorEffectsAction = new KAction(KIcon("colorfx"), i18n("Color Effects..."), this);
    actionCollection()->addAction("imageplugin_colorfx", m_colorEffectsAction );
    connect(m_colorEffectsAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotColorEffects()));

    m_charcoalAction = new KAction(KIcon("charcoaltool"), i18n("Charcoal Drawing..."), this);
    actionCollection()->addAction("imageplugin_charcoal", m_charcoalAction  );
    connect(m_charcoalAction, SIGNAL(triggered(bool)),
            this, SLOT(slotCharcoal()));

    m_embossAction = new KAction(KIcon("embosstool"), i18n("Emboss..."), this);
    actionCollection()->addAction("imageplugin_emboss", m_embossAction );
    connect(m_embossAction, SIGNAL(triggered(bool)),
            this, SLOT(slotEmboss()));

    m_oilpaintAction = new KAction(KIcon("oilpaint"), i18n("Oil Paint..."), this);
    actionCollection()->addAction("imageplugin_oilpaint", m_oilpaintAction);
    connect(m_oilpaintAction, SIGNAL(triggered(bool) ),
            this ,SLOT(slotOilPaint()));

    m_blurfxAction = new KAction(KIcon("blurfx"), i18n("Blur Effects..."), this);
    actionCollection()->addAction("imageplugin_blurfx", m_blurfxAction );
    connect(m_blurfxAction, SIGNAL(triggered(bool)),
            this, SLOT(slotBlurFX()));

    m_distortionfxAction = new KAction(KIcon("distortionfx"), i18n("Distortion Effects..."), this);
    actionCollection()->addAction("imageplugin_distortionfx", m_distortionfxAction );
    connect(m_distortionfxAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotDistortionFX()));

    m_raindropAction = new KAction(KIcon("raindrop"), i18n("Raindrops..."), this);
    actionCollection()->addAction("imageplugin_raindrop", m_raindropAction );
    connect(m_raindropAction, SIGNAL(triggered(bool) ),
            this, SLOT(slotRainDrop()));

    setXMLFile( "digikamimageplugin_fxfilters_ui.rc" );

    kDebug() << "ImagePlugin_FxFilters plugin loaded";
}

ImagePlugin_FxFilters::~ImagePlugin_FxFilters()
{
}

void ImagePlugin_FxFilters::setEnabledActions(bool b)
{
    m_charcoalAction->setEnabled(b);
    m_colorEffectsAction->setEnabled(b);
    m_embossAction->setEnabled(b);
    m_oilpaintAction->setEnabled(b);
    m_blurfxAction->setEnabled(b);
    m_distortionfxAction->setEnabled(b);
    m_raindropAction->setEnabled(b);
}

void ImagePlugin_FxFilters::slotColorEffects()
{
    ColorFxTool* tool = new ColorFxTool(this);
    loadTool(tool);
}

void ImagePlugin_FxFilters::slotCharcoal()
{
    CharcoalTool* tool = new CharcoalTool(this);
    loadTool(tool);
}

void ImagePlugin_FxFilters::slotEmboss()
{
    EmbossTool* tool = new EmbossTool(this);
    loadTool(tool);
}

void ImagePlugin_FxFilters::slotOilPaint()
{
    OilPaintTool* tool = new OilPaintTool(this);
    loadTool(tool);
}

void ImagePlugin_FxFilters::slotBlurFX()
{
    BlurFXTool* tool = new BlurFXTool(this);
    loadTool(tool);
}

void ImagePlugin_FxFilters::slotDistortionFX()
{
    DistortionFXTool* tool = new DistortionFXTool(this);
    loadTool(tool);
}

void ImagePlugin_FxFilters::slotRainDrop()
{
    RainDropTool* tool = new RainDropTool(this);
    loadTool(tool);
}
