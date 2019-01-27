/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to crop an image with ratio.
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

#include "ratiocroptoolplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "editorwindow.h"
#include "ratiocroptool.h"

namespace EditorDigikamRatioCropToolPlugin
{

RatioCropToolPlugin::RatioCropToolPlugin(QObject* const parent)
    : DPluginEditor(parent)
{
}

RatioCropToolPlugin::~RatioCropToolPlugin()
{
}

QString RatioCropToolPlugin::name() const
{
    return i18n("Aspect Ratio Crop");
}

QString RatioCropToolPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RatioCropToolPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("transform-crop"));
}

QString RatioCropToolPlugin::description() const
{
    return i18n("A tool to crop an image with ratio");
}

QString RatioCropToolPlugin::details() const
{
    return i18n("<p>This Image Editor tool can crop an image with ratio.</p>");
}

QList<DPluginAuthor> RatioCropToolPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Jaromir Malenko"),
                             QString::fromUtf8("malenko at email dot cz"),
                             QString::fromUtf8("(C) 2007"))
            << DPluginAuthor(QString::fromUtf8("Roberto Castagnola"),
                             QString::fromUtf8("roberto dot castagnola at gmail dot com"),
                             QString::fromUtf8("(C) 2008"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2004-2019"))
            ;
}

void RatioCropToolPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Aspect Ratio Crop..."));
    ac->setObjectName(QLatin1String("editorwindow_transform_ratiocrop"));
    ac->setActionCategory(DPluginAction::EditorTransform);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotRatioCrop()));

    addAction(ac);
}

void RatioCropToolPlugin::slotRatioCrop()
{
    EditorWindow* const editor = dynamic_cast<EditorWindow*>(sender()->parent());

    if (editor)
    {
        RatioCropTool* const tool = new RatioCropTool(editor);
        tool->setPlugin(this);
        editor->loadTool(tool);
    }
}

} // namespace EditorDigikamRatioCropToolPlugin
