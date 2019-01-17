/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to shear an image.
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

#include "sheartoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "sheartool.h"

namespace Digikam
{

ShearToolPlugin::ShearToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

ShearToolPlugin::~ShearToolPlugin()
{
}

QString ShearToolPlugin::name() const
{
    return i18n("Shear Image");
}

QString ShearToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon ShearToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("transform-shear-left"));
}

QString ShearToolPlugin::description() const
{
    return i18n("A tool to shear an image");
}

QString ShearToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can shear an image.</p>");
}

QList<DPluginAuthor> ShearToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2004-2019"))
            ;
}

void ShearToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "&Shear..."));
    ac->setObjectName(QLatin1String("editorwindow_transform_sheartool"));
    ac->setActionCategory(DPluginAction::EditorTransform);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotShear()));

    addAction(ac);
}

void ShearToolPlugin::slotShear()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        ShearTool* const tool = new ShearTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
