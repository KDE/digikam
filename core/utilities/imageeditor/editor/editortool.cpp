/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-08-20
 * Description : editor tool template class.
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QWidget>
#include <QTimer>
#include <QIcon>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dimgthreadedfilter.h"
#include "dimgthreadedanalyser.h"
#include "imageguidewidget.h"
#include "imageregionwidget.h"
#include "histogramwidget.h"
#include "histogrambox.h"
#include "editortoolsettings.h"
#include "editortooliface.h"

namespace Digikam
{

class EditorTool::Private
{

public:

    Private() :
        initPreview(false),
        version(0),
        view(0),
        timer(0),
        settings(0),
        category(FilterAction::ReproducibleFilter)
    {
    }

    bool                   initPreview;
    QString                helpAnchor;
    QString                name;
    int                    version;

    QWidget*               view;
    QIcon                  icon;
    QTimer*                timer;

    EditorToolSettings*    settings;

    FilterAction::Category category;
};

EditorTool::EditorTool(QObject* const parent)
    : QObject(parent),
      d(new Private)
{
    d->timer = new QTimer(this);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotPreview()));
}

EditorTool::~EditorTool()
{
    delete d->settings;
    delete d->view;
    delete d;
}

void EditorTool::init()
{
    QTimer::singleShot(0, this, SLOT(slotInit()));
}

void EditorTool::setInitPreview(bool b)
{
    d->initPreview = b;
}

QIcon EditorTool::toolIcon() const
{
    return d->icon;
}

void EditorTool::setToolIcon(const QIcon& icon)
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

int EditorTool::toolVersion() const
{
    return d->version;
}

void EditorTool::setToolVersion(const int version)
{
    d->version = version;
}

FilterAction::Category EditorTool::toolCategory() const
{
    return d->category;
}

void EditorTool::setToolCategory(const FilterAction::Category category)
{
    d->category = category;
}

void EditorTool::setPreviewModeMask(int mask)
{
    EditorToolIface::editorToolIface()->setPreviewModeMask(mask);
}

QWidget* EditorTool::toolView() const
{
    return d->view;
}

void EditorTool::setToolView(QWidget* const view)
{
    d->view = view;
    // Will be unblocked in slotInit()
    // This will prevent resize event signals emit during tool init.
    d->view->blockSignals(true);

    ImageGuideWidget* const wgt = dynamic_cast<ImageGuideWidget*>(d->view);

    if (wgt)
    {
        connect(d->view, SIGNAL(spotPositionChangedFromOriginal(Digikam::DColor,QPoint)),
                this, SLOT(slotUpdateSpotInfo(Digikam::DColor,QPoint)));

        connect(d->view, SIGNAL(spotPositionChangedFromTarget(Digikam::DColor,QPoint)),
                this, SLOT(slotUpdateSpotInfo(Digikam::DColor,QPoint)));
    }
}

EditorToolSettings* EditorTool::toolSettings() const
{
    return d->settings;
}

void EditorTool::setToolSettings(EditorToolSettings* const settings)
{
    d->settings = settings;
    d->settings->setToolIcon(toolIcon());
    d->settings->setToolName(toolName());

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
            this, SLOT(slotPreview()));

    connect(d->settings, SIGNAL(signalChannelChanged()),
            this, SLOT(slotChannelChanged()));

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

    if (d->initPreview)
        slotTimer();
}

void EditorTool::setToolHelp(const QString& anchor)
{
    d->helpAnchor = anchor;
    // TODO: use this anchor with editor Help menu
}

QString EditorTool::toolHelp() const
{
    if (d->helpAnchor.isEmpty())
    {
        return (objectName() + QLatin1String(".anchor"));
    }

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

void EditorTool::slotApplyTool()
{
    slotOk();
}

void EditorTool::slotPreviewModeChanged()
{
    slotPreview();
}

void EditorTool::setBackgroundColor(const QColor& bg)
{
    ImageGuideWidget* const view = dynamic_cast<ImageGuideWidget*>(d->view);
    QPalette palette;

    if (view)
    {
        palette.setColor(view->backgroundRole(), bg);
        view->setPalette(palette);
    }

    ImageRegionWidget* const view2 = dynamic_cast<ImageRegionWidget*>(d->view);

    if (view2)
    {
        palette.setColor(view2->backgroundRole(), bg);
        view2->setPalette(palette);
    }
}

void EditorTool::ICCSettingsChanged()
{
    ImageGuideWidget* const view = dynamic_cast<ImageGuideWidget*>(d->view);

    if (view)
    {
        view->ICCSettingsChanged();
    }

    ImageRegionWidget* const view2 = dynamic_cast<ImageRegionWidget*>(d->view);

    if (view2)
    {
        view2->ICCSettingsChanged();
    }
}

void EditorTool::exposureSettingsChanged()
{
    ImageGuideWidget* const view = dynamic_cast<ImageGuideWidget*>(d->view);

    if (view)
    {
        view->exposureSettingsChanged();
    }

    ImageRegionWidget* const view2 = dynamic_cast<ImageRegionWidget*>(d->view);

    if (view2)
    {
        view2->exposureSettingsChanged();
    }
}

void EditorTool::setToolInfoMessage(const QString& txt)
{
    EditorToolIface::editorToolIface()->setToolInfoMessage(txt);
}

void EditorTool::slotUpdateSpotInfo(const DColor& col, const QPoint& point)
{
    DColor color = col;
    setToolInfoMessage(i18n("(%1,%2) RGBA:%3,%4,%5,%6",
                            point.x(), point.y(),
                            color.red(), color.green(),
                            color.blue(), color.alpha()));
}

// ----------------------------------------------------------------

class EditorToolThreaded::Private
{

public:

    Private() :
        delFilter(true),
        currentRenderingMode(EditorToolThreaded::NoneRendering),
        threadedFilter(0),
        threadedAnalyser(0)
    {
    }

    bool                              delFilter;

    EditorToolThreaded::RenderingMode currentRenderingMode;

    QString                           progressMess;

    DImgThreadedFilter*               threadedFilter;
    DImgThreadedAnalyser*             threadedAnalyser;
};

EditorToolThreaded::EditorToolThreaded(QObject* const parent)
    : EditorTool(parent), d(new Private)
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

void EditorToolThreaded::slotInit()
{
    EditorTool::slotInit();

    QWidget* const view = toolView();

    if (dynamic_cast<ImageGuideWidget*>(view))
    {
        connect(view, SIGNAL(signalResized()),
                this, SLOT(slotResized()));
    }

    if (dynamic_cast<ImageRegionWidget*>(view))
    {
        connect(view, SIGNAL(signalOriginalClipFocusChanged()),
                this, SLOT(slotTimer()));
    }
}

void EditorToolThreaded::setFilter(DImgThreadedFilter* const filter)
{
    delete d->threadedFilter;
    d->threadedFilter = filter;

    connect(d->threadedFilter, SIGNAL(started()),
            this, SLOT(slotFilterStarted()));

    connect(d->threadedFilter, SIGNAL(finished(bool)),
            this, SLOT(slotFilterFinished(bool)));

    connect(d->threadedFilter, SIGNAL(progress(int)),
            this, SLOT(slotProgress(int)));

    d->threadedFilter->startFilter();
}

DImgThreadedAnalyser* EditorToolThreaded::analyser() const
{
    return d->threadedAnalyser;
}

void EditorToolThreaded::setAnalyser(DImgThreadedAnalyser* const analyser)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Analys " << toolName() << " started...";

    toolSettings()->enableButton(EditorToolSettings::Ok,      false);
    toolSettings()->enableButton(EditorToolSettings::SaveAs,  false);
    toolSettings()->enableButton(EditorToolSettings::Load,    false);
    toolSettings()->enableButton(EditorToolSettings::Default, false);
    toolSettings()->enableButton(EditorToolSettings::Try,     false);
    toolView()->setEnabled(false);

    EditorToolIface::editorToolIface()->setToolStartProgress(d->progressMess.isEmpty() ? toolName() : d->progressMess);
    qApp->setOverrideCursor(Qt::WaitCursor);

    delete d->threadedAnalyser;
    d->threadedAnalyser = analyser;

    connect(d->threadedAnalyser, SIGNAL(started()),
            this, SLOT(slotAnalyserStarted()));

    connect(d->threadedAnalyser, SIGNAL(finished(bool)),
            this, SLOT(slotAnalyserFinished(bool)));

    connect(d->threadedAnalyser, SIGNAL(progress(int)),
            this, SLOT(slotProgress(int)));

    d->threadedAnalyser->startFilter();
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
        {
            filter()->cancelFilter();
        }
    }

    QTimer::singleShot(0, this, SLOT(slotPreview()));
}

void EditorToolThreaded::slotAbort()
{
    d->currentRenderingMode = EditorToolThreaded::NoneRendering;

    if (analyser())
    {
        analyser()->cancelFilter();
    }

    if (filter())
    {
        filter()->cancelFilter();
    }

    EditorToolIface::editorToolIface()->setToolStopProgress();

    toolSettings()->enableButton(EditorToolSettings::Ok,      true);
    toolSettings()->enableButton(EditorToolSettings::Load,    true);
    toolSettings()->enableButton(EditorToolSettings::SaveAs,  true);
    toolSettings()->enableButton(EditorToolSettings::Try,     true);
    toolSettings()->enableButton(EditorToolSettings::Default, true);
    toolView()->setEnabled(true);

    qApp->restoreOverrideCursor();

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
                qCDebug(DIGIKAM_GENERAL_LOG) << "Preview " << toolName() << " completed...";
                setPreviewImage();
                slotAbort();
                break;
            }

            case EditorToolThreaded::FinalRendering:
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Final" << toolName() << " completed...";
                setFinalImage();
                EditorToolIface::editorToolIface()->setToolStopProgress();
                qApp->restoreOverrideCursor();
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
                qCDebug(DIGIKAM_GENERAL_LOG) << "Preview " << toolName() << " failed...";
                slotAbort();
                break;
            }

            case EditorToolThreaded::FinalRendering:
            default:
                break;
        }
    }
}

void EditorToolThreaded::slotProgress(int progress)
{
    EditorToolIface::editorToolIface()->setToolProgress(progress);
}

void EditorToolThreaded::slotAnalyserStarted()
{
}

void EditorToolThreaded::slotAnalyserFinished(bool success)
{
    if (success)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Analys " << toolName() << " completed...";
        analyserCompleted();
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Analys " << toolName() << " failed...";
        slotAbort();
    }
}

void EditorToolThreaded::slotOk()
{
    // Computation already in process.
    if (d->currentRenderingMode != EditorToolThreaded::PreviewRendering)
    {
        // See bug #305916 : cancel preview before.
        slotAbort();
    }

    writeSettings();

    d->currentRenderingMode = EditorToolThreaded::FinalRendering;
    qCDebug(DIGIKAM_GENERAL_LOG) << "Final " << toolName() << " started...";

    toolSettings()->enableButton(EditorToolSettings::Ok,      false);
    toolSettings()->enableButton(EditorToolSettings::SaveAs,  false);
    toolSettings()->enableButton(EditorToolSettings::Load,    false);
    toolSettings()->enableButton(EditorToolSettings::Default, false);
    toolSettings()->enableButton(EditorToolSettings::Try,     false);
    toolView()->setEnabled(false);

    EditorToolIface::editorToolIface()->setToolStartProgress(d->progressMess.isEmpty() ? toolName() : d->progressMess);
    qApp->setOverrideCursor(Qt::WaitCursor);

    if (d->delFilter && d->threadedFilter)
    {
        delete d->threadedFilter;
        d->threadedFilter = 0;
    }

    prepareFinal();
}

void EditorToolThreaded::slotPreview()
{
    // Computation already in process.
    if (d->currentRenderingMode != EditorToolThreaded::NoneRendering)
    {
        return;
    }

    d->currentRenderingMode = EditorToolThreaded::PreviewRendering;
    qCDebug(DIGIKAM_GENERAL_LOG) << "Preview " << toolName() << " started...";

    toolSettings()->enableButton(EditorToolSettings::Ok,      false);
    toolSettings()->enableButton(EditorToolSettings::SaveAs,  false);
    toolSettings()->enableButton(EditorToolSettings::Load,    false);
    toolSettings()->enableButton(EditorToolSettings::Default, false);
    toolSettings()->enableButton(EditorToolSettings::Try,     false);
    toolView()->setEnabled(false);

    EditorToolIface::editorToolIface()->setToolStartProgress(d->progressMess.isEmpty() ? toolName() : d->progressMess);
    qApp->setOverrideCursor(Qt::WaitCursor);

    if (d->delFilter && d->threadedFilter)
    {
        delete d->threadedFilter;
        d->threadedFilter = 0;
    }

    preparePreview();
}

void EditorToolThreaded::slotCancel()
{
    writeSettings();
    slotAbort();
    emit cancelClicked();
}

void EditorToolThreaded::deleteFilterInstance(bool b)
{
    d->delFilter = b;
}

}  // namespace Digikam
