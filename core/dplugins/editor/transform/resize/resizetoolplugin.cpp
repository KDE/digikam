/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to resize an image.
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

#include "resizetoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "resizetool.h"

namespace EditorDigikamResizeToolPlugin
{

ResizeToolPlugin::ResizeToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

ResizeToolPlugin::~ResizeToolPlugin()
{
}

QString ResizeToolPlugin::name() const
{
    return i18n("Resize Image");
}

QString ResizeToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ResizeToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("transform-scale"));
}

QString ResizeToolPlugin::description() const
{
    return i18n("A tool to resize an image");
}

QString ResizeToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can resize an image.</p>");
}

QList<DPluginAuthor> ResizeToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2019"))
            ;
}

void ResizeToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "&Resize..."));
    ac->setObjectName(QLatin1String("editorwindow_transform_resize"));
    ac->setActionCategory(DPluginAction::EditorTransform);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotResize()));

    addAction(ac);
}

void ResizeToolPlugin::slotResize()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        ResizeTool* const tool = new ResizeTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace EditorDigikamResizeToolPlugin
