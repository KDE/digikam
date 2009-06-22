/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-04-07
 * Description : Raw camera list dialog
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "rawcameradlg.h"
#include "rawcameradlg.moc"

// Qt includes

#include <QStringList>
#include <QString>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>
#include <QTreeWidget>
#include <QHeaderView>

// KDE includes

#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>

// LibKDcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

#if KDCRAW_VERSION < 0x000400
#include <libkdcraw/dcrawbinary.h>
#endif

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

    QTreeWidget   *listView;

    SearchTextBar *searchBar;
};

RawCameraDlg::RawCameraDlg(QWidget *parent)
            : InfoDlg(parent), d(new RawCameraDlgPriv)
{

#if KDCRAW_VERSION < 0x000400
    QStringList list      = KDcrawIface::DcrawBinary::instance()->supportedCamera();
    QString     dcrawVer  = KDcrawIface::DcrawBinary::instance()->internalVersion();
    QString     KDcrawVer = KDcrawIface::KDcraw::version();
#else
    QStringList list      = KDcrawIface::KDcraw::supportedCamera();
    QString     librawVer = KDcrawIface::KDcraw::librawVersion();
    QString     KDcrawVer = KDcrawIface::KDcraw::version();
#endif

    // --------------------------------------------------------

    QLabel *header = new QLabel(this);
#if KDCRAW_VERSION < 0x000400
    header->setText(i18np("<p>Using KDcraw library version %2<br/>"
                          "Using Dcraw program version %3<br/>"
                          "1 model in the list</p>",
                          "<p>Using KDcraw library version %2<br/>"
                          "Using Dcraw program version %3<br/>"
                          "%1 models in the list</p>",
                          list.count(), KDcrawVer, dcrawVer));
#else
    header->setText(i18np("<p>Using KDcraw library version %2<br/>"
                          "Using LibRaw version %3<br/>"
                          "1 model in the list</p>",
                          "<p>Using KDcraw library version %2<br/>"
                          "Using LibRaw version %3<br/>"
                          "%1 models in the list</p>",
                          list.count(), KDcrawVer, librawVer));
#endif

    // --------------------------------------------------------

    kapp->setOverrideCursor(Qt::WaitCursor);
    setCaption(i18n("List of supported RAW cameras"));

    d->searchBar = new SearchTextBar(this, "RawCameraDlgSearchBar");

    listView()->setColumnCount(1);
    listView()->setHeaderLabels(QStringList() << "Camera Model"); // Header is hidden. No i18n here.
    listView()->header()->hide();

    for (QStringList::Iterator it = list.begin() ; it != list.end() ; ++it)
        new QTreeWidgetItem(listView(), QStringList() << *it);

    // --------------------------------------------------------

    QGridLayout *grid = dynamic_cast<QGridLayout*>(mainWidget()->layout());
    grid->addWidget(header,       1, 0, 1,-1);
    grid->addWidget(d->searchBar, 3, 0, 1,-1);

    // --------------------------------------------------------

    connect(d->searchBar, SIGNAL(signalSearchTextSettings(const SearchTextSettings&)),
            this, SLOT(slotSearchTextChanged(const SearchTextSettings&)));
}

RawCameraDlg::~RawCameraDlg()
{
    delete d;
}

void RawCameraDlg::slotSearchTextChanged(const SearchTextSettings& settings)
{
    bool query     = false;
    QString search = settings.text.toLower();

    QTreeWidgetItemIterator it(d->listView);
    while (*it)
    {
        QTreeWidgetItem *item  = *it;

        if (item->text(0).toLower().contains(search, settings.caseSensitive))
        {
            query = true;
            item->setHidden(false);
        }
        else
        {
            item->setHidden(true);
        }

        ++it;
    }

    d->searchBar->slotSearchResult(query);
}

}  // namespace Digikam
