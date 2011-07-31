/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a class to manipulate the results of an renaming options
 *
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include <QList>
#include <QString>

// Local includes

#include "parseable.h"

class QAction;

namespace Digikam
{

class ModifierPriv;
class Modifier : public Parseable
{
    Q_OBJECT

public:

    Modifier(const QString& name, const QString& description);
    Modifier(const QString& name, const QString& description, const QPixmap& icon);
    virtual ~Modifier();

    virtual ParseResults parse(ParseSettings& settings);

protected:

    virtual QString parseOperation(ParseSettings& settings) = 0;

private:

    Modifier(const Modifier&);
    Modifier& operator=(const Modifier&);

private:

    ModifierPriv* const d;
};

typedef QList<Modifier*> ModifierList;

} // namespace Digikam


#endif /* MODIFIER_H */
