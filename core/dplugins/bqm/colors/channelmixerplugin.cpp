/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a BQM plugin to mix color channels
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "channelmixerplugin.h"

// Qt includes

#include <QPointer>
#include <QString>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "channelmixer.h"

namespace Digikam
{

ChannelMixerPlugin::ChannelMixerPlugin(QObject* const parent)
    : DPluginBqm(parent)
{
}

ChannelMixerPlugin::~ChannelMixerPlugin()
{
}

QString ChannelMixerPlugin::name() const
{
    return i18n("Channel Mixer");
}

QString ChannelMixerPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ChannelMixerPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("colorfx"));
}

QString ChannelMixerPlugin::description() const
{
    return i18n("A tool to mix color channel");
}

QString ChannelMixerPlugin::details() const
{
    return i18n("<p>This batch Queue Manager tool can mix color channels from images.</p>");
}

QList<DPluginAuthor> ChannelMixerPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2010-2019"))
            ;
}

void ChannelMixerPlugin::setup(QObject* const parent)
{
    ChannelMixer* const tool = new ChannelMixer(parent);
    tool->setPlugin(this);

    addTool(tool);
}

} // namespace Digikam
