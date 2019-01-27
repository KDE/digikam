/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to blur an image
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

#include "blurtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "blurtool.h"

namespace EditorDigikamBlurToolPlugin
{

BlurToolPlugin::BlurToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

BlurToolPlugin::~BlurToolPlugin()
{
}

QString BlurToolPlugin::name() const
{
    return i18n("Blur");
}

QString BlurToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BlurToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("blurimage"));
}

QString BlurToolPlugin::description() const
{
    return i18n("A tool to blur an image");
}

QString BlurToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can blur an image.</p>");
}

QList<DPluginAuthor> BlurToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Andi Clemens"),
                             QString::fromUtf8("andi dot clemens at gmail dot com"),
                             QString::fromUtf8("(C) 2009"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2019"))
            ;
}

void BlurToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Blur..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_blur"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotBlur()));

    addAction(ac);
}

void BlurToolPlugin::slotBlur()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        BlurTool* const tool = new BlurTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace EditorDigikamBlurToolPlugin
