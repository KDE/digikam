/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to fix colors balance
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

#include "cbtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "cbtool.h"

namespace Digikam
{

CBToolPlugin::CBToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

CBToolPlugin::~CBToolPlugin()
{
}

QString CBToolPlugin::name() const
{
    return i18n("Color Balance");
}

QString CBToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon CBToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("adjustrgb"));
}

QString CBToolPlugin::description() const
{
    return i18n("A tool to adjust color balance");
}

QString CBToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can adjust color balance from image.</p>");
}

QList<DPluginAuthor> CBToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2004-2019"))
            ;
}

void CBToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Color Balance..."));
    ac->setObjectName(QLatin1String("editorwindow_color_rgb"));
    // NOTE: Photoshop 7 use CTRL+B.
    ac->setShortcut(Qt::CTRL+Qt::Key_B);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotColorBalance()));

    addAction(ac);
}

void CBToolPlugin::slotColorBalance()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        CBTool* const tool = new CBTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
