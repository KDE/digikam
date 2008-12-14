/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-04-07
 * Description : Raw camera list dialog
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qlayout.h>
#include <qstringlist.h>
#include <qstring.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qheader.h>

// KDE includes.

#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kaboutdata.h>

// LibKDcraw includes.

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000106
#include <libkdcraw/dcrawbinary.h>
#endif

// Local includes.

#include "searchtextbar.h"
#include "rawcameradlg.h"
#include "rawcameradlg.moc"

namespace Digikam
{

class RawCameraDlgPriv
{
public:

    RawCameraDlgPriv()
    {
        listView  = 0;
        searchBar = 0;
    }

    QListView     *listView;

    SearchTextBar *searchBar;
};

RawCameraDlg::RawCameraDlg(QWidget *parent)
            : KDialogBase(parent, 0, true, QString(), Help|Ok, Ok, true)
{
    setHelp("digitalstillcamera.anchor", "digikam");
    setCaption(i18n("List of supported RAW cameras"));

    d = new RawCameraDlgPriv;

    QWidget *page     = makeMainWidget();
    QGridLayout* grid = new QGridLayout(page, 2, 2, 0, spacingHint());

#if KDCRAW_VERSION < 0x000106
    QStringList list      = KDcrawIface::DcrawBinary::instance()->supportedCamera();
    QString     dcrawVer  = KDcrawIface::DcrawBinary::instance()->internalVersion();
#else
    QStringList list      = KDcrawIface::KDcraw::supportedCamera();
    QString     librawVer = KDcrawIface::KDcraw::librawVersion();
#endif
    QString     KDcrawVer = KDcrawIface::KDcraw::version();

    // --------------------------------------------------------

    QLabel *logo            = new QLabel(page);
    KIconLoader* iconLoader = KApplication::kApplication()->iconLoader();

    if (KApplication::kApplication()->aboutData()->appName() == QString("digikam"))
        logo->setPixmap(iconLoader->loadIcon("digikam", KIcon::NoGroup, 96, KIcon::DefaultState, 0, true));
    else
        logo->setPixmap(iconLoader->loadIcon("showfoto", KIcon::NoGroup, 96, KIcon::DefaultState, 0, true));

    // --------------------------------------------------------

    QLabel *header = new QLabel(page);
#if KDCRAW_VERSION < 0x000106
    header->setText(i18n("<p>Using KDcraw library version %1"
                         "<p>Using Dcraw program version %2"
                         "<p>%3 models in the list")
                         .arg(KDcrawVer).arg(dcrawVer).arg(list.count()));
#else
    header->setText(i18n("<p>Using KDcraw library version %1"
                         "<p>Using LibRaw version %2"
                         "<p>%3 models in the list")
                         .arg(KDcrawVer).arg(librawVer).arg(list.count()));
#endif

    // --------------------------------------------------------

    d->searchBar = new SearchTextBar(page, "RawCameraDlgSearchBar");
    d->listView  = new QListView(page);
    d->listView->addColumn("Camera Model");       // Header is hiden. No i18n here.
    d->listView->setSorting(1);
    d->listView->header()->hide();
    d->listView->setResizeMode(QListView::LastColumn);

    for (QStringList::Iterator it = list.begin() ; it != list.end() ; ++it)
        new QListViewItem(d->listView, *it);

    // --------------------------------------------------------


    grid->addMultiCellWidget(logo,         0, 0, 0, 0);
    grid->addMultiCellWidget(header,       0, 0, 1, 2);
    grid->addMultiCellWidget(d->listView,  1, 1, 0, 2);
    grid->addMultiCellWidget(d->searchBar, 2, 2, 0, 2);
    grid->setColStretch(1, 10);
    grid->setRowStretch(1, 10);

    // --------------------------------------------------------

    connect(d->searchBar, SIGNAL(signalTextChanged(const QString&)),
            this, SLOT(slotSearchTextChanged(const QString&)));

    resize(500, 500);
}

RawCameraDlg::~RawCameraDlg()
{
    delete d;
}

void RawCameraDlg::slotSearchTextChanged(const QString& filter)
{
    bool query     = false;
    QString search = filter.lower();

    QListViewItemIterator it(d->listView);

    for ( ; it.current(); ++it )
    {
        QListViewItem *item  = it.current();

        if (item->text(0).lower().contains(search))
        {
            query = true;
            item->setVisible(true);
        }
        else
        {
            item->setVisible(false);
        }
    }

    d->searchBar->slotSearchResult(query);
}

}  // NameSpace Digikam
