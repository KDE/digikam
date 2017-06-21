/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-21
 * Description : a simple web browser dialog based on Qt WebEngine.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "webbrowserdlg.h"

// Qt includes

#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QWebEngineView>
#include <QToolBar>
#include <QDebug>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "statusprogressbar.h"
#include "searchtextbar.h"

namespace Digikam
{

class WebBrowserDlg::Private
{
public:

    Private()
    {
        browser     = 0;
        toolbar     = 0;
        progressbar = 0;
        searchbar   = 0;
    }

public:

    QUrl               home;
    QWebEngineView*    browser;
    QToolBar*          toolbar;
    StatusProgressBar* progressbar;
    SearchTextBar*     searchbar;
};

WebBrowserDlg::WebBrowserDlg(const QUrl& url, QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    setModal(false);
    d->home    = url;
    d->browser = new QWebEngineView(this);

    d->toolbar = new QToolBar(this);
    d->toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Back));
    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Forward));
    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Reload));
    d->toolbar->addAction(d->browser->pageAction(QWebEnginePage::Stop));

    QAction* const gohome = new QAction(QIcon::fromTheme(QLatin1String("go-home")),
                                        i18n("Home"), this);
    d->toolbar->addAction(gohome);

    d->searchbar = new SearchTextBar(this, QLatin1String("WebBrowserDlgSearchBar"));
    d->searchbar->setHighlightOnResult(true);

    d->progressbar = new StatusProgressBar(this);
    d->progressbar->setProgressTotalSteps(100);
    d->progressbar->setProgressText(i18n("Loading..."));

    // ----------------------

    QGridLayout* const grid = new QGridLayout(this);
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->addWidget(d->toolbar,     0, 0, 1, 1);
    grid->addWidget(d->searchbar,   0, 2, 1, 1);
    grid->addWidget(d->browser,     1, 0, 1, 3);
    grid->addWidget(d->progressbar, 2, 0, 1, 3);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(1, 10);
    setLayout(grid);

    // ----------------------

#if QT_VERSION >= 0x050700
    connect(d->browser, SIGNAL(iconChanged(const QIcon&)),
            this, SLOT(slotIconChanged(const QIcon&)));
#endif

    connect(d->browser, SIGNAL(urlChanged(const QUrl&)),
            this, SLOT(slotUrlChanged(const QUrl&)));

    connect(d->browser, SIGNAL(loadStarted()),
            this, SLOT(slotLoadingStarted()));

    connect(d->browser, SIGNAL(loadFinished(bool)),
            this, SLOT(slotLoadingFinished(bool)));

    connect(d->searchbar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            this, SLOT(slotSearchTextChanged(const SearchTextSettings&)));

    connect(d->browser, SIGNAL(loadProgress(int)),
            d->progressbar, SLOT(setProgressValue(int)));

    connect(gohome, SIGNAL(triggered()),
            this, SLOT(slotGoHome()));

    slotGoHome();
}

WebBrowserDlg::~WebBrowserDlg()
{
    delete d;
}

void WebBrowserDlg::slotUrlChanged(const QUrl& url)
{
    d->progressbar->setText(url.toString());
}

void WebBrowserDlg::slotTitleChanged(const QString& title)
{
    setWindowTitle(title);
}

void WebBrowserDlg::slotIconChanged(const QIcon& icon)
{
    setWindowIcon(icon);
}

void WebBrowserDlg::slotLoadingStarted()
{
    d->progressbar->setEnabled(StatusProgressBar::ProgressBarMode);
}

void WebBrowserDlg::slotLoadingFinished(bool b)
{
    d->progressbar->setEnabled(StatusProgressBar::TextMode);

    if (!b)
        d->progressbar->setText(i18n("Cannot load page..."));
}

void WebBrowserDlg::slotLoadingProgress(int v)
{
    d->progressbar->setProgressValue(v);
}

void WebBrowserDlg::slotSearchTextChanged(const SearchTextSettings& settings)
{
    d->browser->findText(
        settings.text,
        (settings.caseSensitive == Qt::CaseInsensitive) ? QWebEnginePage::FindCaseSensitively 
                                                        : QWebEnginePage::FindFlags(),
        [this](bool found)
        {
            d->searchbar->slotSearchResult(found);
        }
    );
}

void WebBrowserDlg::slotGoHome()
{
    d->browser->setUrl(d->home);
}

} // namespace Digikam
