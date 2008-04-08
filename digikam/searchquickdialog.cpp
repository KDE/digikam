/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-19
 * Description : a dialog to perform simple search in albums
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

/** @file searchquickdialog.cpp */

// Qt includes.

#include <QTimer>
#include <QStringList>
#include <QDateTime>
#include <QLabel>
#include <QGridLayout>
#include <QHideEvent>

// KDE includes.

#include <klineedit.h>
#include <klocale.h>
#include <kurl.h>

// Local includes.

#include "ddebug.h"
#include "searchtextbar.h"
#include "searchresultsview.h"
#include "searchquickdialog.h"
#include "searchquickdialog.moc"

namespace Digikam
{

class SearchQuickDialogPriv
{
public:

    SearchQuickDialogPriv()
    {
        timer       = 0;
        searchEdit  = 0;
        nameEdit    = 0;
        resultsView = 0;
    }

    QTimer            *timer;

    KLineEdit         *nameEdit;

    SearchTextBar     *searchEdit;

    SearchResultsView *resultsView;
};

SearchQuickDialog::SearchQuickDialog(QWidget* parent, KUrl& url)
                 : KDialog(parent), m_url(url)
{
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    setModal(true);
    setCaption(i18n("Quick Search"));
    setHelp("quicksearchtool.anchor", "digikam");

    d = new SearchQuickDialogPriv;
    d->timer = new QTimer(this);

    QWidget *w = new QWidget(this);
    setMainWidget(w);
    
    QGridLayout* grid = new QGridLayout(w);

    // -------------------------------------------------------------
    
    QLabel *label1 = new QLabel("<b>" + i18n("Search:") + "</b>", w);
    d->searchEdit  = new SearchTextBar(w, "SearchQuickDialogSearchEdit", i18n("Enter your search criteria"));
    d->searchEdit->setWhatsThis(i18n("<p>Enter your search criteria to find items in the album library"));
    
    d->resultsView = new SearchResultsView(w);
    d->resultsView->setMinimumSize(320, 200);
    d->resultsView->setWhatsThis( i18n("<p>Here you can see the items found in album library "
                                       "using the current search criteria"));
    
    QLabel *label2 = new QLabel(i18n("Save search as:"), w);
    d->nameEdit    = new KLineEdit(w);
    d->nameEdit->setClearButtonShown(true);
    d->nameEdit->setText(i18n("Last Search"));
    d->nameEdit->setClickMessage(i18n("Enter the name of the current search"));
    d->nameEdit->setWhatsThis( i18n("<p>Enter the name of the current search to save in the "
                                    "\"My Searches\" view"));

    // -------------------------------------------------------------

    grid->addWidget(label1, 0, 0, 1, 1);
    grid->addWidget(d->searchEdit, 0, 1, 1, 2);
    grid->addWidget(d->resultsView, 1, 0, 1, 3 );
    grid->addWidget(label2, 2, 0, 1, 2 );
    grid->addWidget(d->nameEdit, 2, 2, 1, 1);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    // -------------------------------------------------------------

    connect(d->searchEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotSearchChanged(const QString&)));

    connect(d->resultsView, SIGNAL(signalSearchResultsMatch(bool)),
            d->searchEdit, SLOT(slotSearchResult(bool)));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeOut()));

    enableButtonOk(false);

    //resize(configDialogSize("QuickSearch Dialog"));

    // -------------------------------------------------------------

    // check if we are being passed a valid url
    if (m_url.isValid())
    {
        int count = m_url.queryItem("count").toInt();
        if (count > 0)
        {
            QStringList strList;

            for (int i=1; i<=count; i++)
            {
                QString val = m_url.queryItem(QString::number(i) + ".val");
                if (!strList.contains(val))
                {
                    strList.append(val);
                }
            }
            
            d->searchEdit->setText(strList.join(" "));
            d->nameEdit->setText(url.queryItem("name"));
            d->timer->setSingleShot(true);
            d->timer->start(0);
        }
    }
}

SearchQuickDialog::~SearchQuickDialog()
{
    //saveDialogSize(("QuickSearch Dialog"));
    delete d->timer;    
    delete d;
}

void SearchQuickDialog::slotTimeOut()
{
    if (d->searchEdit->text().isEmpty())
    {
        d->resultsView->clear();
        enableButtonOk(false);
        return;
    }

    enableButtonOk(true);
    
    KUrl url;
    url.setProtocol("digikamsearch");

    QString path, num;
    int     count = 0;
    
    QStringList textList = d->searchEdit->text().split(' ', QString::SkipEmptyParts);
    for (QStringList::iterator it = textList.begin(); it != textList.end(); ++it)
    {
        if (count != 0)
            path += " AND ";

        path += QString(" %1 ").arg(count + 1);

        num = QString::number(++count);
        url.addQueryItem(num + ".key", "keyword");
        url.addQueryItem(num + ".op", "like");
        url.addQueryItem(num + ".val", *it);
    }

    url.setPath(path);
    url.addQueryItem("name", "Live Search");
    url.addQueryItem("count", num);

    m_url = url;
    d->resultsView->openURL(url);
}

void SearchQuickDialog::slotSearchChanged(const QString&)
{
    d->timer->setSingleShot(true);
    d->timer->start(500);    
}

void SearchQuickDialog::hideEvent(QHideEvent* e)
{
    m_url.removeQueryItem("name");
    m_url.addQueryItem("name", d->nameEdit->text().isEmpty() ?
                       i18n("Last Search") : d->nameEdit->text());
    KDialog::hideEvent(e);
}

}  // namespace Digikam
