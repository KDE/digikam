/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to add film grain over an image.
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

#include "filmgraintoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "filmgraintool.h"

namespace DigikamEditorFilmGrainToolPlugin
{

FilmGrainToolPlugin::FilmGrainToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

FilmGrainToolPlugin::~FilmGrainToolPlugin()
{
}

QString FilmGrainToolPlugin::name() const
{
    return i18n("Film Grain");
}

QString FilmGrainToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon FilmGrainToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("filmgrain"));
}

QString FilmGrainToolPlugin::description() const
{
    return i18n("A tool to add film grain over an image");
}

QString FilmGrainToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can add film grain over an image.</p>");
}

QList<DPluginAuthor> FilmGrainToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Marcel Wiesweg"),
                             QString::fromUtf8("marcel dot wiesweg at gmx dot de"),
                             QString::fromUtf8("(C) 2006-2012"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2019"))
            ;
}

void FilmGrainToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Add Film Grain..."));
    ac->setObjectName(QLatin1String("editorwindow_filter_filmgrain"));
    ac->setActionCategory(DPluginAction::EditorFilters);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotFilmGrain()));

    addAction(ac);
}

void FilmGrainToolPlugin::slotFilmGrain()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        FilmGrainTool* const tool = new FilmGrainTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace DigikamEditorFilmGrainToolPlugin
