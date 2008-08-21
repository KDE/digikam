/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : editor tool template class.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

// Qt includes.

#include <QWidget>
//Added by qt3to4:
#include <QPixmap>

// Local includes.

#include "editortooliface.h"
#include "editortool.h"
#include "editortool.moc"

namespace Digikam
{

class EditorToolPriv
{

public:

    EditorToolPriv()
    {
        view     = 0;
        settings = 0;
    }

    QString  name;

    QWidget *view;
    QWidget *settings;

    QPixmap  icon;
};

EditorTool::EditorTool(QObject *parent)
          : QObject(parent)
{
    d = new EditorToolPriv;
}

EditorTool::~EditorTool()
{
    saveSettings();
    delete d;
}

QPixmap EditorTool::toolIcon() const
{
    return d->icon;
}

void EditorTool::setToolIcon(const QPixmap& icon)
{
    d->icon = icon;
}

QString EditorTool::toolName() const
{
    return d->name;
}

void EditorTool::setToolName(const QString& name)
{
    d->name = name;
}

QWidget* EditorTool::toolView() const
{
    return d->view;
}

void EditorTool::setToolView(QWidget *view)
{
    d->view = view;
}

QWidget* EditorTool::toolSettings() const
{
    return d->settings;
}

void EditorTool::setToolSettings(QWidget *settings)
{
    d->settings = settings;
}

}  // namespace Digikam
