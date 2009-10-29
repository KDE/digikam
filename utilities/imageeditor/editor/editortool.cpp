/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : editor tool template class.
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "editortool.h"
#include "editortool.moc"

// Qt includes

#include <QWidget>
#include <QTimer>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimgthreadedfilter.h"
#include "imagewidget.h"
#include "imageguidewidget.h"
#include "imagepanelwidget.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "editortoolsettings.h"
#include "editortooliface.h"

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
          : QObject(parent), d(new EditorToolPriv)
{
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
    // Will be unblocked in slotInit()
    // This will prevent resize event signals emit during tool init.
    d->view->blockSignals(true);
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

    connect(d->settings, SIGNAL(signalChannelChanged()),
            this, SLOT(slotChannelChanged()));

    connect(d->settings, SIGNAL(signalColorsChanged()),
            this, SLOT(slotColorsChanged()));

    connect(d->settings, SIGNAL(signalScaleChanged()),
            this, SLOT(slotScaleChanged()));

    // Will be unblocked in slotInit()
    // This will prevent signals emit during tool init.
    d->settings->blockSignals(true);
}

void EditorTool::slotInit()
{
    readSettings();
    // Unlock signals from preview and settings widgets when init is done.
    d->view->blockSignals(false);
    d->settings->blockSignals(false);
}

void EditorTool::setToolHelp(const QString& anchor)
{
    d->helpAnchor = anchor;
    // TODO: use this anchor with editor Help menu
}

QString EditorTool::toolHelp() const
{
    if (d->helpAnchor.isEmpty())
        return (objectName() + QString(".anchor"));

    return d->helpAnchor;
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

void EditorTool::slotCloseTool()
{
    slotCancel();
}

// ----------------------------------------------------------------

class EditorToolThreadedPriv
{

public:

    EditorToolThreadedPriv()
    {
        delFilter            = true;
        threadedFilter       = 0;
        currentRenderingMode = EditorToolThreaded::NoneRendering;
    }

    bool                               delFilter;

    EditorToolThreaded::RenderingMode  currentRenderingMode;

    QString                            progressMess;

    DImgThreadedFilter                *threadedFilter;
};

EditorToolThreaded::EditorToolThreaded(QObject *parent)
                  : EditorTool(parent), d(new EditorToolThreadedPriv)
{
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

    connect(d->threadedFilter, SIGNAL(started()),
            this, SLOT(slotFilterStarted()));

    connect(d->threadedFilter, SIGNAL(finished(bool)),
            this, SLOT(slotFilterFinished(bool)));

    connect(d->threadedFilter, SIGNAL(progress(int)),
            this, SLOT(slotFilterProgress(int)));

    d->threadedFilter->startFilter();
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
          filter()->cancelFilter();
    }

    QTimer::singleShot(0, this, SLOT(slotEffect()));
}

void EditorToolThreaded::slotAbort()
{
    d->currentRenderingMode = EditorToolThreaded::NoneRendering;

    if (filter())
        filter()->cancelFilter();

    EditorToolIface::editorToolIface()->setToolStopProgress();

    toolSettings()->enableButton(EditorToolSettings::Ok,      true);
    toolSettings()->enableButton(EditorToolSettings::Load,    true);
    toolSettings()->enableButton(EditorToolSettings::SaveAs,  true);
    toolSettings()->enableButton(EditorToolSettings::Try,     true);
    toolSettings()->enableButton(EditorToolSettings::Default, true);

    renderingFinished();
}

void EditorToolThreaded::slotFilterStarted()
{
}

void EditorToolThreaded::slotFilterFinished(bool success)
{
    if (success)        // Computation Completed !
    {
        switch (d->currentRenderingMode)
        {
            case EditorToolThreaded::PreviewRendering:
            {
                kDebug() << "Preview " << toolName() << " completed...";
                putPreviewData();
                slotAbort();
                break;
            }

            case EditorToolThreaded::FinalRendering:
            {
                kDebug() << "Final" << toolName() << " completed...";
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
                kDebug() << "Preview " << toolName() << " failed...";
                slotAbort();
                break;
            }

            case EditorToolThreaded::FinalRendering:
            default:
                break;
        }
    }
}

void EditorToolThreaded::slotFilterProgress(int progress)
{
    EditorToolIface::editorToolIface()->setToolProgress(progress);
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
    kDebug() << "Final " << toolName() << " started...";
    writeSettings();

    toolSettings()->enableButton(EditorToolSettings::Ok,      false);
    toolSettings()->enableButton(EditorToolSettings::SaveAs,  false);
    toolSettings()->enableButton(EditorToolSettings::Load,    false);
    toolSettings()->enableButton(EditorToolSettings::Default, false);
    toolSettings()->enableButton(EditorToolSettings::Try,     false);

    EditorToolIface::editorToolIface()->setToolStartProgress(d->progressMess.isEmpty() ? toolName() : d->progressMess);
    kapp->setOverrideCursor( Qt::WaitCursor );

    if (d->delFilter && d->threadedFilter)
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
    kDebug() << "Preview " << toolName() << " started...";

    toolSettings()->enableButton(EditorToolSettings::Ok,      false);
    toolSettings()->enableButton(EditorToolSettings::SaveAs,  false);
    toolSettings()->enableButton(EditorToolSettings::Load,    false);
    toolSettings()->enableButton(EditorToolSettings::Default, false);
    toolSettings()->enableButton(EditorToolSettings::Try,     false);

    EditorToolIface::editorToolIface()->setToolStartProgress(d->progressMess.isEmpty() ? toolName() : d->progressMess);

    if (d->delFilter && d->threadedFilter)
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
    kapp->restoreOverrideCursor();
    emit cancelClicked();
}

void EditorToolThreaded::deleteFilterInstance(bool b)
{
    d->delFilter = b;
}

}  // namespace Digikam
