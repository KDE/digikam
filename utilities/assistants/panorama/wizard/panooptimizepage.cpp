/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#include "panooptimizepage.h"

// Qt includes

#include <QDir>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>
#include <QCheckBox>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "autooptimiserbinary.h"
#include "panomodifybinary.h"
#include "panomanager.h"
#include "panoactionthread.h"
#include "dlayoutbox.h"
#include "dworkingpixmap.h"

namespace Digikam
{

class PanoOptimizePage::Private
{
public:

    explicit Private()
      : progressCount(0),
        progressLabel(0),
        progressTimer(0),
        optimisationDone(false),
        canceled(false),
        title(0),
//      preprocessResults(0),
        horizonCheckbox(0),
//      projectionAndSizeCheckbox(0),
        detailsText(0),
        progressPix(DWorkingPixmap()),
        mngr(0)
    {
    }

    int                        progressCount;
    QLabel*                    progressLabel;
    QTimer*                    progressTimer;
    QMutex                     progressMutex;      // This is a precaution in case the user does a back / next action at the wrong moment
    bool                       optimisationDone;
    bool                       canceled;

    QLabel*                    title;
//  QLabel*                    preprocessResults;

    QCheckBox*                 horizonCheckbox;
//  QCheckBox*                 projectionAndSizeCheckboxs;

    QTextBrowser*              detailsText;

    DWorkingPixmap             progressPix;

    PanoManager*               mngr;
};

PanoOptimizePage::PanoOptimizePage(PanoManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, i18nc("@title:window", "<b>Optimization</b>")),
      d(new Private)
{
    d->mngr                         = mngr;
    d->progressTimer                = new QTimer(this);
    DVBox* const vbox               = new DVBox(this);
    d->title                        = new QLabel(vbox);
    d->title->setOpenExternalLinks(true);
    d->title->setWordWrap(true);

    KConfig config;
    KConfigGroup group              = config.group("Panorama Settings");

    d->horizonCheckbox              = new QCheckBox(i18nc("@option:check", "Level horizon"), vbox);
    d->horizonCheckbox->setChecked(group.readEntry("Horizon", true));
    d->horizonCheckbox->setToolTip(i18nc("@info:tooltip", "Detect the horizon and adapt the project to make it horizontal."));
    d->horizonCheckbox->setWhatsThis(i18nc("@info:whatsthis", "<b>Level horizon</b>: Detect the horizon and adapt the projection so that "
                                           "the detected horizon is an horizontal line in the final panorama"));
/*
    if (!d->mngr->gPano())
    {
        d->projectionAndSizeCheckbox = new QCheckBox(i18nc("@option:check", "Automatic projection and output aspect"), vbox);
        d->projectionAndSizeCheckbox->setChecked(group.readEntry("Output Projection And Size", true));
        d->projectionAndSizeCheckbox->setToolTip(i18nc("@info:tooltip", "Adapt the projection of the panorama and the area rendered on the "
                                                       "resulting projection so that every photo fits in the resulting "
                                                       "panorama."));
        d->projectionAndSizeCheckbox->setWhatsThis(i18nc("@info:whatsthis", "<b>Automatic projection and output aspect</b>: Automatically "
                                                         "adapt the projection and the area rendered of the panorama to "
                                                         "get every photos into the panorama."));
    }
    else
    {
        d->projectionAndSizeCheckbox = new QCheckBox(i18nc("@option:check", "Automatic output aspect"), vbox);
        d->projectionAndSizeCheckbox->setChecked(group.readEntry("Output Projection And Size", true));
        d->projectionAndSizeCheckbox->setToolTip(i18nc("@info:tooltip", "Adapt the area rendered on the resulting projection so that "
                                                       "every photo fits in the resulting panorama."));
        d->projectionAndSizeCheckbox->setWhatsThis(i18nc("@info:whatsthis", "<b>Automatic output aspect</b>: Automatically adapt the area "
                                                         "rendered of the panorama to get every photos into the panorama."));
    }
*/

//  d->preprocessResults            = new QLabel(vbox);

    vbox->setStretchFactor(new QWidget(vbox), 2);

    d->detailsText    = new QTextBrowser(vbox);
    d->detailsText->hide();

    vbox->setStretchFactor(new QWidget(vbox), 2);

    d->progressLabel        = new QLabel(vbox);
    d->progressLabel->setAlignment(Qt::AlignCenter);

    vbox->setStretchFactor(new QWidget(vbox), 10);

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-hugin.png")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

PanoOptimizePage::~PanoOptimizePage()
{
    KConfig config;
    KConfigGroup group = config.group("Panorama Settings");
    group.writeEntry("Horizon", d->horizonCheckbox->isChecked());
//  group.writeEntry("Output Projection And Size", d->projectionAndSizeCheckbox->isChecked());
    config.sync();

    delete d;
}

void PanoOptimizePage::process()
{
    QMutexLocker lock(&d->progressMutex);

    d->title->setText(i18n("<qt>"
                           "<p>Optimization is in progress, please wait.</p>"
                           "<p>This can take a while...</p>"
                           "</qt>"));
    d->horizonCheckbox->hide();
//  d->projectionAndSizeCheckbox->hide();
    d->progressTimer->start(300);

    connect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
            this, SLOT(slotPanoAction(Digikam::PanoActionData)));
    connect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
            this, SLOT(slotPanoAction(Digikam::PanoActionData)));

    d->mngr->resetAutoOptimisePto();
    d->mngr->resetViewAndCropOptimisePto();
    d->mngr->thread()->optimizeProject(d->mngr->cpCleanPtoUrl(),
                                       d->mngr->autoOptimisePtoUrl(),
                                       d->mngr->viewAndCropOptimisePtoUrl(),
                                       d->horizonCheckbox->isChecked(),
                                       d->mngr->gPano(),
                                       d->mngr->autoOptimiserBinary().path(),
                                       d->mngr->panoModifyBinary().path());
}

void PanoOptimizePage::initializePage()
{
    d->title->setText(i18n("<qt>"
                           "<p>The optimization step according to your settings is ready to be performed.</p>"
                           "<p>This step can include an automatic leveling of the horizon, and also "
                           "an automatic projection selection and size</p>"
                           "<p>To perform this operation, the <command>%1</command> program from the "
                           "<a href='%2'>%3</a> project will be used.</p>"
                           "<p>Press the \"Next\" button to run the optimization.</p>"
                           "</qt>",
                           QDir::toNativeSeparators(d->mngr->autoOptimiserBinary().path()),
                           d->mngr->autoOptimiserBinary().url().url(),
                           d->mngr->autoOptimiserBinary().projectName()));

//  QPair<double, int> result = d->mngr->cpFindUrlData().standardDeviation();
//  d->preprocessResults->setText(i18n("Alignment error: %1px", result.first / ((double) result.second)));
    d->detailsText->hide();
    d->horizonCheckbox->show();
//  d->projectionAndSizeCheckbox->show();

    d->canceled = false;
    d->optimisationDone = false;

    setComplete(true);
    emit completeChanged();
}

bool PanoOptimizePage::validatePage()
{
    if (d->optimisationDone)
        return true;

    setComplete(false);
    process();

    return false;
}

void PanoOptimizePage::cleanupPage()
{
    d->canceled = true;

    disconnect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
               this, SLOT(slotPanoAction(Digikam::PanoActionData)));
    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

    d->mngr->thread()->cancel();

    QMutexLocker lock(&d->progressMutex);

    if (d->progressTimer->isActive())
    {
        d->progressTimer->stop();
        d->progressLabel->clear();
    }
}

void PanoOptimizePage::slotProgressTimerDone()
{
    d->progressLabel->setPixmap(d->progressPix.frameAt(d->progressCount));
    if (d->progressPix.frameCount())
    {
        d->progressCount = (d->progressCount + 1) % d->progressPix.frameCount();
    }
    d->progressTimer->start(300);
}

void PanoOptimizePage::slotPanoAction(const Digikam::PanoActionData& ad)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "SlotPanoAction (optimize)";
    qCDebug(DIGIKAM_GENERAL_LOG) << "starting, success, canceled, action: " << ad.starting << ad.success << d->canceled << ad.action;

    QString text;

    QMutexLocker lock(&d->progressMutex);

    if (!ad.starting)           // Something is complete...
    {
        if (!ad.success)        // Something is failed...
        {
            if (d->canceled)    // In that case, the error is expected
            {
                return;
            }

            switch (ad.action)
            {
                case PANO_OPTIMIZE:
                case PANO_AUTOCROP:
                {
                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));
                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    qCWarning(DIGIKAM_GENERAL_LOG) << "Job failed (optimize): " << ad.action;

                    if (d->detailsText->isHidden())
                    {
                        d->title->setText(i18n("<qt>"
                                               "<h1>Optimization has failed.</h1>"
                                               "<p>See processing messages below.</p>"
                                               "</qt>"));
                        d->progressTimer->stop();
                        d->horizonCheckbox->hide();
//                      d->projectionAndSizeCheckbox->hide();
                        d->detailsText->show();
                        d->progressLabel->clear();
                        d->detailsText->setText(ad.message);

                        setComplete(false);
                        emit completeChanged();
                    }
                    break;
                }
                default:
                {
                    qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown action (optimize) " << ad.action;
                    break;
                }
            }
        }
        else                    // Something is done...
        {
            switch (ad.action)
            {
                case PANO_OPTIMIZE:
                {
                    return;
                }
                case PANO_AUTOCROP:
                {
                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));
                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    d->progressTimer->stop();
                    d->progressLabel->clear();
                    d->optimisationDone = true;

                    emit signalOptimized();
                    initializePage();

                    break;
                }
                default:
                {
                    qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown action (optimize) " << ad.action;
                    break;
                }
            }
        }
    }
}

} // namespace Digikam
