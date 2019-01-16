/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to fix white balance
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

#include "whitebalancetoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "whitebalancetool.h"

namespace Digikam
{

WhiteBalanceToolPlugin::WhiteBalanceToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

WhiteBalanceToolPlugin::~WhiteBalanceToolPlugin()
{
}

QString WhiteBalanceToolPlugin::name() const
{
    return i18n("White Balance");
}

QString WhiteBalanceToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon WhiteBalanceToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("whitebalance"));
}

QString WhiteBalanceToolPlugin::description() const
{
    return i18n("A tool to adjust white balance");
}

QString WhiteBalanceToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can can adjust the white balance from image.</p>");
}

QList<DPluginAuthor> WhiteBalanceToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2004-2019"))
            << DPluginAuthor(QLatin1String("Guillaume Castagnino"),
                             QLatin1String("casta at xwing dot info"),
                             QLatin1String("(C) 2008-2009"))
            ;
}
    
void WhiteBalanceToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "White Balance..."));
    ac->setObjectName(QLatin1String("editorwindow_color_whitebalance"));
    ac->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_W);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotWhiteBalance()));

    addAction(ac);
}

void WhiteBalanceToolPlugin::slotWhiteBalance()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        WhiteBalanceTool* const tool = new WhiteBalanceTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
