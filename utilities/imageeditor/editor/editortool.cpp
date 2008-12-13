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

#include <qwidget.h>
#include <qtimer.h>

// KDE includes.

#include <kcursor.h>

// Local includes.

#include "ddebug.h"
#include "imagewidget.h"
#include "imageguidewidget.h"
#include "imagepanelwidget.h"
#include "dimgthreadedfilter.h"
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
}

EditorTool::~EditorTool()
{
    delete d;
}

void EditorTool::init()
{
    QTimer::singleShot(0, this, SLOT(slotInit()));
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

    connect(d->settings, SIGNAL(signalTryClicked()),
            this, SLOT(slotEffect()));
}

void EditorTool::setBusy(bool state)
{
    d->settings->setBusy(state);
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
    d->timer->start(500, true);
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

// ----------------------------------------------------------------

class EditorToolThreadedPriv
{

public:

    EditorToolThreadedPriv()
    {
        threadedFilter       = 0;
        currentRenderingMode = EditorToolThreaded::NoneRendering;
    }

    EditorToolThreaded::RenderingMode  currentRenderingMode;

    QString                            progressMess;

    DImgThreadedFilter                *threadedFilter;
};

EditorToolThreaded::EditorToolThreaded(QObject *parent)
                  : EditorTool(parent)
{
    d = new EditorToolThreadedPriv;
}

EditorToolThreaded::~EditorToolThreaded()
{
    delete d->threadedFilter;
    delete d;
}

EditorToolThreaded::RenderingMode EditorToolThreaded::renderingMode() const
{
    return d->currentRenderingMode;
}

void EditorToolThreaded::setProgressMessage(const QString& mess)
{
    d->progressMess = mess;
}

DImgThreadedFilter* EditorToolThreaded::filter() const
{
    return d->threadedFilter;
}

void EditorToolThreaded::setFilter(DImgThreadedFilter *filter)
{
    d->threadedFilter = filter;
}

void EditorToolThreaded::slotResized()
{
    if (d->currentRenderingMode == EditorToolThreaded::FinalRendering)
    {
       toolView()->update();
       return;
    }
    else if (d->currentRenderingMode == EditorToolThreaded::PreviewRendering)
    {
       if (filter())
          filter()->stopComputation();
    }

    QTimer::singleShot(0, this, SLOT(slotEffect()));
}

void EditorToolThreaded::slotAbort()
{
    d->currentRenderingMode = EditorToolThreaded::NoneRendering;
    EditorToolIface::editorToolIface()->setToolStopProgress();

    toolSettings()->enableButton(EditorToolSettings::Ok,      true);
    toolSettings()->enableButton(EditorToolSettings::Load,    true);
    toolSettings()->enableButton(EditorToolSettings::SaveAs,  true);
    toolSettings()->enableButton(EditorToolSettings::Try,     true);
    toolSettings()->enableButton(EditorToolSettings::Default, true);

    renderingFinished();
}

void EditorToolThreaded::customEvent(QCustomEvent *e)
{
    if (!e) return;

    DImgThreadedFilter::EventData *ed = (DImgThreadedFilter::EventData*)e->data();

    if (!ed) return;

    if (ed->starting)           // Computation in progress !
    {
        EditorToolIface::editorToolIface()->setToolProgress(ed->progress);
    }
    else
    {
        if (ed->success)        // Computation Completed !
        {
            switch (d->currentRenderingMode)
            {
                case EditorToolThreaded::PreviewRendering:
                {
                    DDebug() << "Preview " << toolName() << " completed..." << endl;
                    putPreviewData();
                    slotAbort();
                    break;
                }

                case EditorToolThreaded::FinalRendering:
                {
                    DDebug() << "Final" << toolName() << " completed..." << endl;
                    putFinalData();
                    EditorToolIface::editorToolIface()->setToolStopProgress();
                    kapp->restoreOverrideCursor();
                    emit okClicked();
                    break;
                }

                default:
                    break;
            }
        }
        else                   // Computation Failed !
        {
            switch (d->currentRenderingMode)
            {
                case EditorToolThreaded::PreviewRendering:
                {
                    DDebug() << "Preview " << toolName() << " failed..." << endl;
                    slotAbort();
                    break;
                }

                case EditorToolThreaded::FinalRendering:
                default:
                    break;
            }
        }
    }

    delete ed;
}

void EditorToolThreaded::setToolView(QWidget *view)
{
    EditorTool::setToolView(view);

    if (dynamic_cast<ImageWidget*>(view) || dynamic_cast<ImageGuideWidget*>(view) ||
        dynamic_cast<ImagePanelWidget*>(view))
    {
        connect(view, SIGNAL(signalResized()),
                this, SLOT(slotResized()));
    }
}

void EditorToolThreaded::slotOk()
{
    writeSettings();

    d->currentRenderingMode = EditorToolThreaded::FinalRendering;
    DDebug() << "Final " << toolName() << " started..." << endl;
    writeSettings();

    toolSettings()->enableButton(EditorToolSettings::Ok,      false);
    toolSettings()->enableButton(EditorToolSettings::SaveAs,  false);
    toolSettings()->enableButton(EditorToolSettings::Load,    false);
    toolSettings()->enableButton(EditorToolSettings::Default, false);
    toolSettings()->enableButton(EditorToolSettings::Try,     false);

    EditorToolIface::editorToolIface()->setToolStartProgress(d->progressMess.isEmpty() ? toolName() : d->progressMess);
    kapp->setOverrideCursor( KCursor::waitCursor() );

    if (d->threadedFilter)
    {
        delete d->threadedFilter;
        d->threadedFilter = 0;
    }

    prepareFinal();
}

void EditorToolThreaded::slotEffect()
{
    // Computation already in process.
    if (d->currentRenderingMode != EditorToolThreaded::NoneRendering)
        return;

    d->currentRenderingMode = EditorToolThreaded::PreviewRendering;
    DDebug() << "Preview " << toolName() << " started..." << endl;

    toolSettings()->enableButton(EditorToolSettings::Ok,      false);
    toolSettings()->enableButton(EditorToolSettings::SaveAs,  false);
    toolSettings()->enableButton(EditorToolSettings::Load,    false);
    toolSettings()->enableButton(EditorToolSettings::Default, false);
    toolSettings()->enableButton(EditorToolSettings::Try,     false);

    EditorToolIface::editorToolIface()->setToolStartProgress(d->progressMess.isEmpty() ? toolName() : d->progressMess);

    if (d->threadedFilter)
    {
        delete d->threadedFilter;
        d->threadedFilter = 0;
    }

    prepareEffect();
}

void EditorToolThreaded::slotCancel()
{
    writeSettings();
    slotAbort();
    emit cancelClicked();
}

}  // namespace Digikam
