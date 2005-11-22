/* ============================================================
 * File  : imageplugin_channelmixer.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-02-26
 * Description : 
 * 
 * Copyright 2005 by Gilles Caulier
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
#include <kdebug.h>


// Local includes.

#include "channelmixer.h"
#include "imageplugin_channelmixer.h"

K_EXPORT_COMPONENT_FACTORY( digikamimageplugin_channelmixer,
                            KGenericFactory<ImagePlugin_ChannelMixer>("digikamimageplugin_channelmixer"))

ImagePlugin_ChannelMixer::ImagePlugin_ChannelMixer(QObject *parent, const char*,
                                                   const QStringList &)
                        : Digikam::ImagePlugin(parent, "ImagePlugin_ChannelMixer")
{
    m_channelMixerAction = new KAction(i18n("Channel Mixer..."), "channelmixer", 0, 
                               this, SLOT(slotChannelMixer()),
                               actionCollection(), "imageplugin_channelmixer");

    setXMLFile("digikamimageplugin_channelmixer_ui.rc");
    
    kdDebug() << "ImagePlugin_ChannelMixer plugin loaded" << endl;
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
    Digikam::ImageIface iface(0, 0);

    uint* data = iface.getOriginalData();
    int w      = iface.originalWidth();
    int h      = iface.originalHeight();
    
    DigikamChannelMixerImagesPlugin::ChannelMixerDialog dlg(parentWidget(), data, w, h);
    dlg.exec();
    delete [] data;
}


#include "imageplugin_channelmixer.moc"
