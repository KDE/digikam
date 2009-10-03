/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-10
 * Description : a plugin to apply texture over an image
 *
 * Copyright (C) 2005-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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


#include "imageplugin_texture.h"
#include "imageplugin_texture.moc"

// KDE includes

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>
#include <kapplication.h>

// Local includes

#include "texturetool.h"
#include "debug.h"

using namespace DigikamTextureImagesPlugin;

K_PLUGIN_FACTORY( TextureFactory, registerPlugin<ImagePlugin_Texture>(); )
K_EXPORT_PLUGIN ( TextureFactory("digikamimageplugin_texture") )

ImagePlugin_Texture::ImagePlugin_Texture(QObject *parent, const QVariantList &)
                   : Digikam::ImagePlugin(parent, "ImagePlugin_Texture")
{

    m_textureAction  = new KAction(KIcon("texture"), i18n("Apply Texture..."), this);
    actionCollection()->addAction("imageplugin_texture", m_textureAction );

    connect(m_textureAction, SIGNAL(triggered(bool)),
            this, SLOT(slotTexture()));

    setXMLFile( "digikamimageplugin_texture_ui.rc" );

    kDebug(imagePluginsAreaCode) << "ImagePlugin_Texture plugin loaded";
}

ImagePlugin_Texture::~ImagePlugin_Texture()
{
}

void ImagePlugin_Texture::setEnabledActions(bool enable)
{
    m_textureAction->setEnabled(enable);
}

void ImagePlugin_Texture::slotTexture()
{
    TextureTool *tool = new TextureTool(this);
    loadTool(tool);
}
