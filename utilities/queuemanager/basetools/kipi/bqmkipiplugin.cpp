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

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>

#include <kipiinterface.h>
#include <kipipluginloader.h>

namespace Digikam
{
  
//BqmKipiPlugin* BqmKipiPlugin::m_instance = 0;

BqmKipiPlugin::BqmKipiPlugin(EmbeddablePlugin* plugin, PluginLoader::Info* info, QObject* const parent)
    : BatchTool(plugin->objectName(), KipiTool, parent)
{
    this->plugin = plugin;
    this->info = info;
    
    setToolTitle(this->info->name());
    setToolDescription(this->info->comment());
    setToolIconName(this->info->icon().name());
    //m_instance = this;
    
    m_settingsWidget = plugin->getWidget();
	    
    connect(plugin, SIGNAL(kipiSettingsChanged(QString ,QMap<QString, QVariant>)),
            this, SLOT(slotKipiSettingsChanged(QString,QMap<QString, QVariant>)));
    
    //connect(KipiInterface::instance(), SIGNAL(kipiSettingsChanged(QString ,QMap<QString, QVariant>)),
    //        this, SLOT(slotKipiSettingsChanged(QString,QMap<QString, QVariant>)));
    //registerSettingsWidget();
}

BqmKipiPlugin::~BqmKipiPlugin()
{
}

BatchTool* BqmKipiPlugin::clone(QObject* const parent=0) const
{
    return new BqmKipiPlugin(this->plugin, this->info, parent);
}

//BqmKipiPlugin* BqmKipiPlugin::instance()
//{
//    return m_instance;  
//}

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
    /*
    if (!loadToDImg())
    {
        return false;
    }

    bool lossless = settings()["lossless"].toBool();
    image().setAttribute("quality", lossless ? 100 : settings()["quality"].toInt());

    return (savefromDImg());
    */
    return true;
}
}  // namespace Digikam
