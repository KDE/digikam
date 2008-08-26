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

// KDE includes.

#include <klocale.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <kaction.h>
#include <kcursor.h>

// Local includes.

#include "ddebug.h"
#include "channelmixertool.h"
#include "imageplugin_channelmixer.h"
#include "imageplugin_channelmixer.moc"

using namespace DigikamChannelMixerImagesPlugin;

K_EXPORT_COMPONENT_FACTORY(digikamimageplugin_channelmixer,
                           KGenericFactory<ImagePlugin_ChannelMixer>("digikamimageplugin_channelmixer"))

ImagePlugin_ChannelMixer::ImagePlugin_ChannelMixer(QObject *parent, const char*,
                                                   const QStringList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_ChannelMixer")
{
    m_channelMixerAction = new KAction(i18n("Channel Mixer..."), "channelmixer", 
                               CTRL+Key_H, 
                               this, SLOT(slotChannelMixer()),
                               actionCollection(), "imageplugin_channelmixer");

    setXMLFile("digikamimageplugin_channelmixer_ui.rc");

    DDebug() << "ImagePlugin_ChannelMixer plugin loaded" << endl;
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
    ChannelMixerTool *cm = new ChannelMixerTool(this);
    loadTool(cm);
}
