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

    if (d->settings->selMode == VidSlideSettings::ALBUMS)
    {
        if (!d->wizard->iface())
            return;

        d->progressView->addEntry(i18n("%1 input albums to process:", d->settings->inputAlbums.count()),
                                  DHistoryView::ProgressEntry);

        foreach(const QUrl& url, d->iface->albumsItems(d->settings->inputAlbums))
        {
            d->settings->inputImages << url;
        }
    }
    else
    {
        d->progressView->addEntry(i18n("%1 input images to process", d->settings->inputImages.count()),
                                  DHistoryView::ProgressEntry);
    }

    foreach(const QUrl& url, d->settings->inputImages)
    {
        d->progressView->addEntry(QDir::toNativeSeparators(url.toLocalFile()),
                                  DHistoryView::ProgressEntry);
    }

    if (!d->settings->inputAudio.isEmpty())
    {
        d->progressView->addEntry(i18n("%1 input audio stream to process:",
                                       d->settings->inputAlbums.count()),
                                  DHistoryView::ProgressEntry);

        foreach(const QUrl& url, d->settings->inputAudio)
        {
            d->progressView->addEntry(QDir::toNativeSeparators(url.toLocalFile()),
                                      DHistoryView::ProgressEntry);
        }
    }

    d->progressView->addEntry(i18n("Output video stream: %1",
                              QDir::toNativeSeparators(d->settings->outputVideo.toLocalFile())),
                              DHistoryView::ProgressEntry);

    d->progressBar->setMinimum(0);
    d->progressBar->setMaximum(d->settings->inputImages.count());

    d->encoder = new VidSlideThread(this);

    connect(d->encoder, SIGNAL(signalProgress(int)),
            d->progressBar, SLOT(setValue(int)));

    connect(d->encoder, SIGNAL(signalDone()),
            this, SLOT(slotDone()));

    d->encoder->processStream(d->settings);
    d->encoder->start();
}

void VidSlideFinalPage::slotDone()
{
    d->progressBar->progressCompleted();

/*
    if (generator.warnings())
    {
        d->progressView->addEntry(i18n("Video Slideshow is completed, but some warnings occurred."),
                                  DHistoryView::WarningEntry);
    }
    else
    {
        d->progressView->addEntry(i18n("Video Slideshow completed."),
                                  DHistoryView::ProgressEntry);
    }
*/
    if (d->settings->openInPlayer)
    {
        QDesktopServices::openUrl(d->settings->outputVideo);
        d->progressView->addEntry(i18n("Opening video stream in player..."),
                                  DHistoryView::ProgressEntry);
    }

    d->complete = true;
    emit completeChanged();
}

bool VidSlideFinalPage::isComplete() const
{
    return d->complete;
}

} // namespace Digikam
