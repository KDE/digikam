/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date   : 2005-05-19
 * Description : a dialog to perform simple search in albums
 *
 * Copyright 2005 by Renchi Raju
 *           2006 by Gilles Caulier
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

#include <qtimer.h>
#include <qlayout.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klineedit.h>
#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>

// Local includes.

#include "searchresultsview.h"
#include "searchquickdialog.h"

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

    KLineEdit         *searchEdit;
    KLineEdit         *nameEdit;

    SearchResultsView *resultsView;
};

SearchQuickDialog::SearchQuickDialog(QWidget* parent, KURL& url)
                 : KDialogBase(Plain, i18n("Quick Search"), Help|Ok|Cancel, Ok, 
                               parent, 0, true, true), m_url(url)
{
    d = new SearchQuickDialogPriv;
    d->timer = new QTimer(this);
    setHelp("quicksearchtool.anchor", "digikam");
    
    QGridLayout* grid = new QGridLayout(plainPage(), 2, 2, 0, spacingHint());
    
    QLabel *label1 = new QLabel("<b>" + i18n("Search:") + "</b>", plainPage());
    d->searchEdit  = new KLineEdit(plainPage());
    QWhatsThis::add( d->searchEdit, i18n("<p>Enter here your search arguments in albums library"));
    
    d->resultsView = new SearchResultsView(plainPage());
    d->resultsView->setMinimumSize(320, 200);
    QWhatsThis::add( d->resultsView, i18n("<p>Here you can see the items found in albums library "
                                          "using the current search arguments"));
    
    QLabel *label2 = new QLabel(i18n("Save search as:"), plainPage());
    d->nameEdit    = new KLineEdit(plainPage());
    d->nameEdit->setText(i18n("Last Search"));
    QWhatsThis::add( d->nameEdit, i18n("<p>Enter here the name used to save the current search in "
                                       "\"My Searches\" view"));

    grid->addMultiCellWidget(label1, 0, 0, 0, 0);
    grid->addMultiCellWidget(d->searchEdit, 0, 0, 1, 2);
    grid->addMultiCellWidget(d->resultsView, 1, 1, 0, 2);
    grid->addMultiCellWidget(label2, 2, 2, 0, 1);
    grid->addMultiCellWidget(d->nameEdit, 2, 2, 2, 2);
    
    connect(d->searchEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotSearchChanged(const QString&)));

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotTimeOut()));

    enableButtonOK(false);
    resize(configDialogSize("QuickSearch Dialog"));

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
            d->timer->start(0, true);
        }
    }
}

SearchQuickDialog::~SearchQuickDialog()
{
    saveDialogSize(("QuickSearch Dialog"));
    delete d->timer;    
    delete d;
}

void SearchQuickDialog::slotTimeOut()
{
    if (d->searchEdit->text().isEmpty())
    {
        d->resultsView->clear();
        enableButtonOK(false);
        return;
    }

    enableButtonOK(true);
    
    KURL url;
    url.setProtocol("digikamsearch");

    QString path, num;
    int     count = 0;
    
    QStringList textList = QStringList::split(' ', d->searchEdit->text());
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
    d->timer->start(500, true);    
}

void SearchQuickDialog::hideEvent(QHideEvent* e)
{
    m_url.removeQueryItem("name");
    m_url.addQueryItem("name", d->nameEdit->text().isEmpty() ?
                       i18n("Last Search") : d->nameEdit->text());
    KDialogBase::hideEvent(e);
}

}  // namespace Digikam

#include "searchquickdialog.moc"
