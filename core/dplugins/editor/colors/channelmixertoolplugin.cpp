/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to mix color channels
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

#include "channelmixertoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "channelmixertool.h"

namespace Digikam
{

ChannelMixerToolPlugin::ChannelMixerToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

ChannelMixerToolPlugin::~ChannelMixerToolPlugin()
{
}

QString ChannelMixerToolPlugin::name() const
{
    return i18n("Channel Mixer");
}

QString ChannelMixerToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ChannelMixerToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("channelmixer"));
}

QString ChannelMixerToolPlugin::description() const
{
    return i18n("A tool to mix color channel");
}

QString ChannelMixerToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can mix color channels from image.</p>");
}

QList<DPluginAuthor> ChannelMixerToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2005-2019"))
            ;
}

void ChannelMixerToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Channel Mixer..."));
    ac->setObjectName(QLatin1String("editorwindow_color_channelmixer"));
    ac->setShortcut(Qt::CTRL+Qt::Key_H);
    ac->setActionCategory(DPluginAction::EditorColor);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotChannelMixer()));

    addAction(ac);
}

void ChannelMixerToolPlugin::slotChannelMixer()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        ChannelMixerTool* const tool = new ChannelMixerTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
