/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-28
 * Description : database statistic dialog
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dbstatdlg.h"
#include "dbstatdlg.moc"

// Qt includes

#include <QStringList>
#include <QString>
#include <QLabel>
#include <QFont>
#include <QLayout>
#include <QGridLayout>
#include <QTreeWidget>
#include <QHeaderView>
#include <QMimeData>
#include <QClipboard>

// KDE includes

#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>

// Local includes

#include "daboutdata.h"
#include "albumdb.h"
#include "databaseaccess.h"

namespace Digikam
{

class DBStatDlgPriv
{
public:

    DBStatDlgPriv()
    {
        listView = 0;
    }

    QTreeWidget *listView;
};

DBStatDlg::DBStatDlg(QWidget *parent)
         : KDialog(parent), d(new DBStatDlgPriv)
{
    setButtons(Help|User1|Ok);
    setDefaultButton(Ok);
    setModal(false);
    setHelp("digikam");
    setCaption(i18n("Database Statistic"));
    setButtonText(User1, i18n("Copy to Clipboard"));

    QWidget *page     = new QWidget(this);
    setMainWidget(page);
    QGridLayout* grid = new QGridLayout(page);

    // --------------------------------------------------------

    QLabel *logo = new QLabel(page);
    if (KGlobal::mainComponent().aboutData()->appName() == QString("digikam"))
        logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                                .scaled(92, 92, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else
        logo->setPixmap(QPixmap(KStandardDirs::locate("data", "showfoto/data/logo-showfoto.png"))
                                .scaled(92, 92, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // --------------------------------------------------------

    QLabel *header = new QLabel(page);
    header->setWordWrap(true);
    header->setText(i18n("<font size=\"5\">%1</font><br/><b>Version %2</b>"
                         "<p>%3</p>",
                    KGlobal::mainComponent().aboutData()->programName(),
                    KGlobal::mainComponent().aboutData()->version(),
                    digiKamSlogan().toString()));

    // --------------------------------------------------------

    d->listView = new QTreeWidget(page);
    d->listView->setSortingEnabled(false);
    d->listView->setRootIsDecorated(false);
    d->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    d->listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->listView->setAllColumnsShowFocus(true);
    d->listView->setColumnCount(2);
    d->listView->setHeaderLabels(QStringList() << i18n("Format") << i18n("Count"));
    d->listView->header()->setResizeMode(QHeaderView::Stretch);

    // --------------------------------------------------------

    grid->addWidget(logo,        0, 0, 1, 1);
    grid->addWidget(header,      0, 1, 1, 2);
    grid->addWidget(d->listView, 1, 0, 1, 2);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(1, 10);
    grid->setMargin(0);
    grid->setSpacing(KDialog::spacingHint());

    // --------------------------------------------------------

    connect(this, SIGNAL(user1Clicked()),
            this, SLOT(slotCopy2ClipBoard()));

    resize(400, 500);

    // --------------------------------------------------------

    kapp->setOverrideCursor(Qt::WaitCursor);

    int total                   = 0;
    QMap<QString, int>     stat = DatabaseAccess().db()->getImageFormatStatistics();
    QMap<QString, QString> map;

    for (QMap<QString, int>::const_iterator it = stat.constBegin(); it != stat.constEnd(); ++it)
    {
        total += it.value();
        map.insert(it.key(), QString::number(it.value()));
    }
    setInfoMap(map);

    // To see total count of items at end of list.
    QTreeWidgetItem *ti = new QTreeWidgetItem(d->listView, QStringList() << i18n("Total Items") << QString::number(total));
    QFont ft = ti->font(0);
    ft.setBold(true);
    ti->setFont(0, ft);
    ft = ti->font(1);
    ft.setBold(true);
    ti->setFont(1, ft);

    kapp->restoreOverrideCursor();
}

DBStatDlg::~DBStatDlg()
{
    delete d;
}

void DBStatDlg::setInfoMap(const QMap<QString, QString>& list)
{
    for (QMap<QString, QString>::const_iterator it = list.constBegin(); it != list.constEnd() ; ++it)
        new QTreeWidgetItem(d->listView, QStringList() << it.key() << it.value());
}

void DBStatDlg::slotCopy2ClipBoard()
{
    QString textInfo;

    textInfo.append(KGlobal::mainComponent().aboutData()->programName());
    textInfo.append(" version ");
    textInfo.append(KGlobal::mainComponent().aboutData()->version());
    textInfo.append("\n");

    QTreeWidgetItemIterator it(d->listView);
    while (*it)
    {
        textInfo.append((*it)->text(0));
        textInfo.append(": ");
        textInfo.append((*it)->text(1));
        textInfo.append("\n");
        ++it;
    }

    QMimeData *mimeData = new QMimeData();
    mimeData->setText(textInfo);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

}  // namespace Digikam
