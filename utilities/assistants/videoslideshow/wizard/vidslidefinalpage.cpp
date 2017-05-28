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

    Private()
      : progressView(0),
        progressBar(0),
        complete(false)
    {
    }

    DHistoryView* progressView;
    DProgressWdg* progressBar;
    bool          complete;
};

VidSlideFinalPage::VidSlideFinalPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private)
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
    VidSlideWizard* const wizard = dynamic_cast<VidSlideWizard*>(assistant());

    if (!wizard)
    {
        d->progressView->addEntry(i18n("Internal Error"),
                                  DHistoryView::ErrorEntry);
        return;
    }

    d->progressView->clear();
    d->progressBar->reset();

    VidSlideSettings* const settings  = wizard->settings();

    d->progressView->addEntry(i18n("Starting to generate video slideshow..."),
                              DHistoryView::ProgressEntry);

    if (settings->selMode == VidSlideSettings::ALBUMS)
    {
        if (!wizard->iface())
            return;

        d->progressView->addEntry(i18n("%1 input albums to process:", settings->inputAlbums.count()),
                                  DHistoryView::ProgressEntry);

        foreach(const QUrl& url, wizard->iface()->albumsItems(settings->inputAlbums))
        {
            d->progressView->addEntry(QDir::toNativeSeparators(url.toLocalFile()),
                                      DHistoryView::ProgressEntry);
        }
    }
    else
    {
        d->progressView->addEntry(i18n("%1 input images to process", settings->inputImages.count()),
                                  DHistoryView::ProgressEntry);
    }

    if (!settings->inputAudio.isEmpty())
    {
        d->progressView->addEntry(i18n("%1 input audio stream to process:", settings->inputAlbums.count()),
                                  DHistoryView::ProgressEntry);

        foreach(const QUrl& url, settings->inputAudio)
        {
            d->progressView->addEntry(QDir::toNativeSeparators(url.toLocalFile()),
                                      DHistoryView::ProgressEntry);
        }
    }

    d->progressView->addEntry(i18n("Output video stream: %1",
                              QDir::toNativeSeparators(settings->outputVideo.toLocalFile())),
                              DHistoryView::ProgressEntry);
/*
    VidSlideThread encoder(this);
    encoder
    generator.setProgressWidgets(d->progressView, d->progressBar);

    if (!generator.run())
    {
        return;
    }

    if (generator.warnings())
    {
        d->progressView->addEntry(i18n("Gallery is completed, but some warnings occurred."),
                                  DHistoryView::WarningEntry);
    }
    else
    {
        d->progressView->addEntry(i18n("Gallery completed."),
                                  DHistoryView::ProgressEntry);
    }
*/
    if (settings->openInPlayer)
    {
        QDesktopServices::openUrl(settings->outputVideo);
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
