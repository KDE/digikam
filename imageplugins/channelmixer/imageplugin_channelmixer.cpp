/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-26
 * Description : image channels mixer.
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


#include "imageplugin_channelmixer.moc"

// KDE includes

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcursor.h>
#include <kapplication.h>
#include <kdebug.h>

// Local includes

#include "channelmixertool.h"

using namespace DigikamChannelMixerImagesPlugin;

K_PLUGIN_FACTORY( ChannelMixerFactory, registerPlugin<ImagePlugin_ChannelMixer>(); )
K_EXPORT_PLUGIN ( ChannelMixerFactory("digikamimageplugin_channelmixer") )

ImagePlugin_ChannelMixer::ImagePlugin_ChannelMixer(QObject *parent, const QVariantList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_ChannelMixer")
{
    m_channelMixerAction  = new KAction(KIcon("channelmixer"), i18n("Channel Mixer..."), this);
    m_channelMixerAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_H));
    actionCollection()->addAction("imageplugin_channelmixer", m_channelMixerAction );

    connect(m_channelMixerAction, SIGNAL(triggered(bool)),
            this, SLOT(slotChannelMixer()));

    setXMLFile("digikamimageplugin_channelmixer_ui.rc");

    kDebug() << "ImagePlugin_ChannelMixer plugin loaded";
}

ImagePlugin_ChannelMixer::~ImagePlugin_ChannelMixer()
{
}

void ImagePlugin_ChannelMixer::setEnabledActions(bool enable)
{
    m_channelMixerAction->setEnabled(enable);
}

void ImagePlugin_ChannelMixer::slotChannelMixer()
{
    ChannelMixerTool *tool = new ChannelMixerTool(this);
    loadTool(tool);
}
