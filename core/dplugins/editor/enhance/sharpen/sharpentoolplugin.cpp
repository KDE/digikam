/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to sharp image
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

#include "sharpentoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "sharpentool.h"

namespace DigikamEditorSharpenToolPlugin
{

SharpenToolPlugin::SharpenToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

SharpenToolPlugin::~SharpenToolPlugin()
{
}

QString SharpenToolPlugin::name() const
{
    return i18n("Sharpen");
}

QString SharpenToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon SharpenToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("sharpenimage"));
}

QString SharpenToolPlugin::description() const
{
    return i18n("A tool to sharp an image");
}

QString SharpenToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can sharp an image.</p>");
}

QList<DPluginAuthor> SharpenToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2019"))
            ;
}

void SharpenToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Sharpen..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_sharpen"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotSharpen()));

    addAction(ac);
}

void SharpenToolPlugin::slotSharpen()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        SharpenTool* const tool = new SharpenTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorSharpenToolPlugin
