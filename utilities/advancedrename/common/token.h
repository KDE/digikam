/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : a token class
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef TOKEN_H
#define TOKEN_H

// Qt includes

#include <QList>
#include <QObject>
#include <QString>

class  QAction;

namespace Digikam
{

class Token : public QObject
{
    Q_OBJECT

public:

    Token(const QString& id, const QString& alias, const QString& description);
    ~Token();

    QString  id()          { return m_id; };
    QString  alias()       { return m_alias; };
    QString  description() { return m_description; };
    QAction* action()      { return m_action; };

Q_SIGNALS:

    void signalTokenTriggered(const QString&);

private Q_SLOTS:

    void slotTriggered();

private:

    QString  m_id;
    QString  m_alias;
    QString  m_description;
    QAction* m_action;
};

typedef QList<Token*> TokenList;

} // namespace Digikam


#endif /* TOKEN_H */
