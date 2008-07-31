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

// Qt includes

#include <QStringList>
#include <QString>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>
#include <QTreeWidget>
#include <QHeaderView>

// KDElib includes

#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>

// LibKDcraw includes.

#include <libkdcraw/kdcraw.h>
#include <libkdcraw/dcrawbinary.h>

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

    QTreeWidget   *listView;

    SearchTextBar *searchBar;
};

RawCameraDlg::RawCameraDlg(QWidget *parent)
            : KDialog(parent)
{
    setButtons(Help|Ok);
    setDefaultButton(Ok);
    setModal(true);
    setHelp("digitalstillcamera.anchor", "digikam");
    setCaption(i18n("List of supported RAW camera"));

    d = new RawCameraDlgPriv;

    QWidget *page = new QWidget(this);
    setMainWidget(page);
    QGridLayout* grid = new QGridLayout(page);

    QStringList list      = KDcrawIface::DcrawBinary::instance()->supportedCamera();
    QString     dcrawVer  = KDcrawIface::DcrawBinary::instance()->internalVersion();
    QString     KDcrawVer = KDcrawIface::KDcraw::version();

    // --------------------------------------------------------

    QLabel *logo = new QLabel(page);
    if (KGlobal::mainComponent().aboutData()->appName() == QString("digikam"))
        logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                                .scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        logo->setPixmap(QPixmap(KStandardDirs::locate("data", "showfoto/data/logo-showfoto.png"))
                                .scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // --------------------------------------------------------

    QLabel *header = new QLabel(page);
    header->setText(i18n("<p>Using KDcraw library version %1"
                         "<p>Using Dcraw program version %2"
                         "<p>%3 models in the list",
                         KDcrawVer, dcrawVer, list.count()));

    // --------------------------------------------------------

    d->searchBar = new SearchTextBar(page, "RawCameraDlgSearchBar");
    d->listView  = new QTreeWidget(page);

    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setColumnCount(1);
    d->listView->setHeaderLabels(QStringList() << "Camera Model"); // Header is hiden. No i18n here.
    d->listView->header()->hide();

    for (QStringList::Iterator it = list.begin() ; it != list.end() ; ++it)
        new QTreeWidgetItem(d->listView, QStringList() << *it);

    // --------------------------------------------------------

    grid->addWidget(logo,         0, 0, 1, 1);
    grid->addWidget(header,       0, 1, 1, 2);
    grid->addWidget(d->listView,  1, 0, 1, 2);
    grid->addWidget(d->searchBar, 2, 0, 1, 2);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(1, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(d->searchBar, SIGNAL(textChanged(const QString&)),
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
    QString search = filter.toLower();

    QTreeWidgetItemIterator it(d->listView);
    while (*it)
    {
        QTreeWidgetItem *item  = *it;

        if (item->text(0).toLower().contains(search))
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

}  // NameSpace Digikam
