/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to restore an image
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

#include "localcontrasttoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "localcontrasttool.h"

namespace Digikam
{

LocalContrastToolPlugin::LocalContrastToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

LocalContrastToolPlugin::~LocalContrastToolPlugin()
{
}

QString LocalContrastToolPlugin::name() const
{
    return i18n("Local Contrast");
}

QString LocalContrastToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon LocalContrastToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("contrast"));
}

QString LocalContrastToolPlugin::description() const
{
    return i18n("A tool to emulate tone mapping");
}

QString LocalContrastToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can emulate tone mapping over an image.</p>");
}

QList<DPluginAuthor> LocalContrastToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Julien Pontabry"),
                             QLatin1String("julien dot pontabry at gmail dot com"),
                             QLatin1String("(C) 2009"))
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2009-2019"))
            ;
}
    
void LocalContrastToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Local Contrast..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_localcontrast"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotLocalContrast()));

    addAction(ac);
}
    
void LocalContrastToolPlugin::slotLocalContrast()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        LocalContrastTool* const tool = new LocalContrastTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
