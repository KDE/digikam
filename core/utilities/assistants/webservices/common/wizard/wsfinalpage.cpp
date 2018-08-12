/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : finish page of export tool, where user can watch upload process.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2018 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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
#include "wsauthentication.h"

namespace Digikam
{

class WSFinalPage::Private
{
public:

    explicit Private(QWizard* const dialog)
      : progressView(0),
        progressBar(0),
        complete(false),
        wizard(0),
        iface(0),
        wsAuth(0)
    {
        wizard = dynamic_cast<WSWizard*>(dialog);

        if (wizard)
        {
            iface    = wizard->iface();
            wsAuth   = wizard->wsAuth();
        }
    }

    DHistoryView*     progressView;
    DProgressWdg*     progressBar;
    bool              complete;
    WSWizard*         wizard;
    DInfoInterface*   iface;
    WSAuthentication* wsAuth;
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
    
    connect(d->wsAuth, SIGNAL(signalProgress(int)),
            d->progressBar, SLOT(setValue(int)));
    connect(d->wsAuth, SIGNAL(signalMessage(QString, bool)),
            this, SLOT(slotMessage(QString, bool)));
    connect(d->wsAuth, SIGNAL(signalDone()),
            this, SLOT(slotDone()));
}

WSFinalPage::~WSFinalPage()
{
    if (d->wsAuth)
    {
        d->wsAuth->slotCancel();
    }

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
        d->progressView->addEntry(i18n("Internal Error"), DHistoryView::ErrorEntry);
        return;
    }

    d->progressView->clear();
    d->progressBar->reset();

    d->progressView->addEntry(i18n("Preparing files..."), DHistoryView::ProgressEntry);
    d->wsAuth->prepareForUpload();

    d->progressView->addEntry(i18n("%1 input items to process", d->wsAuth->numberItemsUpload()), 
                              DHistoryView::ProgressEntry);

    d->progressView->addEntry(i18n("Start transferring process..."), DHistoryView::ProgressEntry);
    d->wsAuth->startTransfer();

    d->progressBar->setMinimum(0);
    d->progressBar->setMaximum(d->wsAuth->numberItemsUpload());
}

void WSFinalPage::cleanupPage()
{
    if (d->wsAuth)
    {
        d->wsAuth->slotCancel();
    }
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
