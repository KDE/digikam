/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : Image editor interface used by editor tools.
 *
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "editortooliface.moc"

// Qt includes

#include <QWidget>

// Local includes

#include "sidebar.h"
#include "canvas.h"
#include "statusprogressbar.h"
#include "editortool.h"
#include "editortoolsettings.h"
#include "editorstackview.h"
#include "editorwindow.h"
#include "imageguidewidget.h"
#include "imageregionwidget.h"
#include "imagepanelwidget.h"

namespace Digikam
{

class EditorToolIfacePriv
{

public:

    EditorToolIfacePriv()
    {
        editor = 0;
        tool   = 0;
    }

    EditorTool*   tool;
    EditorWindow* editor;
};

EditorToolIface* EditorToolIface::m_iface = 0;

EditorToolIface* EditorToolIface::editorToolIface()
{
    return m_iface;
}

EditorToolIface::EditorToolIface(EditorWindow* editor)
               : QObject(), d(new EditorToolIfacePriv)
{
    d->editor = editor;
    m_iface   = this;
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
    d->editor->rightSideBar()->appendTab(d->tool->toolSettings(), d->tool->toolIcon(), d->tool->toolName());
    d->editor->rightSideBar()->setActiveTab(d->tool->toolSettings());
    d->editor->toggleActions(false);

    // If editor tool has zoomable preview, switch on zoom actions.
    if (d->editor->editorStackView()->previewWidget())
        d->editor->toggleZoomActions(true);

    ImageGuideWidget* view = dynamic_cast<ImageGuideWidget*>(d->tool->toolView());
    if (view)
    {
        connect(d->editor, SIGNAL(signalPreviewModeChanged(int)),
                view, SLOT(slotPreviewModeChanged(int)));
                
        view->slotPreviewModeChanged(d->editor->previewMode());
    }

    ImagePanelWidget* view2 = dynamic_cast<ImagePanelWidget*>(d->tool->toolView());
    if (view2)
    {
        connect(d->editor, SIGNAL(signalPreviewModeChanged(int)),
                view2->previewWidget(), SLOT(slotPreviewModeChanged(int)));
                
        view2->previewWidget()->slotPreviewModeChanged(d->editor->previewMode());
    }

    updateExposureSettings();
    updateICCSettings();
    setToolInfoMessage(QString());    
}

void EditorToolIface::unLoadTool()
{
    if (!d->tool) return;

    d->editor->editorStackView()->setViewMode(EditorStackView::CanvasMode);
    d->editor->editorStackView()->setToolView(0);
    d->editor->rightSideBar()->deleteTab(d->tool->toolSettings());
    d->editor->toggleActions(true);
    d->editor->setPreviewModeMask(PreviewToolBar::NoPreviewMode);
    // To restore canvas zoom level in zoom combobox.
    if (!d->editor->editorStackView()->canvas()->fitToWindow())
        d->editor->editorStackView()->setZoomFactor(d->editor->editorStackView()->canvas()->zoomFactor());
    delete d->tool;
    d->tool = 0;
    
    // Reset info label in status bar with canvas selection info.
    d->editor->slotSelected(!d->editor->m_canvas->getSelectedArea().isNull());
}

void EditorToolIface::setToolInfoMessage(const QString& txt)
{
    d->editor->setToolInfoMessage(txt);
}

void EditorToolIface::setToolStartProgress(const QString& toolName)
{
    d->editor->setToolStartProgress(toolName);
    if (d->editor->editorStackView()->previewWidget())
        d->editor->toggleZoomActions(false);
}

void EditorToolIface::setToolProgress(int progress)
{
    d->editor->setToolProgress(progress);
}

void EditorToolIface::setToolStopProgress()
{
    d->editor->setToolStopProgress();
    if (d->editor->editorStackView()->previewWidget())
        d->editor->toggleZoomActions(true);
}

void EditorToolIface::slotToolAborted()
{
    EditorToolThreaded *tool = dynamic_cast<EditorToolThreaded*>(d->tool);
    if (tool) tool->slotAbort();
}

void EditorToolIface::slotCloseTool()
{
    EditorTool *tool = dynamic_cast<EditorTool*>(d->tool);
    if (tool) tool->slotCloseTool();
}

void EditorToolIface::setupICC()
{
    d->editor->setupICC();
}

void EditorToolIface::updateICCSettings()
{
    ICCSettingsContainer* cmSettings = d->editor->cmSettings();
    d->editor->editorStackView()->canvas()->setICCSettings(cmSettings);
    EditorTool *tool = dynamic_cast<EditorTool*>(d->tool);
    if (tool) tool->ICCSettingsChanged();
}

void EditorToolIface::updateExposureSettings()
{
    ExposureSettingsContainer* expoSettings = d->editor->exposureSettings();
    d->editor->editorStackView()->canvas()->setExposureSettings(expoSettings);
    EditorTool *tool = dynamic_cast<EditorTool*>(d->tool);
    if (tool) tool->exposureSettingsChanged();
}

void EditorToolIface::setPreviewModeMask(int mask)
{
    d->editor->setPreviewModeMask(mask);
}

}  // namespace Digikam
