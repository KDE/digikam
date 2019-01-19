/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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

#include "restorationtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "restorationtool.h"

namespace Digikam
{

RestoreToolPlugin::RestoreToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

RestoreToolPlugin::~RestoreToolPlugin()
{
}

QString RestoreToolPlugin::name() const
{
    return i18n("Restoration");
}

QString RestoreToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RestoreToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("restoration"));
}

QString RestoreToolPlugin::description() const
{
    return i18n("A tool to restore an image using Greystoration algorithm");
}

QString RestoreToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can restore an image using Greystoration algorithm.</p>");
}

QList<DPluginAuthor> RestoreToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2019"))
            ;
}

void RestoreToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Restoration..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_restoration"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotRestore()));

    addAction(ac);
}
    
void RestoreToolPlugin::slotRestore()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        RestorationTool* const tool = new RestorationTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
