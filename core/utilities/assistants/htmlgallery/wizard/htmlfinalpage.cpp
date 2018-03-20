/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dprogresswdg.h"
#include "dhistoryview.h"
#include "webbrowserdlg.h"

namespace Digikam
{

class HTMLFinalPage::Private
{
public:

    explicit Private()
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

    d->progressView->clear();
    d->progressBar->reset();

    GalleryInfo* const info  = wizard->galleryInfo();

    // Generate GalleryInfo

    qCDebug(DIGIKAM_GENERAL_LOG) << info;

    d->progressView->addEntry(i18n("Starting to generate gallery..."),
                              DHistoryView::ProgressEntry);

    if (info->m_getOption == GalleryInfo::ALBUMS)
    {
        if (!info->m_iface)
            return;

        d->progressView->addEntry(i18n("%1 albums to process:", info->m_albumList.count()),
                                  DHistoryView::ProgressEntry);

        foreach(const QUrl& url, info->m_iface->albumsItems(info->m_albumList))
        {
            d->progressView->addEntry(QDir::toNativeSeparators(url.toLocalFile()),
                                      DHistoryView::ProgressEntry);
        }
    }
    else
    {
        d->progressView->addEntry(i18n("%1 items to process", info->m_imageList.count()),
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

    QUrl url = info->destUrl().adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1String("/index.html"));

    switch (info->openInBrowser())
    {
        case GalleryConfig::DESKTOP:
        {
            QDesktopServices::openUrl(url);
            d->progressView->addEntry(i18n("Opening gallery with default desktop browser..."),
                                      DHistoryView::ProgressEntry);
            break;
        }

        case GalleryConfig::INTERNAL:
        {
            WebBrowserDlg* const browser = new WebBrowserDlg(url, this);
            browser->show();
            d->progressView->addEntry(i18n("Opening gallery with internal browser..."),
                                      DHistoryView::ProgressEntry);
            break;
        }

        default:
            break;
    }

    d->complete = true;
    emit completeChanged();
}

bool HTMLFinalPage::isComplete() const
{
    return d->complete;
}

} // namespace Digikam
