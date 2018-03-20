/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "panopreviewpage.h"

// Qt includes

#include <QLabel>
#include <QMutex>
#include <QMutexLocker>
#include <QTextDocument>
#include <QStandardPaths>
#include <QVBoxLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dpreviewmanager.h"
#include "dprogresswdg.h"
#include "dhistoryview.h"
#include "panomanager.h"
#include "panoactionthread.h"
#include "enblendbinary.h"
#include "makebinary.h"
#include "nonabinary.h"
#include "pto2mkbinary.h"
#include "huginexecutorbinary.h"
#include "dlayoutbox.h"

namespace Digikam
{

class PanoPreviewPage::Private
{
public:

    explicit Private(PanoManager* const m)
      : title(0),
        previewWidget(0),
        previewBusy(false),
        previewDone(false),
        stitchingBusy(false),
        stitchingDone(false),
        postProcessing(0),
        progressBar(0),
        curProgress(0),
        totalProgress(0),
        canceled(false),
        mngr(m),
        dlg(0)
    {
    }

    QLabel*                title;

    DPreviewManager*       previewWidget;
    bool                   previewBusy;
    bool                   previewDone;
    bool                   stitchingBusy;
    bool                   stitchingDone;
    DHistoryView*          postProcessing;
    DProgressWdg*          progressBar;
    int                    curProgress, totalProgress;
    QMutex                 previewBusyMutex;      // This is a precaution in case the user does a back / next action at the wrong moment
    bool                   canceled;

    QString                output;

    PanoManager*           mngr;

    QWizard*               dlg;
};

PanoPreviewPage::PanoPreviewPage(PanoManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, i18nc("@title:window", "<b>Preview and Post-Processing</b>")),
      d(new Private(mngr))
{
    d->dlg            = dlg;

    DVBox* const vbox = new DVBox(this);

    d->title          = new QLabel(vbox);
    d->title->setOpenExternalLinks(true);
    d->title->setWordWrap(true);

    d->previewWidget  = new DPreviewManager(vbox);
    d->previewWidget->setButtonText(i18nc("@action:button", "Details..."));

    d->postProcessing = new DHistoryView(vbox);
    d->progressBar    = new DProgressWdg(vbox);

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-hugin.png")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

    connect(d->progressBar, SIGNAL(signalProgressCanceled()),
            this, SLOT(slotCancel()));
}

PanoPreviewPage::~PanoPreviewPage()
{
    delete d;
}

void PanoPreviewPage::computePreview()
{
    // Cancel any stitching being processed
    if (d->stitchingBusy)
    {
        cleanupPage();
    }

    QMutexLocker lock(&d->previewBusyMutex);

    connect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
            this, SLOT(slotPanoAction(Digikam::PanoActionData)));

    connect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
            this, SLOT(slotPanoAction(Digikam::PanoActionData)));

    d->canceled = false;

    d->previewWidget->setBusy(true, i18n("Processing Panorama Preview..."));
    d->previewDone = false;
    d->previewBusy = true;

    d->mngr->resetPreviewPto();
    d->mngr->resetPreviewUrl();
    d->mngr->resetPreviewMkUrl();
    d->mngr->thread()->generatePanoramaPreview(d->mngr->viewAndCropOptimisePtoData(),
                                               d->mngr->previewPtoUrl(),
                                               d->mngr->previewMkUrl(),
                                               d->mngr->previewUrl(),
                                               d->mngr->preProcessedMap(),
                                               d->mngr->makeBinary().path(),
                                               d->mngr->pto2MkBinary().path(),
                                               d->mngr->huginExecutorBinary().path(),
                                               d->mngr->hugin2015(),
                                               d->mngr->enblendBinary().path(),
                                               d->mngr->nonaBinary().path());
}

void PanoPreviewPage::startStitching()
{
    QMutexLocker lock(&d->previewBusyMutex);

    if (d->previewBusy)
    {
        // The real beginning of the stitching starts after preview has finished / failed
        connect(this, SIGNAL(signalPreviewFinished()), this, SLOT(slotStartStitching()));
        cleanupPage(lock);
        return;
    }

    connect(d->mngr->thread(), SIGNAL(starting(Digikam::PanoActionData)),
            this, SLOT(slotPanoAction(Digikam::PanoActionData)));

    connect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
            this, SLOT(slotPanoAction(Digikam::PanoActionData)));

    connect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
            this, SLOT(slotPanoAction(Digikam::PanoActionData)));

    d->canceled      = false;
    d->stitchingBusy = true;
    d->curProgress   = 0;

    if (d->mngr->hugin2015())
    {
        d->totalProgress = 1;
    }
    else
    {
        d->totalProgress = d->mngr->preProcessedMap().size() + 1;
    }

    d->previewWidget->hide();

    QSize panoSize      = d->mngr->viewAndCropOptimisePtoData()->project.size;
    QRect panoSelection = d->mngr->viewAndCropOptimisePtoData()->project.crop;

    if (d->previewDone)
    {
        QSize previewSize = d->mngr->previewPtoData()->project.size;
        QRectF selection  = d->previewWidget->getSelectionArea();
        QRectF proportionSelection(selection.x()      / previewSize.width(),
                                   selection.y()      / previewSize.height(),
                                   selection.width()  / previewSize.width(),
                                   selection.height() / previewSize.height());

        // At this point, if no selection area was created, proportionSelection is null,
        // hence panoSelection becomes a null rectangle
        panoSelection = QRect(proportionSelection.x()      * panoSize.width(),
                              proportionSelection.y()      * panoSize.height(),
                              proportionSelection.width()  * panoSize.width(),
                              proportionSelection.height() * panoSize.height());
    }

    d->title->setText(i18n("<qt>"
                           "<p><h1>Panorama Post-Processing</h1></p>"
                           "</qt>"));

    d->progressBar->reset();
    d->progressBar->setMaximum(d->totalProgress);
    d->progressBar->progressScheduled(i18nc("@title:group", "Panorama Post-Processing"), true, true);
    d->progressBar->progressThumbnailChanged(QIcon::fromTheme(QLatin1String("panorama")).pixmap(22, 22));
    d->progressBar->show();
    d->postProcessing->show();

    d->mngr->resetPanoPto();
    d->mngr->resetMkUrl();
    d->mngr->resetPanoUrl();
    d->mngr->thread()->compileProject(d->mngr->viewAndCropOptimisePtoData(),
                                      d->mngr->panoPtoUrl(),
                                      d->mngr->mkUrl(),
                                      d->mngr->panoUrl(),
                                      d->mngr->preProcessedMap(),
                                      d->mngr->format(),
                                      panoSelection,
                                      d->mngr->makeBinary().path(),
                                      d->mngr->pto2MkBinary().path(),
                                      d->mngr->huginExecutorBinary().path(),
                                      d->mngr->hugin2015(),
                                      d->mngr->enblendBinary().path(),
                                      d->mngr->nonaBinary().path());
}

void PanoPreviewPage::preInitializePage()
{
    d->title->setText(QString());
    d->previewWidget->show();
    d->progressBar->progressCompleted();
    d->progressBar->hide();
    d->postProcessing->hide();

    setComplete(true);
    emit completeChanged();
}

void PanoPreviewPage::initializePage()
{
    preInitializePage();

    computePreview();
}

bool PanoPreviewPage::validatePage()
{
    if (d->stitchingDone)
        return true;

    setComplete(false);
    startStitching();

    return false;
}

void PanoPreviewPage::cleanupPage()
{
    QMutexLocker lock(&d->previewBusyMutex);
    cleanupPage(lock);
}

void PanoPreviewPage::cleanupPage(QMutexLocker& /*lock*/)
{
    d->canceled = true;

    d->mngr->thread()->cancel();
    d->progressBar->progressCompleted();

    if (d->previewBusy)
    {
        d->previewBusy = false;
        d->previewWidget->setBusy(false);
        d->previewWidget->setText(i18n("Preview Processing Cancelled."));
    }
    else if (d->stitchingBusy)
    {
        d->stitchingBusy = false;
    }
}

void PanoPreviewPage::slotCancel()
{
    d->dlg->reject();
}

void PanoPreviewPage::slotStartStitching()
{
    disconnect(this, SIGNAL(signalPreviewFinished()),
               this, SLOT(slotStartStitching()));

    startStitching();
}

void PanoPreviewPage::slotPanoAction(const Digikam::PanoActionData& ad)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "SlotPanoAction (preview)";
    qCDebug(DIGIKAM_GENERAL_LOG) << "\tstarting, success, canceled, action: " << ad.starting << ad.success << d->canceled << ad.action;

    QString      text;

    QMutexLocker lock(&d->previewBusyMutex);

    qCDebug(DIGIKAM_GENERAL_LOG) << "\tbusy (preview / stitch):" << d->previewBusy << d->stitchingBusy;

    if (!ad.starting)           // Something is complete...
    {
        if (!ad.success)        // Something is failed...
        {
            switch (ad.action)
            {
                case PANO_CREATEPREVIEWPTO:
                case PANO_NONAFILEPREVIEW:
                case PANO_STITCHPREVIEW:
                case PANO_CREATEMKPREVIEW:
                case PANO_HUGINEXECUTORPREVIEW:
                {
                    if (!d->previewBusy)
                    {
                        lock.unlock();
                        emit signalPreviewFinished();
                        return;
                    }

                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    d->output      = ad.message;
                    d->previewWidget->setBusy(false);
                    d->previewBusy = false;

                    qCWarning(DIGIKAM_GENERAL_LOG) << "Preview compilation failed: " << ad.message;
                    QString errorString(i18n("<h1><b>Error</b></h1><p>%1</p>",
                                              ad.message));
                    d->previewWidget->setText(errorString);
                    d->previewWidget->setSelectionAreaPossible(false);

                    setComplete(false);
                    emit completeChanged();

                    lock.unlock();
                    emit signalPreviewFinished();

                    break;
                }
                case PANO_CREATEMK:
                {
                    if (!d->stitchingBusy)
                    {
                        return;
                    }

                    disconnect(d->mngr->thread(), SIGNAL(starting(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    d->stitchingBusy = false;
                    QString message  = i18nc("Here a makefile is a script for GNU Make",
                                             "<p><b>Cannot create makefile: </b></p><p>%1</p>",
                                             ad.message);
                    qCWarning(DIGIKAM_GENERAL_LOG) << "pto2mk call failed";
                    d->postProcessing->addEntry(message, DHistoryView::ErrorEntry);

                    setComplete(false);
                    emit completeChanged();

                    break;
                }
                case PANO_CREATEFINALPTO:
                {
                    if (!d->stitchingBusy)
                    {
                        return;
                    }

                    disconnect(d->mngr->thread(), SIGNAL(starting(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    d->stitchingBusy = false;
                    QString message  = i18nc("a project file is a .pto file, as given to hugin to build a panorama",
                                             "<p><b>Cannot create project file: </b></p><p>%1</p>",
                                             ad.message);
                    qCWarning(DIGIKAM_GENERAL_LOG) << "pto creation failed";
                    d->postProcessing->addEntry(message, DHistoryView::ErrorEntry);

                    setComplete(false);
                    emit completeChanged();

                    break;
                }
                case PANO_NONAFILE:
                {
                    if (!d->stitchingBusy)
                    {
                        return;
                    }

                    disconnect(d->mngr->thread(), SIGNAL(starting(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    d->stitchingBusy = false;
                    QString message  = i18nc("Error message for image file number %1 out of %2",
                                             "<p><b>Processing file %1 / %2: </b></p><p>%3</p>",
                                             QString::number(ad.id + 1),
                                             QString::number(d->totalProgress - 1),
                                             ad.message);
                    qCWarning(DIGIKAM_GENERAL_LOG) << "Nona call failed for file #" << ad.id;
                    d->postProcessing->addEntry(message, DHistoryView::ErrorEntry);

                    setComplete(false);
                    emit completeChanged();

                    break;
                }
                case PANO_STITCH:
                case PANO_HUGINEXECUTOR:
                {
                    if (!d->stitchingBusy)
                    {
                        return;
                    }

                    disconnect(d->mngr->thread(), SIGNAL(starting(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    d->stitchingBusy = false;
                    d->postProcessing->addEntry(i18nc("Error message for panorama compilation",
                                                         "<p><b>Panorama compilation: </b></p><p>%1</p>",
                                                         ad.message.toHtmlEscaped()), DHistoryView::ErrorEntry);
                    qCWarning(DIGIKAM_GENERAL_LOG) << "Enblend call failed";

                    setComplete(false);
                    emit completeChanged();

                    break;
                }
                default:
                {
                    qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown action (preview) " << ad.action;
                    break;
                }
            }
        }
        else                    // Something is done...
        {
            switch (ad.action)
            {
                case PANO_CREATEPREVIEWPTO:
                case PANO_CREATEMKPREVIEW:
                case PANO_NONAFILEPREVIEW:
                case PANO_CREATEFINALPTO:
                case PANO_CREATEMK:
                {
                    // Nothing to do yet, a step is finished, that's all
                    break;
                }
                case PANO_STITCHPREVIEW:
                case PANO_HUGINEXECUTORPREVIEW:
                {
                    if (d->previewBusy)
                    {
                        disconnect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
                                this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                        disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
                                this, SLOT(slotPanoAction(Digikam::PanoActionData)));
                    }

                    d->previewBusy = false;
                    d->previewDone = true;

                    lock.unlock();
                    emit signalPreviewFinished();

                    d->title->setText(i18n("<qt>"
                                           "<h1>Panorama Preview</h1>"
                                           "<p>Draw a rectangle if you want to crop the image.</p>"
                                           "<p>Pressing the <i>Next</i> button will then launch the final "
                                           "stitching process.</p>"
                                           "</qt>"));
                    d->previewWidget->setSelectionAreaPossible(true);
//                     d->previewWidget->load(QUrl::fromLocalFile(d->mngr->previewUrl().toLocalFile()), true);
                    d->previewWidget->load(d->mngr->previewUrl(), true);
                    QSize panoSize    = d->mngr->viewAndCropOptimisePtoData()->project.size;
                    QRect panoCrop    = d->mngr->viewAndCropOptimisePtoData()->project.crop;
                    QSize previewSize = d->mngr->previewPtoData()->project.size;
                    d->previewWidget->setSelectionArea(QRectF(
                        ((double) panoCrop.left())   / panoSize.width()  * previewSize.width(),
                        ((double) panoCrop.top())    / panoSize.height() * previewSize.height(),
                        ((double) panoCrop.width())  / panoSize.width()  * previewSize.width(),
                        ((double) panoCrop.height()) / panoSize.height() * previewSize.height()
                    ));

                    break;
                }
                case PANO_NONAFILE:
                {
                    QString message = i18nc("Success for image file number %1 out of %2",
                                            "Processing file %1 / %2",
                                            QString::number(ad.id + 1),
                                            QString::number(d->totalProgress - 1));
                    d->postProcessing->addEntry(message, DHistoryView::SuccessEntry);
                    d->curProgress++;
                    d->progressBar->setValue(d->curProgress);
                    d->progressBar->setMaximum(d->totalProgress);

                    break;
                }
                case PANO_STITCH:
                case PANO_HUGINEXECUTOR:
                {
                    if (!d->stitchingBusy)
                    {
                        return;
                    }

                    disconnect(d->mngr->thread(), SIGNAL(starting(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    d->stitchingBusy = false;
                    d->postProcessing->addEntry(i18nc("Success for panorama compilation", "Panorama compilation"), DHistoryView::SuccessEntry);
                    d->curProgress++;
                    d->progressBar->setValue(d->curProgress);
                    d->progressBar->setMaximum(d->totalProgress);
                    d->progressBar->progressCompleted();
                    d->progressBar->hide();
                    d->postProcessing->hide();
                    d->stitchingDone = true;

                    emit signalStitchingFinished();
                    preInitializePage();

                    break;
                }
                default:
                {
                    qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown action (preview) " << ad.action;
                    break;
                }
            }
        }
    }
    else           // Some step is started...
    {
        switch (ad.action)
        {
            case PANO_CREATEPREVIEWPTO:
            case PANO_CREATEMKPREVIEW:
            case PANO_NONAFILEPREVIEW:
            case PANO_STITCHPREVIEW:
            case PANO_CREATEFINALPTO:
            case PANO_CREATEMK:
            case PANO_HUGINEXECUTORPREVIEW:
            {
                // Nothing to do...
                break;
            }
            case PANO_NONAFILE:
            {
                QString message = i18nc("Compilation started for image file number %1 out of %2",
                                        "Processing file %1 / %2",
                                        QString::number(ad.id + 1),
                                        QString::number(d->totalProgress - 1));
                d->postProcessing->addEntry(message, DHistoryView::StartingEntry);
                break;
            }
            case PANO_STITCH:
            case PANO_HUGINEXECUTOR:
            {
                d->postProcessing->addEntry(i18nc("Panorama compilation started", "Panorama compilation"), DHistoryView::StartingEntry);
                break;
            }
            default:
            {
                qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown starting action (preview) " << ad.action;
                break;
            }
        }
    }
}

} // namespace Digikam
