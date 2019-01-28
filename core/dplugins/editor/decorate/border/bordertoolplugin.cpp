/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to add border
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

#include "bordertoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "bordertool.h"

namespace DigikamEditorBorderToolPlugin
{

BorderToolPlugin::BorderToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

BorderToolPlugin::~BorderToolPlugin()
{
}

QString BorderToolPlugin::name() const
{
    return i18n("Add Border");
}

QString BorderToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BorderToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("bordertool"));
}

QString BorderToolPlugin::description() const
{
    return i18n("A tool to add a border around image");
}

QString BorderToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can add decorative border around image.</p>");
}

QList<DPluginAuthor> BorderToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Marcel Wiesweg"),
                             QString::fromUtf8("marcel dot wiesweg at gmx dot de"),
                             QString::fromUtf8("(C) 2006-2012"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2019"))
            ;
}

void BorderToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Add Border..."));
    ac->setObjectName(QLatin1String("editorwindow_decorate_border"));
    ac->setActionCategory(DPluginAction::EditorDecorate);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotBorder()));

    addAction(ac);
}

void BorderToolPlugin::slotBorder()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        BorderTool* const tool = new BorderTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorBorderToolPlugin
