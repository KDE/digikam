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

#include "dng.moc"

// Qt includes

#include <QFileInfo>
#include <QWidget>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kglobal.h>

#include <kipipluginloader.h>

namespace Digikam
{

DNG::DNG(QObject* const parent)
    : BatchTool("DNG", KipiTool, parent)
{
    setToolTitle(i18n("Convert To DNG"));
    setToolDescription(i18n("Convert RAW images to DNG format."));
    setToolIconName("image-jp2");
}

DNG::~DNG()
{
}

void DNG::registerSettingsWidget()
{
    
    KipiPluginLoader* loader = KipiPluginLoader::instance();
    
    PluginLoader::PluginList list = loader->listPlugins();
    
    for (PluginLoader::PluginList::ConstIterator it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        Plugin* const plugin = (*it)->plugin();
	if (plugin->objectName() == QString("DNGConverter"))
	{
	    m_settingsWidget = plugin->settingsWidget();
	}
    }
    /*
    m_settingsWidget = m_settings;

    connect(m_settings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
    */
}

BatchToolSettings DNG::defaultSettings()
{
    /* 
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");
    int compression           = group.readEntry("JPEG2000Compression", 75);
    bool lossLessCompression  = group.readEntry("JPEG2000LossLess",    true);
    BatchToolSettings settings;
    settings.insert("quality",  compression);
    settings.insert("lossless", lossLessCompression);
    return settings;
    */
    BatchToolSettings def;
    return def;
    
}

void DNG::slotAssignSettings2Widget()
{
    /*  
    m_settings->setCompressionValue(settings()["quality"].toInt());
    m_settings->setLossLessCompression(settings()["lossless"].toBool());
   */
  
}

void DNG::slotSettingsChanged()
{
    /*
    BatchToolSettings settings;
    settings.insert("quality",  m_settings->getCompressionValue());
    settings.insert("lossless", m_settings->getLossLessCompression());
    BatchTool::slotSettingsChanged(settings);
    */
}

QString DNG::outputSuffix() const
{
    return QString("dng");
}

bool DNG::toolOperations()
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
