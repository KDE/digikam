/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-19
 * Description : an option to provide database information to the parser
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef DATABASEOPTION_H
#define DATABASEOPTION_H

// Qt includes

#include <QString>
#include <QMap>

// Local includes

#include "option.h"
#include "ruledialog.h"

class QLineEdit;

namespace Digikam
{
class DbKeysCollection;
class DbKeySelectorView;

class DatabaseOptionDialog : public RuleDialog
{
    Q_OBJECT

public:

    explicit DatabaseOptionDialog(Rule* const parent);
    ~DatabaseOptionDialog();

    DbKeySelectorView* dbkeySelectorView;
    QLineEdit*         separatorLineEdit;

private:

    DatabaseOptionDialog(const DatabaseOptionDialog&);
    DatabaseOptionDialog& operator=(const DatabaseOptionDialog&);
};

// --------------------------------------------------------

typedef QMap<QString, DbKeysCollection*> DbOptionKeysMap;

// --------------------------------------------------------

class DatabaseOption : public Option
{
    Q_OBJECT

public:

    DatabaseOption();
    ~DatabaseOption();

protected:

    virtual QString parseOperation(ParseSettings& settings);

private Q_SLOTS:

    void slotTokenTriggered(const QString& token);

private:

    DatabaseOption(const DatabaseOption&);
    DatabaseOption& operator=(const DatabaseOption&);

    QString parseDatabase(const QString& keyword, ParseSettings& settings);
    void addDbKeysCollection(DbKeysCollection* key);

    void registerKeysCollection();
    void unregisterKeysCollection();

private:

    DbOptionKeysMap m_map;
};

} // namespace Digikam

#endif /* DATABASEOPTION_H */
