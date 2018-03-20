/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "vidslidefinalpage.h"

// Qt includes

#include <QIcon>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QStyle>
#include <QTimer>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "vidslidewizard.h"
#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dprogresswdg.h"
#include "dhistoryview.h"
#include "vidslidethread.h"
#include "vidplayerdlg.h"

namespace Digikam
{

class VidSlideFinalPage::Private
{
public:

    Private(QWizard* const dialog)
      : progressView(0),
        progressBar(0),
        complete(false),
        encoder(0),
        wizard(0),
        settings(0),
        iface(0)
    {
        wizard = dynamic_cast<VidSlideWizard*>(dialog);

        if (wizard)
        {
            iface    = wizard->iface();
            settings = wizard->settings();
        }
    }

    DHistoryView*     progressView;
    DProgressWdg*     progressBar;
    bool              complete;
    VidSlideThread*   encoder;
    VidSlideWizard*   wizard;
    VidSlideSettings* settings;
    DInfoInterface*   iface;
};

VidSlideFinalPage::VidSlideFinalPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    setObjectName(QLatin1String("FinalPage"));

    DVBox* const vbox = new DVBox(this);
    d->progressView   = new DHistoryView(vbox);
    d->progressBar    = new DProgressWdg(vbox);

    vbox->setStretchFactor(d->progressBar, 10);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("system-run")));
}

VidSlideFinalPage::~VidSlideFinalPage()
{
    if (d->encoder)
        d->encoder->cancel();

    delete d;
}

void VidSlideFinalPage::initializePage()
{
    d->complete = false;
    emit completeChanged();
    QTimer::singleShot(0, this, SLOT(slotProcess()));
}

void VidSlideFinalPage::slotProcess()
{
    if (!d->wizard)
    {
        d->progressView->addEntry(i18n("Internal Error"),
                                  DHistoryView::ErrorEntry);
        return;
    }

    d->progressView->clear();
    d->progressBar->reset();

    d->progressView->addEntry(i18n("Starting to generate video slideshow..."),
                              DHistoryView::ProgressEntry);

    d->progressView->addEntry(i18n("%1 input images to process", d->settings->inputImages.count()),
                                  DHistoryView::ProgressEntry);

    foreach(const QUrl& url, d->settings->inputImages)
    {
        d->progressView->addEntry(QDir::toNativeSeparators(url.toLocalFile()),
                                  DHistoryView::ProgressEntry);
    }

    if (!d->settings->inputAudio.isEmpty())
    {
        d->progressView->addEntry(i18n("%1 input audio stream to process:",
                                       d->settings->inputAudio.count()),
                                  DHistoryView::ProgressEntry);

        foreach(const QUrl& url, d->settings->inputAudio)
        {
            d->progressView->addEntry(QDir::toNativeSeparators(url.toLocalFile()),
                                      DHistoryView::ProgressEntry);
        }
    }

    d->progressBar->setMinimum(0);
    d->progressBar->setMaximum(d->settings->inputImages.count());

    d->encoder = new VidSlideThread(this);

    connect(d->encoder, SIGNAL(signalProgress(int)),
            d->progressBar, SLOT(setValue(int)));

    connect(d->encoder, SIGNAL(signalMessage(QString, bool)),
            this, SLOT(slotMessage(QString, bool)));

    connect(d->encoder, SIGNAL(signalDone(bool)),
            this, SLOT(slotDone(bool)));

    d->encoder->processStream(d->settings);
    d->encoder->start();
}

void VidSlideFinalPage::cleanupPage()
{
    if (d->encoder)
        d->encoder->cancel();
}

void VidSlideFinalPage::slotMessage(const QString& mess, bool err)
{
    d->progressView->addEntry(mess, err ? DHistoryView::ErrorEntry
                                        : DHistoryView::ProgressEntry);
}

void VidSlideFinalPage::slotDone(bool completed)
{
    d->progressBar->progressCompleted();
    d->complete = completed;

    if (!d->complete)
    {
        d->progressView->addEntry(i18n("Video Slideshow is not completed"),
                                  DHistoryView::WarningEntry);
    }
    else
    {
        d->progressView->addEntry(i18n("Video Slideshow completed."),
                                  DHistoryView::ProgressEntry);

        if (d->settings->outputPlayer != VidSlideSettings::NOPLAYER)
        {
            d->progressView->addEntry(i18n("Opening video stream in player..."),
                                      DHistoryView::ProgressEntry);

            if (d->settings->outputPlayer == VidSlideSettings::INTERNAL)
            {
                VidPlayerDlg* const player = new VidPlayerDlg(d->settings->outputVideo, this);
                player->show();
                player->resize(800, 600);
            }
            else
            {
                QDesktopServices::openUrl(QUrl::fromLocalFile(d->settings->outputVideo));
            }
        }
    }

    emit completeChanged();
}

bool VidSlideFinalPage::isComplete() const
{
    return d->complete;
}

} // namespace Digikam
