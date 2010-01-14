/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a modifier to fill a string with a character to
 *               match a certain length
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

#ifndef FILLMODIFIER_H
#define FILLMODIFIER_H

// Local includes

#include "modifier.h"
#include "parseobjectdialog.h"

namespace Ui
{
    class FillModifierDialogWidget;
}

namespace Digikam
{
class FillDialog : public ParseObjectDialog
{
    Q_OBJECT

public:

    FillDialog(ParseObject* parent);
    ~FillDialog();

    enum Alignment
    {
        Left = 0,
        Right
    };

    Ui::FillModifierDialogWidget* const ui;
};

// --------------------------------------------------------

class FillModifier : public Modifier
{
    Q_OBJECT

public:

    FillModifier();
    virtual QString modifyOperation(const ParseSettings& settings, const QString& str2Modify);

private Q_SLOTS:

    void slotTokenTriggered(const QString& token);
};

} // namespace Digikam


#endif /* FILLMODIFIER_H */
