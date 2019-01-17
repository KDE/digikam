/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to add rain drops over an image.
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

#include "raindroptoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "raindroptool.h"

namespace Digikam
{

RainDropToolPlugin::RainDropToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

RainDropToolPlugin::~RainDropToolPlugin()
{
}

QString RainDropToolPlugin::name() const
{
    return i18n("Raindrops");
}

QString RainDropToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RainDropToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("raindrop"));
}

QString RainDropToolPlugin::description() const
{
    return i18n("A tool to add rain drops over an image");
}

QString RainDropToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can add rain drops over an image.</p>");
}

QList<DPluginAuthor> RainDropToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Marcel Wiesweg"),
                             QLatin1String("marcel dot wiesweg at gmx dot de"),
                             QLatin1String("(C) 2006-2012"))
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2006-2019"))
            ;
}
    
void RainDropToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Raindrops..."));
    ac->setObjectName(QLatin1String("editorwindow_filter_raindrop"));
    ac->setActionCategory(DPluginAction::EditorFilters);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotRainDrop()));

    addAction(ac);
}

void RainDropToolPlugin::slotRainDrop()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        RainDropTool* const tool = new RainDropTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
