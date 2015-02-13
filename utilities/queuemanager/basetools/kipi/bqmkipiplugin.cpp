/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-8-21
 * Description : Plugged Kipi DNGConverter tool
 *
 * Copyright (C) 2008-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2014 by Shourya Singh Gupta <shouryasgupta@gmail.com>
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

#include "bqmkipiplugin.moc"

// Qt includes

#include <QFileInfo>
#include <QWidget>
#include <QMap>
#include <QDebug>

//local includes

#include <kipiinterface.h>
#include <kipipluginloader.h>

namespace Digikam
{

BqmKipiPlugin::BqmKipiPlugin(EmbeddablePlugin* plugin, PluginLoader::Info* info, QObject* const parent)
    : BatchTool(plugin->objectName(), KipiTool, parent)
{
    this->plugin = plugin;
    this->info = info;
    
    setToolTitle(this->info->name());
    setToolDescription(this->info->comment());
    setToolIconName(this->info->icon().name());
    
    m_settingsWidget = plugin->getWidget();
	    
    connect(plugin, SIGNAL(kipiSettingsChanged(QString ,QMap<QString, QVariant>)),
            this, SLOT(slotKipiSettingsChanged(QString,QMap<QString, QVariant>)));
}

BqmKipiPlugin::~BqmKipiPlugin()
{
}

BatchTool* BqmKipiPlugin::clone(QObject* const parent=0) const
{
    return new BqmKipiPlugin(this->plugin, this->info, parent);
}

void BqmKipiPlugin::registerSettingsWidget()
{
    m_settingsWidget = plugin->getWidget();
    BatchTool::registerSettingsWidget(); 
}

BatchToolSettings BqmKipiPlugin::defaultSettings()
{
    return plugin->defaultSettings();
}

void BqmKipiPlugin::slotAssignSettings2Widget()
{
    plugin->assignSettings(settings());
}

void BqmKipiPlugin::slotKipiSettingsChanged(QString pluginName,QMap<QString, QVariant> settings)
{
    BatchTool::slotSettingsChanged(settings);
}

void BqmKipiPlugin::slotSettingsChanged()
{
}

QString BqmKipiPlugin::outputSuffix() const
{
    return plugin->outputSuffix();
}

bool BqmKipiPlugin::toolOperations()
{
    plugin->startTask(inputUrl());
    while(plugin->status == KIPI::WAITING);                  //This is important. It waits till the thread(of kipi plugin) processing the image has done its work.
    if(plugin->status == KIPI::SUCCESS)
    {
        setOutputUrl(plugin->tempImg());
        setInputUrl(plugin->tempImg());              //This is done so that loadToDImg(which uses input url to load image) loads the image data of the processed image.
        plugin->reset();
        return(loadToDImg());                        //This is done because the next batch tool requries the image data of the processed image of previous batch tool.  
    }
    else
    {
        plugin->reset();
        setErrorDescription(plugin->errorDescription());
        return false; 
    }
}

}  // namespace Digikam
