/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a class to manipulate the results of an renaming options
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

#ifndef MODIFIER_H
#define MODIFIER_H

// Qt includes

#include <QPixmap>
#include <QList>
#include <QString>

// Local includes

#include "parseobject.h"

class  QAction;

namespace Digikam
{

class Modifier : public ParseObject
{
    Q_OBJECT

public:

    Modifier(const QString& name, const QString& description, const QPixmap& icon = QPixmap());
    virtual ~Modifier() {};

    virtual QString modify(const QString& parseString, const QString& result);

protected:

    virtual QString modifyOperation(const QString& parseString, const QString& result) = 0;
};

typedef QList<Modifier*> ModifierList;

} // namespace Digikam


#endif /* MODIFIER_H */
