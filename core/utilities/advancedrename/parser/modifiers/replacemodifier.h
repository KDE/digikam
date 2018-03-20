/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-18
 * Description : a modifier for replacing text in a token result
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

#ifndef REPLACEMODIFIER_H
#define REPLACEMODIFIER_H

// Local includes

#include "modifier.h"
#include "ruledialog.h"

class QCheckBox;

namespace Ui
{
class ReplaceModifierDialogWidget;
}

namespace Digikam
{

class ReplaceDialog : public RuleDialog
{
    Q_OBJECT

public:

    explicit ReplaceDialog(Rule* const parent);
    ~ReplaceDialog();

    Ui::ReplaceModifierDialogWidget* const ui;

private:

    ReplaceDialog(const ReplaceDialog&);
    ReplaceDialog& operator=(const ReplaceDialog&);
};

// --------------------------------------------------------

class ReplaceModifier : public Modifier
{
    Q_OBJECT

public:

    ReplaceModifier();
    virtual QString parseOperation(ParseSettings& settings);

private Q_SLOTS:

    void slotTokenTriggered(const QString& token);

private:

    ReplaceModifier(const ReplaceModifier&);
    ReplaceModifier& operator=(const ReplaceModifier&);
};

} // namespace Digikam

#endif /* REPLACEMODIFIER_H */
