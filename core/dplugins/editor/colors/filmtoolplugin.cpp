/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to emulate film color
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

#include "filmtoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "filmtool.h"

namespace Digikam
{

FilmToolPlugin::FilmToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

FilmToolPlugin::~FilmToolPlugin()
{
}

QString FilmToolPlugin::name() const
{
    return i18n("Color Negative Film");
}

QString FilmToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon FilmToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("colorneg"));
}

QString FilmToolPlugin::description() const
{
    return i18n("A tool to emulate color negative film");
}

QString FilmToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can emulate color negative film from image.</p>");
}

QList<DPluginAuthor> FilmToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QLatin1String("Matthias Welwarsky"),
                             QLatin1String("matthias at welwarsky dot de"),
                             QLatin1String("(C) 2012"))
            ;
}

void FilmToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Color Negative..."));
    ac->setObjectName(QLatin1String("editorwindow_color_film"));
    ac->setShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_I);
    ac->setActionCategory(DPluginAction::EditorColor);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotFilmTool()));

    addAction(ac);
}

void FilmToolPlugin::slotFilmTool()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        FilmTool* const tool = new FilmTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace Digikam
