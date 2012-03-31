/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : Image editor interface used by editor tools.
 *
 * Copyright (C) 2008-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

namespace Digikam
{

class EditorToolIface::EditorToolIfacePriv
{

public:

    EditorToolIfacePriv() :
        tool(0),
        editor(0)
    {
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
    {
        m_iface = 0;
    }
}

EditorTool* EditorToolIface::currentTool() const
{
    return d->tool;
}

void EditorToolIface::loadTool(EditorTool* tool)
{
    if (d->tool)
    {
        unLoadTool();
    }

    d->tool = tool;
    d->editor->editorStackView()->setToolView(d->tool->toolView());
    d->editor->editorStackView()->setViewMode(EditorStackView::ToolViewMode);
    d->editor->rightSideBar()->appendTab(d->tool->toolSettings(), d->tool->toolIcon(), d->tool->toolName());
    d->editor->rightSideBar()->setActiveTab(d->tool->toolSettings());
    d->editor->toggleActions(false);
    d->editor->toggleToolActions(d->tool);

    // If editor tool has zoomable preview, switch on zoom actions.
    d->editor->toggleZoomActions(d->editor->editorStackView()->isZoomablePreview());

    ImageGuideWidget* view = dynamic_cast<ImageGuideWidget*>(d->tool->toolView());

    if (view)
    {
        connect(d->editor, SIGNAL(signalPreviewModeChanged(int)),
                view, SLOT(slotPreviewModeChanged(int)));

        view->slotPreviewModeChanged(d->editor->previewMode());
    }

    ImageRegionWidget* view2 = dynamic_cast<ImageRegionWidget*>(d->tool->toolView());

    if (view2)
    {
        connect(d->editor, SIGNAL(signalPreviewModeChanged(int)),
                view2, SLOT(slotPreviewModeChanged(int)));

        if (d->editor->editorStackView()->canvas()->fitToWindow())
        {
            view2->toggleFitToWindow();
        }
        else
        {
            view2->setZoomFactor(d->editor->editorStackView()->canvas()->zoomFactor());
            QPoint tl = d->editor->editorStackView()->canvas()->visibleArea().topLeft();
            view2->setContentsPos(tl.x(), tl.y());
        }

        view2->slotPreviewModeChanged(d->editor->previewMode());
    }

    themeChanged();
    updateExposureSettings();
    updateICCSettings();
    setToolInfoMessage(QString());

    connect(d->editor, SIGNAL(signalPreviewModeChanged(int)),
            d->tool, SLOT(slotPreviewModeChanged()));

    connect(d->tool, SIGNAL(okClicked()),
            this, SLOT(slotToolApplied()));
}

void EditorToolIface::unLoadTool()
{
    if (!d->tool)
    {
        return;
    }

    d->editor->editorStackView()->setViewMode(EditorStackView::CanvasMode);
    d->editor->editorStackView()->setToolView(0);
    d->editor->rightSideBar()->deleteTab(d->tool->toolSettings());
    d->editor->toggleActions(true);
    d->editor->toggleToolActions();
    d->editor->setPreviewModeMask(PreviewToolBar::NoPreviewMode);

    // To restore canvas zoom level in zoom combobox.
    if (!d->editor->editorStackView()->canvas()->fitToWindow())
    {
        d->editor->editorStackView()->setZoomFactor(d->editor->editorStackView()->canvas()->zoomFactor());
    }

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
    d->editor->toggleZoomActions(!d->editor->editorStackView()->isZoomablePreview());
}

void EditorToolIface::setToolProgress(int progress)
{
    d->editor->setToolProgress(progress);
}

void EditorToolIface::setToolStopProgress()
{
    d->editor->setToolStopProgress();
    d->editor->toggleZoomActions(d->editor->editorStackView()->isZoomablePreview());
}

void EditorToolIface::slotToolAborted()
{
    EditorToolThreaded* tool = dynamic_cast<EditorToolThreaded*>(d->tool);

    if (tool)
    {
        tool->slotAbort();
    }
}

void EditorToolIface::slotCloseTool()
{
    EditorTool* tool = dynamic_cast<EditorTool*>(d->tool);

    if (tool)
    {
        tool->slotCloseTool();
    }
}

void EditorToolIface::slotApplyTool()
{
    EditorTool* tool = dynamic_cast<EditorTool*>(d->tool);

    if (tool)
    {
        tool->slotApplyTool();
    }
}

void EditorToolIface::setupICC()
{
    d->editor->setupICC();
}

void EditorToolIface::themeChanged()
{
    EditorTool* tool = dynamic_cast<EditorTool*>(d->tool);

    if (tool)
    {
        tool->setBackgroundColor(d->editor->m_bgColor);
    }
}

void EditorToolIface::updateICCSettings()
{
    EditorTool* tool = dynamic_cast<EditorTool*>(d->tool);

    if (tool)
    {
        tool->ICCSettingsChanged();
    }
}

void EditorToolIface::updateExposureSettings()
{
    ExposureSettingsContainer* expoSettings = d->editor->exposureSettings();
    d->editor->editorStackView()->canvas()->setExposureSettings(expoSettings);
    EditorTool* tool = dynamic_cast<EditorTool*>(d->tool);

    if (tool)
    {
        tool->exposureSettingsChanged();
    }
}

void EditorToolIface::setPreviewModeMask(int mask)
{
    d->editor->setPreviewModeMask(mask);
}

void EditorToolIface::slotToolApplied()
{
    emit (d->editor->signalToolApplied());
}

}  // namespace Digikam
