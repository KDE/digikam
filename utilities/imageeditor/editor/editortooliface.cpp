/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : Image editor interface used by editor tools.
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

#include <qwidget.h>

// Local includes.

#include "sidebar.h"
#include "statusprogressbar.h"
#include "editortool.h"
#include "editortoolsettings.h"
#include "editorstackview.h"
#include "editorwindow.h"
#include "editortooliface.h"
#include "editortooliface.moc"

namespace Digikam
{

class EditorToolIfacePriv
{

public:

    EditorToolIfacePriv()
    {
        prevTab = 0;
        editor  = 0;
        tool    = 0;
    }

    QWidget      *prevTab;

    EditorTool   *tool;

    EditorWindow *editor;
};

EditorToolIface* EditorToolIface::m_iface = 0;

EditorToolIface* EditorToolIface::editorToolIface()
{
    return m_iface;
}

EditorToolIface::EditorToolIface(EditorWindow *editor)
               : QObject()
{
    d = new EditorToolIfacePriv;
    d->editor = editor;
    m_iface   = this;

    connect(d->editor->m_nameLabel, SIGNAL(signalCancelButtonPressed()),
            this, SLOT(slotToolAborted()));
}

EditorToolIface::~EditorToolIface()
{
    delete d;
    if (m_iface == this)
        m_iface = 0;
}

EditorTool* EditorToolIface::currentTool() const
{
    return d->tool;
}

void EditorToolIface::loadTool(EditorTool* tool)
{
    if (d->tool) unLoadTool();

    d->tool = tool;
    d->editor->editorStackView()->setToolView(d->tool->toolView());
    d->editor->editorStackView()->setViewMode(EditorStackView::ToolViewMode);
    d->prevTab = d->editor->rightSideBar()->getActiveTab();
    d->editor->rightSideBar()->appendTab(d->tool->toolSettings(), d->tool->toolIcon(), d->tool->toolName());
    d->editor->rightSideBar()->setActiveTab(d->tool->toolSettings());
    d->editor->toggleActions(false);
}

void EditorToolIface::unLoadTool()
{
    if (!d->tool) return;

    d->editor->editorStackView()->setViewMode(EditorStackView::CanvasMode);
    d->editor->editorStackView()->setToolView(0);
    d->editor->rightSideBar()->deleteTab(d->tool->toolSettings());
    d->editor->rightSideBar()->setActiveTab(d->prevTab);
    d->editor->toggleActions(true);
    delete d->tool;
    d->tool = 0;
}

void EditorToolIface::setToolStartProgress(const QString& toolName)
{
    d->editor->setToolStartProgress(toolName);
}

void EditorToolIface::setToolProgress(int progress)
{
    d->editor->setToolProgress(progress);
}

void EditorToolIface::setToolStopProgress()
{
    d->editor->setToolStopProgress();
}

void EditorToolIface::slotToolAborted()
{
    EditorToolThreaded *tool = dynamic_cast<EditorToolThreaded*>(d->tool);
    if (tool) tool->slotAbort();
}

}  // namespace Digikam
