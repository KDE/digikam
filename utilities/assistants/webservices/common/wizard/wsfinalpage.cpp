/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items to web services.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "wsfinalpage.h"

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

#include "wswizard.h"
#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dprogresswdg.h"
#include "dhistoryview.h"
//#include "wsprocess.h"

namespace Digikam
{

class WSFinalPage::Private
{
public:

    explicit Private(QWizard* const dialog)
      : progressView(0),
        progressBar(0),
        complete(false),
//        processor(0),
        wizard(0),
        settings(0),
        iface(0)
    {
        wizard = dynamic_cast<WSWizard*>(dialog);

        if (wizard)
        {
            iface    = wizard->iface();
            settings = wizard->settings();
        }
    }

    DHistoryView*     progressView;
    DProgressWdg*     progressBar;
    bool              complete;
//    MailProcess*      processor;
    WSWizard*         wizard;
    WSSettings*       settings;
    DInfoInterface*   iface;
};

WSFinalPage::WSFinalPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    DVBox* const vbox = new DVBox(this);
    d->progressView   = new DHistoryView(vbox);
    d->progressBar    = new DProgressWdg(vbox);

    vbox->setStretchFactor(d->progressBar, 10);
    vbox->setContentsMargins(QMargins());
    vbox->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("WS_send")));
}

WSFinalPage::~WSFinalPage()
{
/*
    if (d->processor)
        d->processor->slotCancel();
*/
    delete d;
}

void WSFinalPage::initializePage()
{
    d->complete = false;
    emit completeChanged();
    QTimer::singleShot(0, this, SLOT(slotProcess()));
}

void WSFinalPage::slotDone()
{
    d->complete = true;
    emit completeChanged();
}

void WSFinalPage::slotProcess()
{
    if (!d->wizard)
    {
        d->progressView->addEntry(i18n("Internal Error"),
                                  DHistoryView::ErrorEntry);
        return;
    }

    d->progressView->clear();
    d->progressBar->reset();

    d->progressView->addEntry(i18n("Preparing file to export by mail..."),
                              DHistoryView::ProgressEntry);

    foreach (const QUrl& url, d->settings->inputImages)
    {
        d->settings->setMailUrl(url, QUrl());
    }

    d->progressView->addEntry(i18n("%1 input items to process", d->settings->itemsList.count()),
                                  DHistoryView::ProgressEntry);

    for (QMap<QUrl, QUrl>::const_iterator it = d->settings->itemsList.constBegin();
         it != d->settings->itemsList.constEnd(); ++it)
    {
        d->progressView->addEntry(QDir::toNativeSeparators(it.key().toLocalFile()),
                                  DHistoryView::ProgressEntry);
    }


    d->progressBar->setMinimum(0);
    d->progressBar->setMaximum(d->settings->itemsList.count());
/*
    d->processor = new MailProcess(d->settings, d->iface, this);

    connect(d->processor, SIGNAL(signalProgress(int)),
            d->progressBar, SLOT(setValue(int)));

    connect(d->processor, SIGNAL(signalMessage(QString, bool)),
            this, SLOT(slotMessage(QString, bool)));

    connect(d->processor, SIGNAL(signalDone(bool)),
            this, SLOT(slotDone()));

    d->processor->firstStage();
*/
}

void WSFinalPage::cleanupPage()
{
/*
    if (d->processor)
        d->processor->slotCancel();
*/
}

void WSFinalPage::slotMessage(const QString& mess, bool err)
{
    d->progressView->addEntry(mess, err ? DHistoryView::ErrorEntry
                                        : DHistoryView::ProgressEntry);
}

bool WSFinalPage::isComplete() const
{
    return d->complete;
}

} // namespace Digikam
