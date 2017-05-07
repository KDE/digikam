/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "htmlfinalpage.h"

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

#include "htmlwizard.h"
#include "abstractthemeparameter.h"
#include "galleryinfo.h"
#include "gallerygenerator.h"
#include "dwidgetutils.h"
#include "digikam_debug.h"
#include "dprogresswdg.h"
#include "dhistoryview.h"

namespace Digikam
{

class HTMLFinalPage::Private
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

HTMLFinalPage::HTMLFinalPage(QWizard* const dialog, const QString& title)
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

HTMLFinalPage::~HTMLFinalPage()
{
    delete d;
}

void HTMLFinalPage::initializePage()
{
    d->complete = false;
    emit completeChanged();
    QTimer::singleShot(0, this, SLOT(slotProcess()));
}

void HTMLFinalPage::slotProcess()
{
    HTMLWizard* const wizard = dynamic_cast<HTMLWizard*>(assistant());

    if (!wizard)
    {
        d->progressView->addEntry(i18n("Internal Error"),
                                  DHistoryView::ErrorEntry);
        return;
    }

    GalleryInfo* const info  = wizard->galleryInfo();

    // Generate GalleryInfo

    qCDebug(DIGIKAM_GENERAL_LOG) << info;

    d->progressView->addEntry(i18n("Starting to generate gallery..."),
                              DHistoryView::ProgressEntry);

    if (info->mGetOption == GalleryInfo::ALBUMS)
    {
        if (!info->mIface)
            return;

        d->progressView->addEntry(i18n("%1 albums to process:", info->mAlbumList.count()),
                                  DHistoryView::ProgressEntry);

        foreach(QUrl url, info->mIface->albumsItems(info->mAlbumList))
        {
            d->progressView->addEntry(QDir::toNativeSeparators(url.toLocalFile()),
                                      DHistoryView::ProgressEntry);
        }
    }
    else
    {
        d->progressView->addEntry(i18n("%1 items to process", info->mImageList.count()),
                                  DHistoryView::ProgressEntry);
    }

    d->progressView->addEntry(i18n("Output directory: %1",
                              QDir::toNativeSeparators(info->destUrl().toLocalFile())),
                              DHistoryView::ProgressEntry);

    GalleryGenerator generator(info);
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

    if (info->openInBrowser())
    {
        QUrl url = info->destUrl().adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + QLatin1String("/index.html"));
        QDesktopServices::openUrl(url);
        d->progressView->addEntry(i18n("Opening gallery in browser..."),
                                  DHistoryView::ProgressEntry);
    }

    d->complete = true;
    emit completeChanged();
}

bool HTMLFinalPage::isComplete() const
{
    return d->complete;
}

} // namespace Digikam
