/* ============================================================
 * File  : imageplugin_channelmixer.cpp
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
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

// Local includes.

#include "bannerwidget.h"
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
    QString title = i18n("Color Channel Mixer");
    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(0, title);
    DigikamChannelMixerImagesPlugin::ChannelMixerDialog dlg(parentWidget(),
                                title, headerFrame);
    dlg.exec();
    delete headerFrame; 
}

#include "imageplugin_channelmixer.moc"
