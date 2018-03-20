/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a tool to create panorama by fusion of several images.
 * Acknowledge : based on the expoblending tool
 *
 * Copyright (C) 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "panopreprocesspage.h"

// Qt includes

#include <QDir>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QPixmap>
#include <QPushButton>
#include <QCheckBox>
#include <QMutex>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>

// #include <QList>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "cpcleanbinary.h"
#include "cpfindbinary.h"
#include "panomanager.h"
#include "panoactionthread.h"
#include "dlayoutbox.h"
#include "dworkingpixmap.h"

namespace Digikam
{

class PanoPreProcessPage::Private
{
public:

    explicit Private()
      : progressCount(0),
        progressLabel(0),
        progressTimer(0),
        preprocessingDone(false),
        canceled(false),
        nbFilesProcessed(0),
        title(0),
        celesteCheckBox(0),
        detailsText(0),
        progressPix(DWorkingPixmap()),
        mngr(0)
    {
    }

    int                        progressCount;
    QLabel*                    progressLabel;
    QTimer*                    progressTimer;
    QMutex                     progressMutex;      // This is a precaution in case the user does a back / next action at the wrong moment
    bool                       preprocessingDone;
    bool                       canceled;

    int                        nbFilesProcessed;
    QMutex                     nbFilesProcessed_mutex;

    QLabel*                    title;

    QCheckBox*                 celesteCheckBox;

    QTextBrowser*              detailsText;

    DWorkingPixmap             progressPix;

    PanoManager*               mngr;
};

PanoPreProcessPage::PanoPreProcessPage(PanoManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, i18nc("@title:window", "<b>Pre-Processing Images</b>")),
      d(new Private)
{
    d->mngr                 = mngr;
    d->progressTimer        = new QTimer(this);
    DVBox* const vbox       = new DVBox(this);
    d->title                = new QLabel(vbox);
    d->title->setWordWrap(true);
    d->title->setOpenExternalLinks(true);

    KConfig config;
    KConfigGroup group  = config.group("Panorama Settings");

    d->celesteCheckBox  = new QCheckBox(i18nc("@option:check", "Detect moving skies"), vbox);
    d->celesteCheckBox->setChecked(group.readEntry("Celeste", false));
    d->celesteCheckBox->setToolTip(i18nc("@info:tooltip", "Automatic detection of clouds to prevent wrong keypoints matching "
                                         "between images due to moving clouds."));
    d->celesteCheckBox->setWhatsThis(i18nc("@info:whatsthis", "<b>Detect moving skies</b>: During the control points selection and matching, "
                                           "this option discards any points that are associated to a possible cloud. This "
                                           "is useful to prevent moving clouds from altering the control points matching "
                                           "process."));
    vbox->setStretchFactor(new QWidget(vbox), 2);

    d->detailsText    = new QTextBrowser(vbox);
    d->detailsText->hide();

    vbox->setStretchFactor(new QWidget(vbox), 2);

    d->progressLabel = new QLabel(vbox);
    d->progressLabel->setAlignment(Qt::AlignCenter);

    vbox->setStretchFactor(new QWidget(vbox), 10);

    setPageWidget(vbox);

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-preprocessing.png")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

PanoPreProcessPage::~PanoPreProcessPage()
{
    KConfig config;
    KConfigGroup group = config.group("Panorama Settings");
    group.writeEntry("Celeste", d->celesteCheckBox->isChecked());
    config.sync();

    delete d;
}

void PanoPreProcessPage::process()
{
    QMutexLocker lock(&d->progressMutex);

    d->title->setText(i18n("<qt>"
                           "<p>Pre-processing is in progress, please wait.</p>"
                           "<p>This can take a while...</p>"
                           "</qt>"));

    d->celesteCheckBox->hide();
    d->progressTimer->start(300);

    connect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
            this, SLOT(slotPanoAction(Digikam::PanoActionData)));
    connect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
            this, SLOT(slotPanoAction(Digikam::PanoActionData)));

//  d->nbFilesProcessed = 0;

    d->mngr->resetBasePto();
    d->mngr->resetCpFindPto();
    d->mngr->resetCpCleanPto();
    d->mngr->preProcessedMap().clear();
    d->mngr->thread()->preProcessFiles(d->mngr->itemsList(),
                                       d->mngr->preProcessedMap(),
                                       d->mngr->basePtoUrl(),
                                       d->mngr->cpFindPtoUrl(),
                                       d->mngr->cpCleanPtoUrl(),
                                       d->celesteCheckBox->isChecked(),
//                                     d->mngr->hdr(),
                                       d->mngr->format(),
                                       d->mngr->gPano(),
                                       d->mngr->cpFindBinary().version(),
                                       d->mngr->cpCleanBinary().path(),
                                       d->mngr->cpFindBinary().path());
}

void PanoPreProcessPage::initializePage()
{
    d->title->setText(i18n("<qt>"
                           "<p>Now, we will pre-process images before stitching them.</p>"
                           "<p>Pre-processing operations include Raw demosaicing. Raw images will be converted "
                           "to 16-bit sRGB images with auto-gamma.</p>"
                           "<p>Pre-processing also include a calculation of some control points to match "
                           "overlaps between images. For that purpose, the <b>%1</b> program from the "
                           "<a href='%2'>%3</a> project will be used.</p>"
                           "<p>Press \"Next\" to start pre-processing.</p>"
                           "</qt>",
                           QDir::toNativeSeparators(d->mngr->cpFindBinary().path()),
                           d->mngr->cpFindBinary().url().url(),
                           d->mngr->cpFindBinary().projectName()));

    d->detailsText->hide();
    d->celesteCheckBox->show();

    d->canceled          = false;
    d->preprocessingDone = false;

    setComplete(true);
    emit completeChanged();
}

bool PanoPreProcessPage::validatePage()
{
    if (d->preprocessingDone)
        return true;

    setComplete(false);
    process();

    return false;
}

void PanoPreProcessPage::cleanupPage()
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

void PanoPreProcessPage::slotProgressTimerDone()
{
    d->progressLabel->setPixmap(d->progressPix.frameAt(d->progressCount));
    if (d->progressPix.frameCount())
    {
        d->progressCount = (d->progressCount + 1) % d->progressPix.frameCount();
    }
    d->progressTimer->start(300);
}

void PanoPreProcessPage::slotPanoAction(const Digikam::PanoActionData& ad)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "SlotPanoAction (preprocessing)";
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
                case PANO_PREPROCESS_INPUT:
                case PANO_CREATEPTO:
                case PANO_CPFIND:
                case PANO_CPCLEAN:
                {
                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));
                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
                               this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    qCWarning(DIGIKAM_GENERAL_LOG) << "Job failed (preprocessing): " << ad.action;

                    if (d->detailsText->isHidden())  // Ensures only the first failed task is shown
                    {
                        d->title->setText(i18n("<qt>"
                                                "<h1>Pre-processing has failed.</h1>"
                                                "<p>See processing messages below.</p>"
                                                "</qt>"));
                        d->progressTimer->stop();
                        d->celesteCheckBox->hide();
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
                    qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown action (preprocessing) " << ad.action;
                    break;
                }
            }
        }
        else                    // Something is done...
        {
            switch (ad.action)
            {
                case PANO_PREPROCESS_INPUT:
                {
//                     QMutexLocker nbProcessed(&d->nbFilesProcessed_mutex);

//                     d->nbFilesProcessed++;

                    break;
                }
                case PANO_CREATEPTO:
                case PANO_CPFIND:
                {
                    // Nothing to do, that just another step towards the end
                    break;
                }
                case PANO_CPCLEAN:
                {
                    disconnect(d->mngr->thread(), SIGNAL(stepFinished(Digikam::PanoActionData)),
                            this, SLOT(slotPanoAction(Digikam::PanoActionData)));
                    disconnect(d->mngr->thread(), SIGNAL(jobCollectionFinished(Digikam::PanoActionData)),
                            this, SLOT(slotPanoAction(Digikam::PanoActionData)));

                    d->progressTimer->stop();
                    d->progressLabel->clear();
                    d->preprocessingDone = true;

                    emit signalPreProcessed();
                    initializePage();

                    break;
                }
                default:
                {
                    qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown action (preprocessing) " << ad.action;
                    break;
                }
            }
        }
    }
}

} // namespace Digikam
