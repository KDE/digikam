/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-19
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju
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

#include <qlineedit.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>

// KDE includes.

#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>

// Local includes.

#include "searchresultsview.h"
#include "searchquickdialog.h"

namespace Digikam
{

SearchQuickDialog::SearchQuickDialog(QWidget* parent, KURL& url)
    : KDialogBase(parent, 0, true, i18n("Quick Search"),
                  Help|Ok|Cancel), m_url(url)
{
    setHelp("quicksearchtool.anchor", "digikam");
    
    QVBox* vbox = new QVBox(this);
    vbox->setSpacing(spacingHint());

    QHBox* hbox = new QHBox(vbox);
    hbox->setSpacing(spacingHint());
    
    new QLabel("<b>" + i18n("Search:") + "</b>", hbox);
    m_searchEdit = new QLineEdit(hbox);
    QWhatsThis::add( m_searchEdit, i18n("<p>Enter here your search arguments in albums library"));
    
    m_resultsView = new SearchResultsView(vbox);
    QWhatsThis::add( m_resultsView, i18n("<p>Here you can see the items found in albums library using the current search arguments"));
    
    hbox = new QHBox(vbox);
    hbox->setSpacing(spacingHint());

    new QLabel(i18n("Save search as:"), hbox);
    m_nameEdit = new QLineEdit(hbox);
    m_nameEdit->setText(i18n("Last Search"));
    QWhatsThis::add( m_nameEdit, i18n("<p>Enter here the name used to save the current search in \"My Searches\" view"));
    
    setMainWidget(vbox);

    m_timer = new QTimer(this);
    
    connect(m_searchEdit, SIGNAL(textChanged(const QString&)),
            SLOT(slotSearchChanged(const QString&)));
    connect(m_timer, SIGNAL(timeout()),
            SLOT(slotTimeOut()));

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
            
            m_searchEdit->setText(strList.join(" "));
            m_nameEdit->setText(url.queryItem("name"));
            m_timer->start(0, true);
        }
    }
}

SearchQuickDialog::~SearchQuickDialog()
{
    saveDialogSize(("QuickSearch Dialog"));
    delete m_timer;    
}

void SearchQuickDialog::slotTimeOut()
{
    if (m_searchEdit->text().isEmpty())
    {
        m_resultsView->clear();
        enableButtonOK(false);
        return;
    }

    enableButtonOK(true);
    
    KURL url;
    url.setProtocol("digikamsearch");

    QString path, num;
    int     count = 0;
    
    QStringList textList = QStringList::split(' ', m_searchEdit->text());
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
    m_resultsView->openURL(url);
}

void SearchQuickDialog::slotSearchChanged(const QString&)
{
    m_timer->start(500, true);    
}

void SearchQuickDialog::hideEvent(QHideEvent* e)
{
    m_url.removeQueryItem("name");
    m_url.addQueryItem("name", m_nameEdit->text().isEmpty() ?
                       i18n("Last Search") : m_nameEdit->text());
    KDialogBase::hideEvent(e);
}

}  // namespace Digikam

#include "searchquickdialog.moc"
