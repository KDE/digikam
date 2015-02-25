/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : database key selector.
 *
 * Copyright (C) 2010-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef DBKEYSELECTOR_H
#define DBKEYSELECTOR_H

// Qt includes

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QStringList>

// Local includes

#include "searchtextbar.h"
#include "databaseoption.h"

namespace Digikam
{
class DbHeaderListItem;

class DbKeySelectorItem : public QTreeWidgetItem
{

public:

    DbKeySelectorItem(DbHeaderListItem* const parent, const QString& title, const QString& desc);
    virtual ~DbKeySelectorItem();

    QString key()         const;
    QString description() const;

private:

    DbKeySelectorItem(const DbKeySelectorItem&);
    DbKeySelectorItem& operator=(const DbKeySelectorItem&);

private:

    QString m_key;
    QString m_description;
};

// ------------------------------------------------------------------------------------

class DbKeySelector : public QTreeWidget
{

public:

    explicit DbKeySelector(QWidget* const parent);
    virtual ~DbKeySelector();

    void setKeysMap(const DbOptionKeysMap& map);
    QStringList checkedKeysList();

private:

    DbKeySelector(const DbKeySelector&);
    DbKeySelector& operator=(const DbKeySelector&);
};

// ------------------------------------------------------------------------------------

class DbKeySelectorView : public QWidget
{
    Q_OBJECT

public:

    explicit DbKeySelectorView(QWidget* const parent);
    virtual ~DbKeySelectorView();

    void setKeysMap(const DbOptionKeysMap& map);
    QStringList checkedKeysList() const;

private Q_SLOTS:

    void slotSearchTextChanged(const SearchTextSettings&);

private:

    DbKeySelectorView(const DbKeySelectorView&);
    DbKeySelectorView& operator=(const DbKeySelectorView&);

    void removeChildlessHeaders();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* DBKEYSELECTOR_H */
