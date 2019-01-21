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

#include "redeyetoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "redeyetool.h"

namespace Digikam
{

RedEyeToolPlugin::RedEyeToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

RedEyeToolPlugin::~RedEyeToolPlugin()
{
}

QString RedEyeToolPlugin::name() const
{
    return i18n("Red Eye");
}

QString RedEyeToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RedEyeToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("redeyes"));
}

QString RedEyeToolPlugin::description() const
{
    return i18n("A tool to automatically detect and correct red eye effect");
}

QString RedEyeToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can reduce red eye effect on image.</p>");
}

QList<DPluginAuthor> RedEyeToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Renchi Raju"),
                             QString::fromUtf8("renchi dot raju at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2005"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2019"))
            ;
}

void RedEyeToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Red Eye..."));
    ac->setWhatsThis(i18n("This filter can be used to correct red eyes in a photo. "
                          "Select a region including the eyes to use this option."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_redeye"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotRedEye()));

    addAction(ac);
}

void RedEyeToolPlugin::slotRedEye()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        RedEyeTool* const tool = new RedEyeTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
