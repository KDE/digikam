/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-16
 * Description : metadata selector.
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

#ifndef DBKEYSELECTOR_H
#define DBKEYSELECTOR_H

// Qt includes

#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QStringList>

// Local includes

#include "digikam_export.h"
#include "searchtextbar.h"
#include "databaseoption.h"

namespace Digikam
{

class DIGIKAM_EXPORT DbKeySelectorItem : public QTreeWidgetItem
{

public:

    DbKeySelectorItem(QTreeWidget* parent, const QString& title, const QString& desc);
    virtual ~DbKeySelectorItem();

    QString key()         const;
    QString description() const;

private:

    QString m_key;
    QString m_description;
};

// ------------------------------------------------------------------------------------

class DIGIKAM_EXPORT DbKeySelector : public QTreeWidget
{

public:

    DbKeySelector(QWidget* parent);
    virtual ~DbKeySelector();

    void setKeysMap(const DbOptionKeysMap& map);
    QStringList checkedKeysList();
};

// ------------------------------------------------------------------------------------

class DbKeySelectorViewPriv;

class DIGIKAM_EXPORT DbKeySelectorView : public QWidget
{
    Q_OBJECT

public:

    DbKeySelectorView(QWidget* parent);
    virtual ~DbKeySelectorView();

    void setKeysMap(const DbOptionKeysMap& map);
    QStringList checkedKeysList() const;

private Q_SLOTS:

    void slotSearchTextChanged(const SearchTextSettings&);

private:

    DbKeySelectorViewPriv* const d;
};

}  // namespace Digikam

#endif /* DBKEYSELECTOR_H */
