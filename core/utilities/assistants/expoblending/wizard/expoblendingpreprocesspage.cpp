/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a tool to blend bracketed images.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2015      by Benjamin Girault, <benjamin dot girault at gmail dot com>
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

#include "expoblendingpreprocesspage.h"

// Qt includes

#include <QDir>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QPixmap>
#include <QPushButton>
#include <QCheckBox>
#include <QStyle>
#include <QApplication>
#include <QTextBrowser>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <kconfig.h>

// Local includes

#include "digikam_debug.h"
#include "alignbinary.h"
#include "expoblendingmanager.h"
#include "expoblendingthread.h"
#include "dlayoutbox.h"
#include "dworkingpixmap.h"

namespace Digikam
{

class ExpoBlendingPreProcessPage::Private
{
public:

    explicit Private()
    {
        progressPix   = DWorkingPixmap();
        progressCount = 0;
        progressTimer = 0;
        progressLabel = 0,
        mngr          = 0;
        title         = 0;
        alignCheckBox = 0;
        detailsText   = 0;
    }

    int                  progressCount;
    QLabel*              progressLabel;
    QTimer*              progressTimer;

    QLabel*              title;

    QCheckBox*           alignCheckBox;

    QTextBrowser*        detailsText;

    DWorkingPixmap       progressPix;

    ExpoBlendingManager* mngr;
};

ExpoBlendingPreProcessPage::ExpoBlendingPreProcessPage(ExpoBlendingManager* const mngr, QWizard* const dlg)
    : DWizardPage(dlg, i18nc("@title:window", "<b>Pre-Processing Bracketed Images</b>")),
      d(new Private)
{
    d->mngr           = mngr;
    d->progressTimer  = new QTimer(this);
    DVBox* const vbox = new DVBox(this);
    d->title          = new QLabel(vbox);
    d->title->setWordWrap(true);
    d->title->setOpenExternalLinks(true);

    KConfig config;
    KConfigGroup group = config.group("ExpoBlending Settings");
    d->alignCheckBox   = new QCheckBox(i18nc("@option:check", "Align bracketed images"), vbox);
    d->alignCheckBox->setChecked(group.readEntry("Auto Alignment", true));

    vbox->setStretchFactor(new QWidget(vbox), 2);

    d->detailsText     = new QTextBrowser(vbox);
    d->detailsText->hide();

    vbox->setStretchFactor(new QWidget(vbox), 2);

    d->progressLabel   = new QLabel(vbox);
    d->progressLabel->setAlignment(Qt::AlignCenter);

    vbox->setStretchFactor(new QWidget(vbox), 10);

    setPageWidget(vbox);

    resetTitle();

    QPixmap leftPix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/assistant-preprocessing.png")));
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

    connect(d->mngr->thread(), SIGNAL(starting(Digikam::ExpoBlendingActionData)),
            this, SLOT(slotExpoBlendingAction(Digikam::ExpoBlendingActionData)));

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTimerDone()));
}

ExpoBlendingPreProcessPage::~ExpoBlendingPreProcessPage()
{
    KConfig config;
    KConfigGroup group = config.group("ExpoBlending Settings");
    group.writeEntry("Auto Alignment", d->alignCheckBox->isChecked());
    config.sync();

    delete d;
}

void ExpoBlendingPreProcessPage::resetTitle()
{
    d->title->setText(i18n("<qt>"
                           "<p>Now, we will pre-process bracketed images before fusing them.</p>"
                           "<p>To perform auto-alignment, the <b>%1</b> program from the "
                           "<a href='%2'>%3</a> project will be used. "
                           "Alignment must be performed if you have not used a tripod to take bracketed images. "
                           "Alignment operations can take a while.</p>"
                           "<p>Pre-processing operations include Raw demosaicing. Raw images will be converted "
                           "to 16-bit sRGB images with auto-gamma.</p>"
                           "<p>Press \"Next\" to start pre-processing.</p>"
                           "</qt>",
                           QDir::toNativeSeparators(d->mngr->alignBinary().path()),
                           d->mngr->alignBinary().url().url(),
                           d->mngr->alignBinary().projectName()));
    d->detailsText->hide();
    d->alignCheckBox->show();
}

void ExpoBlendingPreProcessPage::process()
{
    d->title->setText(i18n("<qt>"
                           "<p>Pre-processing is under progress, please wait.</p>"
                           "<p>This can take a while...</p>"
                           "</qt>"));

    d->alignCheckBox->hide();
    d->progressTimer->start(300);

    connect(d->mngr->thread(), SIGNAL(finished(Digikam::ExpoBlendingActionData)),
            this, SLOT(slotExpoBlendingAction(Digikam::ExpoBlendingActionData)));

    d->mngr->thread()->setPreProcessingSettings(d->alignCheckBox->isChecked());
    d->mngr->thread()->preProcessFiles(d->mngr->itemsList(), d->mngr->alignBinary().path());

    if (!d->mngr->thread()->isRunning())
        d->mngr->thread()->start();
}

void ExpoBlendingPreProcessPage::cancel()
{
    disconnect(d->mngr->thread(), SIGNAL(finished(Digikam::ExpoBlendingActionData)),
               this, SLOT(slotExpoBlendingAction(Digikam::ExpoBlendingActionData)));

    d->mngr->thread()->cancel();
    d->progressTimer->stop();
    d->progressLabel->clear();
    resetTitle();
}

void ExpoBlendingPreProcessPage::slotProgressTimerDone()
{
    d->progressLabel->setPixmap(d->progressPix.frameAt(d->progressCount));

    d->progressCount++;

    if (d->progressCount == 8)
        d->progressCount = 0;

    d->progressTimer->start(300);
}

void ExpoBlendingPreProcessPage::slotExpoBlendingAction(const Digikam::ExpoBlendingActionData& ad)
{
    QString text;

    if (!ad.starting)           // Something is complete...
    {
        if (!ad.success)        // Something is failed...
        {
            switch (ad.action)
            {
                case(EXPOBLENDING_PREPROCESSING):
                {
                    d->title->setText(i18n("<qt>"
                                           "<p>Pre-processing has failed.</p>"
                                           "<p>Please check your bracketed images stack...</p>"
                                           "<p>See processing messages below.</p>"
                                           "</qt>"));
                    d->progressTimer->stop();
                    d->alignCheckBox->hide();
                    d->detailsText->show();
                    d->progressLabel->clear();
                    d->detailsText->setText(ad.message);
                    emit signalPreProcessed(ExpoBlendingItemUrlsMap());
                    break;
                }
                default:
                {
                    qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown action";
                    break;
                }
            }
        }
        else                    // Something is done...
        {
            switch (ad.action)
            {
                case(EXPOBLENDING_PREPROCESSING):
                {
                    d->progressTimer->stop();
                    d->progressLabel->clear();
                    emit signalPreProcessed(ad.preProcessedUrlsMap);
                    break;
                }
                default:
                {
                    qCWarning(DIGIKAM_GENERAL_LOG) << "Unknown action";
                    break;
                }
            }
        }
    }
}

}   // namespace Digikam
