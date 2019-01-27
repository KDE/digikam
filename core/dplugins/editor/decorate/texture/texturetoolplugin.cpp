/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to add texture
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

#include "texturetoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "texturetool.h"

namespace EditorDigikamTextureToolPlugin
{

TextureToolPlugin::TextureToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

TextureToolPlugin::~TextureToolPlugin()
{
}

QString TextureToolPlugin::name() const
{
    return i18n("Texture");
}

QString TextureToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon TextureToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("texture"));
}

QString TextureToolPlugin::description() const
{
    return i18n("A tool to apply a texture over an image");
}

QString TextureToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can apply a texture over an image.</p>");
}

QList<DPluginAuthor> TextureToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Marcel Wiesweg"),
                             QString::fromUtf8("marcel dot wiesweg at gmx dot de"),
                             QString::fromUtf8("(C) 2006-2010"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2005-2019"))
            ;
}

void TextureToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Apply Texture..."));
    ac->setObjectName(QLatin1String("editorwindow_decorate_texture"));
    ac->setActionCategory(DPluginAction::EditorDecorate);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotTexture()));

    addAction(ac);
}

void TextureToolPlugin::slotTexture()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        TextureTool* const tool = new TextureTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace EditorDigikamTextureToolPlugin
