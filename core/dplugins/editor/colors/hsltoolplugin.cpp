/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to adjust HSL
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

#include "hsltoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "hsltool.h"

namespace Digikam
{

HSLToolPlugin::HSLToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

HSLToolPlugin::~HSLToolPlugin()
{
}

QString HSLToolPlugin::name() const
{
    return i18n("HSL Correction");
}

QString HSLToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon HSLToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("adjusthsl"));
}

QString HSLToolPlugin::description() const
{
    return i18n("A tool to fix Hue / Saturation / Lightness");
}

QString HSLToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can adjust Hue / Saturation / Lightness from image.</p>");
}

QList<DPluginAuthor> HSLToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2004-2019"))
            ;
}
    
void HSLToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Hue/Saturation/Lightness..."));
    ac->setObjectName(QLatin1String("editorwindow_color_hsl"));
    // NOTE: Photoshop 7 use CTRL+U.
    ac->setShortcut(Qt::CTRL+Qt::Key_U);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotHSL()));

    addAction(ac);
}

void HSLToolPlugin::slotHSL()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        HSLTool* const tool = new HSLTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
