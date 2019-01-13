/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to fix BCG.
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

#include "bcgtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "bcgtool.h"

namespace Digikam
{

BCGToolPlugin::BCGToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

BCGToolPlugin::~BCGToolPlugin()
{
}

QString BCGToolPlugin::name() const
{
    return i18n("BCG Correction");
}

QString BCGToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BCGToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("contrast"));
}

QString BCGToolPlugin::description() const
{
    return i18n("A tool to fix Brightness / Contrast / Gamma");
}

QString BCGToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can adjust Brightness / Contrast / Gamma from image.</p>");
}

QList<DPluginAuthor> BCGToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Renchi Raju"),
                             QLatin1String("renchi dot raju at gmail dot com"),
                             QLatin1String("(C) 2004"))
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2005-2019"))
            ;
}

void BCGToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Brightness/Contrast/Gamma..."));
    ac->setObjectName(QLatin1String("editorwindow_color_bcg"));
    ac->setActionCategory(DPluginAction::EditorColor);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotBCGTool()));

    addAction(ac);
}

void BCGToolPlugin::slotBCGTool()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        BCGTool* const tool = new BCGTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
