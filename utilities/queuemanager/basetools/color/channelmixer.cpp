/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-19
 * Description : Channel Mixer batch tool.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "channelmixer.moc"

// Qt includes

#include <QWidget>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "dimg.h"
#include "mixerfilter.h"
#include "mixersettings.h"

namespace Digikam
{

ChannelMixer::ChannelMixer(QObject* parent)
            : BatchTool("ChannelMixer", BaseTool, parent)
{
    setToolTitle(i18n("Channel Mixer"));
    setToolDescription(i18n("A tool to mix Color Channel."));
    setToolIcon(KIcon(SmallIcon("channelmixer")));

    QWidget *box   = new QWidget;
    m_settingsView = new MixerSettings(box);
    setSettingsWidget(box);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

ChannelMixer::~ChannelMixer()
{
}

BatchToolSettings ChannelMixer::defaultSettings()
{
    BatchToolSettings prm;
    MixerContainer defaultPrm = m_settingsView->defaultSettings();
    
    prm.insert("bPreserveLum",   (bool)defaultPrm.bPreserveLum);
    prm.insert("bMonochrome",    (bool)defaultPrm.bMonochrome);

    // Standard settings.
    prm.insert("redRedGain",     (double)defaultPrm.redRedGain);
    prm.insert("redGreenGain",   (double)defaultPrm.redGreenGain);
    prm.insert("redBlueGain",    (double)defaultPrm.redBlueGain);
    prm.insert("greenRedGain",   (double)defaultPrm.greenRedGain);
    prm.insert("greenGreenGain", (double)defaultPrm.greenGreenGain);
    prm.insert("greenBlueGain",  (double)defaultPrm.greenBlueGain);
    prm.insert("blueRedGain",    (double)defaultPrm.blueRedGain);
    prm.insert("blueGreenGain",  (double)defaultPrm.blueGreenGain);
    prm.insert("blueBlueGain",   (double)defaultPrm.blueBlueGain);

    // Monochrome settings.
    prm.insert("blackRedGain",   (double)defaultPrm.blackRedGain);
    prm.insert("blackGreenGain", (double)defaultPrm.blackGreenGain);
    prm.insert("blackBlueGain",  (double)defaultPrm.blackBlueGain);

    return prm;
}

void ChannelMixer::slotAssignSettings2Widget()
{
    MixerContainer prm;

    prm.bPreserveLum   = settings()["bPreserveLum"].toBool();
    prm.bMonochrome    = settings()["bMonochrome"].toBool();

    // Standard settings.
    prm.redRedGain     = settings()["redRedGain"].toDouble();
    prm.redGreenGain   = settings()["redGreenGain"].toDouble();
    prm.redBlueGain    = settings()["redBlueGain"].toDouble();
    prm.greenRedGain   = settings()["greenRedGain"].toDouble();
    prm.greenGreenGain = settings()["greenGreenGain"].toDouble();
    prm.greenBlueGain  = settings()["greenBlueGain"].toDouble();
    prm.blueRedGain    = settings()["blueRedGain"].toDouble();
    prm.blueGreenGain  = settings()["blueGreenGain"].toDouble();
    prm.blueBlueGain   = settings()["blueBlueGain"].toDouble();

    // Monochrome settings.
    prm.blackRedGain   = settings()["blackRedGain"].toDouble();
    prm.blackGreenGain = settings()["blackGreenGain"].toDouble();
    prm.blackBlueGain  = settings()["blackBlueGain"].toDouble();   
    
    m_settingsView->setSettings(prm);
}

void ChannelMixer::slotSettingsChanged()
{
    BatchToolSettings prm;
    MixerContainer currentPrm = m_settingsView->settings();
    
    prm.insert("bPreserveLum",   (bool)currentPrm.bPreserveLum);
    prm.insert("bMonochrome",    (bool)currentPrm.bMonochrome);

    // Standard settings.
    prm.insert("redRedGain",     (double)currentPrm.redRedGain);
    prm.insert("redGreenGain",   (double)currentPrm.redGreenGain);
    prm.insert("redBlueGain",    (double)currentPrm.redBlueGain);
    prm.insert("greenRedGain",   (double)currentPrm.greenRedGain);
    prm.insert("greenGreenGain", (double)currentPrm.greenGreenGain);
    prm.insert("greenBlueGain",  (double)currentPrm.greenBlueGain);
    prm.insert("blueRedGain",    (double)currentPrm.blueRedGain);
    prm.insert("blueGreenGain",  (double)currentPrm.blueGreenGain);
    prm.insert("blueBlueGain",   (double)currentPrm.blueBlueGain);

    // Monochrome settings.
    prm.insert("blackRedGain",   (double)currentPrm.blackRedGain);
    prm.insert("blackGreenGain", (double)currentPrm.blackGreenGain);
    prm.insert("blackBlueGain",  (double)currentPrm.blackBlueGain);    

    setSettings(prm);
}

bool ChannelMixer::toolOperations()
{
    if (!loadToDImg()) return false;

    MixerContainer prm;

    prm.bPreserveLum   = settings()["bPreserveLum"].toBool();
    prm.bMonochrome    = settings()["bMonochrome"].toBool();

    // Standard settings.
    prm.redRedGain     = settings()["redRedGain"].toDouble();
    prm.redGreenGain   = settings()["redGreenGain"].toDouble();
    prm.redBlueGain    = settings()["redBlueGain"].toDouble();
    prm.greenRedGain   = settings()["greenRedGain"].toDouble();
    prm.greenGreenGain = settings()["greenGreenGain"].toDouble();
    prm.greenBlueGain  = settings()["greenBlueGain"].toDouble();
    prm.blueRedGain    = settings()["blueRedGain"].toDouble();
    prm.blueGreenGain  = settings()["blueGreenGain"].toDouble();
    prm.blueBlueGain   = settings()["blueBlueGain"].toDouble();

    // Monochrome settings.
    prm.blackRedGain   = settings()["blackRedGain"].toDouble();
    prm.blackGreenGain = settings()["blackGreenGain"].toDouble();
    prm.blackBlueGain  = settings()["blackBlueGain"].toDouble();  
    
    MixerFilter mixer(&image(), 0L, prm);
    mixer.startFilterDirectly();
    image().putImageData(mixer.getTargetImage().bits());

    return (savefromDImg());
}

}  // namespace Digikam
