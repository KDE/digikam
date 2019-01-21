/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to reduce lens artifacts
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

#include "lensautofixtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "lensautofixtool.h"

namespace Digikam
{

LensAutoFixToolPlugin::LensAutoFixToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

LensAutoFixToolPlugin::~LensAutoFixToolPlugin()
{
}

QString LensAutoFixToolPlugin::name() const
{
    return i18n("Lens Auto-Correction");
}

QString LensAutoFixToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon LensAutoFixToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("lensautofix"));
}

QString LensAutoFixToolPlugin::description() const
{
    return i18n("A tool to fix automatically lens artifacts");
}

QString LensAutoFixToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can fix automatically lens artifacts over an image.</p>");
}

QList<DPluginAuthor> LensAutoFixToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Adrian Schroeter"),
                             QString::fromUtf8("adrian at suse dot de"),
                             QString::fromUtf8("(C) 2008"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2008-2019"))
            ;
}

void LensAutoFixToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Auto-Correction..."));
    ac->setObjectName(QLatin1String("editorwindow_enhance_lensautofix"));
    ac->setActionCategory(DPluginAction::EditorEnhance);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotLensAutoFix()));

    addAction(ac);
}

void LensAutoFixToolPlugin::slotLensAutoFix()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        LensAutoFixTool* const tool = new LensAutoFixTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
