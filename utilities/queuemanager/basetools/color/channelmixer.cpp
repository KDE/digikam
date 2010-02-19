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
/*
    prm.insert("Brightness", (double)defaultPrm.brightness);
    prm.insert("Contrast",   (double)defaultPrm.contrast);
    prm.insert("Gamma",      (double)defaultPrm.gamma);
*/
    return prm;
}

void ChannelMixer::slotAssignSettings2Widget()
{
    MixerContainer prm;
/*    
    prm.brightness = settings()["Brightness"].toDouble();
    prm.contrast   = settings()["Contrast"].toDouble();
    prm.gamma      = settings()["Gamma"].toDouble();
*/    
    m_settingsView->setSettings(prm);
}

void ChannelMixer::slotSettingsChanged()
{
    BatchToolSettings prm;
    MixerContainer currentPrm = m_settingsView->settings();
/*
    prm.insert("Brightness", (double)currentPrm.brightness);
    prm.insert("Contrast",   (double)currentPrm.contrast);
    prm.insert("Gamma",      (double)currentPrm.gamma);
*/
    setSettings(prm);
}

bool ChannelMixer::toolOperations()
{
    if (!loadToDImg()) return false;

    MixerContainer prm;
/*    
    prm.brightness = settings()["Brightness"].toDouble();
    prm.contrast   = settings()["Contrast"].toDouble();
    prm.gamma      = settings()["Gamma"].toDouble();
*/
    MixerFilter mixer(&image(), 0L, prm);
    mixer.startFilterDirectly();
    image().putImageData(mixer.getTargetImage().bits());

    return (savefromDImg());
}

}  // namespace Digikam
