/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to adjust color levels.
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

#include "adjustlevelstoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "adjustlevelstool.h"

namespace Digikam
{

AdjustLevelsToolPlugin::AdjustLevelsToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

AdjustLevelsToolPlugin::~AdjustLevelsToolPlugin()
{
}

QString AdjustLevelsToolPlugin::name() const
{
    return i18n("Adjust Levels");
}

QString AdjustLevelsToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AdjustLevelsToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("adjustlevels"));
}

QString AdjustLevelsToolPlugin::description() const
{
    return i18n("A tool to adjust color levels");
}

QString AdjustLevelsToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can adjust the color levels from image.</p>");
}

QList<DPluginAuthor> AdjustLevelsToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2004-2019"))
            ;
}

void AdjustLevelsToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Levels Adjust..."));
    ac->setObjectName(QLatin1String("editorwindow_color_adjustlevels"));
    // NOTE: Photoshop 7 use CTRL+M (but it's used in KDE to toogle menu bar).
    ac->setShortcut(Qt::CTRL+Qt::Key_L);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotAdjustCurvesTool()));

    addAction(ac);
}

void AdjustLevelsToolPlugin::slotAdjustCurvesTool()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        AdjustLevelsTool* const tool = new AdjustLevelsTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
