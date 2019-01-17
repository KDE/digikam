/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to adjust perspective.
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

#include "perspectivetoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "perspectivetool.h"

namespace Digikam
{

PerspectiveToolPlugin::PerspectiveToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

PerspectiveToolPlugin::~PerspectiveToolPlugin()
{
}

QString PerspectiveToolPlugin::name() const
{
    return i18n("Perspective");
}

QString PerspectiveToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon PerspectiveToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("perspective"));
}

QString PerspectiveToolPlugin::description() const
{
    return i18n("A tool to adjust perspective");
}

QString PerspectiveToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can adjust perspective from an image.</p>");
}

QList<DPluginAuthor> PerspectiveToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Marcel Wiesweg"),
                             QLatin1String("marcel dot wiesweg at gmx dot de"),
                             QLatin1String("(C) 2006-2012"))
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2005-2019"))
            ;
}

void PerspectiveToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Perspective Adjustment..."));
    ac->setObjectName(QLatin1String("editorwindow_transform_perspective"));
    ac->setActionCategory(DPluginAction::EditorTransform);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotPerspective()));

    addAction(ac);
}

void PerspectiveToolPlugin::slotPerspective()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        PerspectiveTool* const tool = new PerspectiveTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
