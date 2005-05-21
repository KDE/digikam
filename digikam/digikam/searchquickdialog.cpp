/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-19
 * Description : 
 * 
 * Copyright 2005 by Renchi Raju

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

#include <qlineedit.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qlabel.h>

#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>
#include <kdeversion.h>
#include <kcalendarsystem.h>
#include <kglobal.h>

#include "searchresultsview.h"
#include "searchquickdialog.h"

SearchQuickDialog::SearchQuickDialog(QWidget* parent, KURL& url)
    : KDialogBase(parent, 0, true, i18n("Quick Search"),
                  Ok|Cancel), m_url(url)
{
    QVBox* vbox = new QVBox(this);
    vbox->setSpacing(spacingHint());

    QHBox* hbox = new QHBox(vbox);
    hbox->setSpacing(spacingHint());
    
    new QLabel(i18n("Search:"), hbox);
    m_searchEdit = new QLineEdit(hbox);
    
    m_resultsView = new SearchResultsView(vbox);

    hbox = new QHBox(vbox);
    hbox->setSpacing(spacingHint());

    new QLabel(i18n("Save Search as:"), hbox);
    m_nameEdit = new QLineEdit(hbox);
    m_nameEdit->setText(i18n("Last Search"));
    
    setMainWidget(vbox);

    m_timer = new QTimer(this);
    
    connect(m_searchEdit, SIGNAL(textChanged(const QString&)),
            SLOT(slotSearchChanged(const QString&)));
    connect(m_timer, SIGNAL(timeout()),
            SLOT(slotTimeOut()));

    enableButtonOK(false);
    setInitialSize(QSize(480,400));
    adjustSize();

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
                if (m_url.queryItem(QString::number(i) + ".key") == "imagedate")
                {
                    val.remove('%');
                    val.remove('-');
                }
                    
                if (!strList.contains(val))
                {
                    strList.append(val);
                }
            }

            m_searchEdit->setText(strList.join(" "));
            m_timer->start(0, true);
        }
    }

    // build a lookup table for month names
    const KCalendarSystem* cal = KGlobal::locale()->calendar();
    for (int i=1; i<=12; ++i)
    {
        m_shortMonthsMap[i] = cal->monthName(i, 2000, true);
        m_longMonthsMap[i]  = cal->monthName(i, 2000, false);
    }
}

SearchQuickDialog::~SearchQuickDialog()
{
    delete m_timer;    
}

QString SearchQuickDialog::possibleDate(const QString& str, bool& exact) const
{
    QDate date = QDate::fromString(str, Qt::ISODate);
    if (date.isValid())
    {
        exact = true;
        return date.toString(Qt::ISODate);
    }

    exact = false;

    bool ok;
    int num = str.toInt(&ok);
    if (ok)
    {
        // ok. its an int, does it look like a year?
        if (1970 <= num && num <= QDate::currentDate().year())
        {
            // very sure its a year
            return QString("%1-%-%").arg(num);
        }
    }
    else
    {
        // hmm... not a year. is it a particular month?
        for (int i=1; i<=12; i++)
        {
            if (str.lower() == m_shortMonthsMap[i] ||
                str.lower() == m_longMonthsMap[i])
            {
                QString monGlob;
                monGlob.sprintf("%.2d", i);
                monGlob = "%-" + monGlob + "-%";
                return monGlob;
            }
        }
    }

    return QString::null;
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

        bool exact;
        QString possDate = possibleDate(*it, exact);
        if (!possDate.isEmpty())
        {
            path += QString(" %1 ")
                    .arg(++count);

            if (exact)
            {
                num = QString::number(count);
                url.addQueryItem(num + ".key", "imagedate");
                url.addQueryItem(num + ".op", "eq");
                url.addQueryItem(num + ".val", possDate);
            }
            else
            {
                num = QString::number(count);
                url.addQueryItem(num + ".key", "imagedate");
                url.addQueryItem(num + ".op", "like");
                url.addQueryItem(num + ".val", possDate);
            }

            continue;
        }
        
        path += QString(" ( %1 OR %2 OR %3 OR %4 OR %5 OR %6 ) ")
                .arg(count + 1)
                .arg(count + 2)
                .arg(count + 3)
                .arg(count + 4)
                .arg(count + 5)
                .arg(count + 6);

        num = QString::number(++count);
        url.addQueryItem(num + ".key", "albumname");
        url.addQueryItem(num + ".op", "like");
        url.addQueryItem(num + ".val", *it);

        num = QString::number(++count);
        url.addQueryItem(num + ".key", "imagename");
        url.addQueryItem(num + ".op", "like");
        url.addQueryItem(num + ".val", *it);

        num = QString::number(++count);
        url.addQueryItem(num + ".key", "tagname");
        url.addQueryItem(num + ".op", "like");
        url.addQueryItem(num + ".val", *it);

        num = QString::number(++count);
        url.addQueryItem(num + ".key", "albumcaption");
        url.addQueryItem(num + ".op", "like");
        url.addQueryItem(num + ".val", *it);

        num = QString::number(++count);
        url.addQueryItem(num + ".key", "albumcollection");
        url.addQueryItem(num + ".op", "like");
        url.addQueryItem(num + ".val", *it);

        num = QString::number(++count);
        url.addQueryItem(num + ".key", "imagecaption");
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

#include "searchquickdialog.moc"
