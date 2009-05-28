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

DBStatDlg::DBStatDlg(QWidget *parent)
         : InfoDlg(parent)
{
    setCaption(i18n("Database Statistic"));
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
    QTreeWidgetItem *ti = new QTreeWidgetItem(listView(), QStringList() << i18n("Total Items") << QString::number(total));
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
}

}  // namespace Digikam
