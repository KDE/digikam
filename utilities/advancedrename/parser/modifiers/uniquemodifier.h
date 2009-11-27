/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-27
 * Description : a modifier for setting an additional string to a renaming option
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

#ifndef UNIQUEMODIFIER_H
#define UNIQUEMODIFIER_H

// Qt includes

#include <qstringlist.h>

// Local includes

#include "modifier.h"
#include "parseobjectdialog.h"

class KLineEdit;

namespace Digikam
{

//class UniqueDialog : public ParseObjectDialog
//{
//    Q_OBJECT
//
//public:
//
//    UniqueDialog(ParseObject* parent);
//    ~UniqueDialog();
//
//    KLineEdit* valueInput;
//};

// --------------------------------------------------------

class UniqueModifier : public Modifier
{
    Q_OBJECT

public:

    UniqueModifier();
    virtual QString modifyOperation(const QString& parseString, const QString& result);

    virtual void reset();

//private Q_SLOTS:
//
//    void slotTokenTriggered(const QString& token);

private:

    QStringList cache;
};

} // namespace Digikam


#endif /* UNIQUEMODIFIER_H */
