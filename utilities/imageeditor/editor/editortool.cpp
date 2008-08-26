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

    QString             helpAnchor;
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

    QTimer::singleShot(0, this, SLOT(slotInit()));
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

void EditorTool::setToolHelp(const QString& anchor)
{
    d->helpAnchor = anchor;
    // TODO: use this anchor with editor Help menu
}

QString EditorTool::toolHelp() const
{
    if (d->helpAnchor.isEmpty())
        return (name() + QString(".anchor"));

    return d->helpAnchor;
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

    connect(d->settings, SIGNAL(signalDefaultClicked()),
            this, SLOT(slotResetSettings()));

    connect(d->settings, SIGNAL(signalSaveAsClicked()),
            this, SLOT(slotSaveAsSettings()));

    connect(d->settings, SIGNAL(signalLoadClicked()),
            this, SLOT(slotLoadSettings()));
}

void EditorTool::readSettings()
{
    d->settings->readSettings();
}

void EditorTool::writeSettings()
{
    d->settings->writeSettings();
}

void EditorTool::slotResetSettings()
{
    d->settings->resetSettings();
}

void EditorTool::slotTimer()
{
    d->timer->setSingleShot(true);
    d->timer->start(500);
}

void EditorTool::slotOk()
{
    writeSettings();
    finalRendering();
    emit okClicked();
}

void EditorTool::slotCancel()
{
    writeSettings();
    emit cancelClicked();
}

void EditorTool::slotInit()
{
    readSettings();
}

}  // namespace Digikam
