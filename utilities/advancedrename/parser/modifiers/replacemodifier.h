/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-18
 * Description : a modifier for replacing text in a token result
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

#ifndef REPLACEMODIFIER_H
#define REPLACEMODIFIER_H

// Local includes

#include "modifier.h"
#include "parseobjectdialog.h"

class QCheckBox;
class KLineEdit;

namespace Digikam
{

class ReplaceDialog : public ParseObjectDialog
{
    Q_OBJECT

public:

    ReplaceDialog(ParseObject* parent);
    ~ReplaceDialog();

    KLineEdit* source;
    KLineEdit* destination;
    QCheckBox* caseSensitive;
};

// --------------------------------------------------------

class ReplaceModifier : public Modifier
{
    Q_OBJECT

public:

    ReplaceModifier();
    virtual QString modifyOperation(const QString& parseString, const QString& result);

private Q_SLOTS:

    void slotTokenTriggered(const QString& token);
};

} // namespace Digikam


#endif /* REPLACEMODIFIER_H */
