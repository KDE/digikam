/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to insert text
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

#include "inserttexttoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "inserttexttool.h"

namespace DigikamEditorInsertTextToolPlugin
{

InsertTextToolPlugin::InsertTextToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

InsertTextToolPlugin::~InsertTextToolPlugin()
{
}

QString InsertTextToolPlugin::name() const
{
    return i18n("Insert Text");
}

QString InsertTextToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon InsertTextToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("insert-text"));
}

QString InsertTextToolPlugin::description() const
{
    return i18n("A tool to insert text over an image");
}

QString InsertTextToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can insert text over an image.</p>");
}

QList<DPluginAuthor> InsertTextToolPlugin::authors() const
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

void InsertTextToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Insert Text..."));
    ac->setObjectName(QLatin1String("editorwindow_decorate_inserttext"));
    ac->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_T);
    ac->setActionCategory(DPluginAction::EditorDecorate);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotInsertText()));

    addAction(ac);
}

void InsertTextToolPlugin::slotInsertText()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        InsertTextTool* const tool = new InsertTextTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorInsertTextToolPlugin
