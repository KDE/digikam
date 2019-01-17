/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to apply blur effects.
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

#include "blurfxtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "blurfxtool.h"

namespace Digikam
{

BlurFXToolPlugin::BlurFXToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

BlurFXToolPlugin::~BlurFXToolPlugin()
{
}

QString BlurFXToolPlugin::name() const
{
    return i18n("Blur Effects");
}

QString BlurFXToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon BlurFXToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("blurfx"));
}

QString BlurFXToolPlugin::description() const
{
    return i18n("A tool to apply blur effects to an image");
}

QString BlurFXToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can apply blur effects to an image.</p>");
}

QList<DPluginAuthor> BlurFXToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Marcel Wiesweg"),
                             QLatin1String("marcel dot wiesweg at gmx dot de"),
                             QLatin1String("(C) 2006-2012"))
            << DPluginAuthor(QLatin1String("Gilles Caulier"),
                             QLatin1String("caulier dot gilles at gmail dot com"),
                             QLatin1String("(C) 2005-2019"))
            ;
}
    
void BlurFXToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Blur Effects..."));
    ac->setObjectName(QLatin1String("editorwindow_filter_blurfx"));
    ac->setActionCategory(DPluginAction::EditorFilters);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotBlurFX()));

    addAction(ac);
}

void BlurFXToolPlugin::slotBlurFX()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        BlurFXTool* const tool = new BlurFXTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
