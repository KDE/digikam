/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to apply color effects.
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

#include "colorfxtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "colorfxtool.h"

namespace Digikam
{

ColorFXToolPlugin::ColorFXToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

ColorFXToolPlugin::~ColorFXToolPlugin()
{
}

QString ColorFXToolPlugin::name() const
{
    return i18n("Color Effects");
}

QString ColorFXToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ColorFXToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("colorfx"));
}

QString ColorFXToolPlugin::description() const
{
    return i18n("A tool to apply color effects to an image");
}

QString ColorFXToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can apply color effects to an image.</p>");
}

QList<DPluginAuthor> ColorFXToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Renchi Raju"),
                             QLatin1String("renchi dot raju at gmail dot com"),
                             QLatin1String("(C) 2004-2005"))
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2006-2019"))
            ;
}
    
void ColorFXToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Color Effects..."));
    ac->setObjectName(QLatin1String("editorwindow_filter_colorfx"));
    ac->setActionCategory(DPluginAction::EditorFilters);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotColorFX()));

    addAction(ac);
}

void ColorFXToolPlugin::slotColorFX()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        ColorFxTool* const tool = new ColorFxTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
