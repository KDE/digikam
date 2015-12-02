/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-28
 * Description : database statistics dialog
 *
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QStringList>
#include <QString>
#include <QFont>
#include <QTreeWidget>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "daboutdata.h"
#include "coredb.h"
#include "applicationsettings.h"
#include "coredbaccess.h"
#include "digikam_config.h"

namespace Digikam
{

DBStatDlg::DBStatDlg(QWidget* const parent)
    : InfoDlg(parent)
{
    qApp->setOverrideCursor(Qt::WaitCursor);

    setWindowTitle(i18n("Database Statistics"));
    listView()->setHeaderLabels(QStringList() << i18n("Format") << i18n("Count"));

    // get image format statistics
    int totalImages = generateItemsList(DatabaseItem::Image, i18n("Images"));
    int totalVideos = generateItemsList(DatabaseItem::Video, i18n("Videos"));
    int totalAudio  = generateItemsList(DatabaseItem::Audio, i18n("Audio"));
    int total       = totalImages + totalVideos + totalAudio;

    // --------------------------------------------------------

    // To see total count of items at end of list.
    new QTreeWidgetItem(listView(), QStringList() << i18n("Total Items") << QString::number(total));

    // get album statistics
    int albums   = CoreDbAccess().db()->scanAlbums().count();
    new QTreeWidgetItem(listView(), QStringList() << i18n("Albums") << QString::number(albums));

    // get tags statistics
    int tags     = CoreDbAccess().db()->scanTags().count();
    new QTreeWidgetItem(listView(), QStringList() << i18n("Tags") << QString::number(tags));

    // Database Backend information
    QString dbBe = ApplicationSettings::instance()->getDbEngineParameters().databaseType;
    new QTreeWidgetItem(listView(), QStringList() << i18n("Database backend") << dbBe);

    if (dbBe != QLatin1String("QSQLITE"))
    {
        QString internal = ApplicationSettings::instance()->getDbEngineParameters().internalServer ? i18n("Yes") : i18n("No");
        new QTreeWidgetItem(listView(), QStringList() << i18n("Database internal server") << internal);
    }

    qApp->restoreOverrideCursor();
}

DBStatDlg::~DBStatDlg()
{
}

int DBStatDlg::generateItemsList(DatabaseItem::Category category, const QString& title)
{
    // get image format statistics
    QMap<QString, int> stat = CoreDbAccess().db()->getFormatStatistics(category);

    // do not add items if the map is empty
    if (stat.isEmpty())
    {
        return 0;
    }

    int total = 0;
    QMap<QString, QString> map;

    for (QMap<QString, int>::const_iterator it = stat.constBegin(); it != stat.constEnd(); ++it)
    {
        total += it.value();
        map.insert(it.key(), QString::number(it.value()));
    }

    // --------------------------------------------------------

    QTreeWidgetItem* ti = new QTreeWidgetItem(listView(), QStringList() << title << QString());
    QFont ft            = ti->font(0);
    ft.setBold(true);
    ti->setFont(0, ft);
    ti->setFont(1, ft);

    setInfoMap(map);

    ti = new QTreeWidgetItem(listView(), QStringList() << i18n("total") << QString::number(total));
    ti->setFont(0, ft);
    ti->setFont(1, ft);

    // Add space.
    new QTreeWidgetItem(listView(), QStringList());
    new QTreeWidgetItem(listView(), QStringList());

    return total;
}

}  // namespace Digikam
