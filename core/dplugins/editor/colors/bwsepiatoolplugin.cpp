/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to convert to Black and White
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

#include "bwsepiatoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "bwsepiatool.h"

namespace Digikam
{

BWSepiaToolPlugin::BWSepiaToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

BWSepiaToolPlugin::~BWSepiaToolPlugin()
{
}

QString BWSepiaToolPlugin::name() const
{
    return i18n("Black and White");
}

QString BWSepiaToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BWSepiaToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("bwtonal"));
}

QString BWSepiaToolPlugin::description() const
{
    return i18n("A tool to convert to black and white");
}

QString BWSepiaToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can can convert image to black and white.</p>");
}

QList<DPluginAuthor> BWSepiaToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Renchi Raju"),
                             QLatin1String("renchi dot raju at gmail dot com"),
                             QLatin1String("(C) 2004-2005"))
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2006-2019"))
            ;
}

void BWSepiaToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Black && White..."));
    ac->setObjectName(QLatin1String("editorwindow_color_blackwhite"));
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotBWSepia()));

    addAction(ac);
}

void BWSepiaToolPlugin::slotBWSepia()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        BWSepiaTool* const tool = new BWSepiaTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
