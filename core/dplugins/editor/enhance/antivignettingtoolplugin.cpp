/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to reduce vignetting
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

#include "antivignettingtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "antivignettingtool.h"

namespace Digikam
{

AntiVignettingToolPlugin::AntiVignettingToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

AntiVignettingToolPlugin::~AntiVignettingToolPlugin()
{
}

QString AntiVignettingToolPlugin::name() const
{
    return i18n("Vignetting Correction");
}

QString AntiVignettingToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AntiVignettingToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("antivignetting"));
}

QString AntiVignettingToolPlugin::description() const
{
    return i18n("A tool to correct vignetting");
}

QString AntiVignettingToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool correct vignetting from an image.</p>");
}

QList<DPluginAuthor> AntiVignettingToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Julien Narboux"),
                             QLatin1String("julien at narboux dot fr"),
                             QLatin1String("(C) 2010"))
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2004-2019"))
            ;
}
    
void AntiVignettingToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Vignetting Correction..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_antivignetting"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotAntiVignetting()));

    addAction(ac);
}
    
void AntiVignettingToolPlugin::slotAntiVignetting()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        AntiVignettingTool* const tool = new AntiVignettingTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
