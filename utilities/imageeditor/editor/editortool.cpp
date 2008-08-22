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
#include <QTimer>

// Local includes.

#include "editortoolsettings.h"
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
        timer    = 0;
        view     = 0;
        settings = 0;
    }

    QString             name;

    QWidget            *view;

    QPixmap             icon;

    QTimer             *timer;

    EditorToolSettings *settings;
};

EditorTool::EditorTool(QObject *parent)
          : QObject(parent)
{
    d = new EditorToolPriv;
    d->timer = new QTimer(this);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotEffect()));
}

EditorTool::~EditorTool()
{
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

EditorToolSettings* EditorTool::toolSettings() const
{
    return d->settings;
}

void EditorTool::setToolSettings(EditorToolSettings *settings)
{
    d->settings = settings;

    connect(d->settings, SIGNAL(signalOkClicked()),
            this, SLOT(slotOk()));

    connect(d->settings, SIGNAL(signalCancelClicked()),
            this, SLOT(slotCancel()));
}

void EditorTool::readSettings()
{
    d->settings->readSettings();
}

void EditorTool::saveSettings()
{
    d->settings->saveSettings();
}

void EditorTool::resetSettings()
{
    d->settings->slotDefaultSettings();
}

void EditorTool::slotTimer()
{
    d->timer->setSingleShot(true);
    d->timer->start(500);
}

void EditorTool::slotOk()
{
    saveSettings();
    finalRendering();
    emit okClicked();
}

void EditorTool::slotCancel()
{
    saveSettings();
    emit cancelClicked();
}

}  // namespace Digikam
