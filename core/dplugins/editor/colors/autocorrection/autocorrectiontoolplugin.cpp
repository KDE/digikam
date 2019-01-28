/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to fix colors automatically
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

#include "autocorrectiontoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "autocorrectiontool.h"

namespace DigikamEditorAutoCorrectionToolPlugin
{

AutoCorrectionToolPlugin::AutoCorrectionToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

AutoCorrectionToolPlugin::~AutoCorrectionToolPlugin()
{
}

QString AutoCorrectionToolPlugin::name() const
{
    return i18n("Auto Correction");
}

QString AutoCorrectionToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon AutoCorrectionToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("autocorrection"));
}

QString AutoCorrectionToolPlugin::description() const
{
    return i18n("A tool to fix colors automatically");
}

QString AutoCorrectionToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can adjust colors automatically from image.</p>");
}

QList<DPluginAuthor> AutoCorrectionToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2019"))
            ;
}

void AutoCorrectionToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Auto-Correction..."));
    ac->setObjectName(QLatin1String("editorwindow_color_autocorrection"));
    // NOTE: Photoshop 7 use CTRL+SHIFT+B
    ac->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_B);
    ac->setActionCategory(DPluginAction::EditorColors);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotAutoCorrection()));

    addAction(ac);
}

void AutoCorrectionToolPlugin::slotAutoCorrection()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        AutoCorrectionTool* const tool = new AutoCorrectionTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorAutoCorrectionToolPlugin
