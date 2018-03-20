/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to add a sequence number to the parser
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

#ifndef SEQUENCENUMBEROPTION_H
#define SEQUENCENUMBEROPTION_H

// Local includes

#include "option.h"
#include "ruledialog.h"

namespace Ui
{
class SequenceNumberOptionDialogWidget;
}

namespace Digikam
{

class SequenceNumberDialog : public RuleDialog
{
    Q_OBJECT

public:

    explicit SequenceNumberDialog(Rule* const parent);
    ~SequenceNumberDialog();

    Ui::SequenceNumberOptionDialogWidget* const ui;

private:

    SequenceNumberDialog(const SequenceNumberDialog&);
    SequenceNumberDialog& operator=(const SequenceNumberDialog&);
};

// --------------------------------------------------------

class SequenceNumberOption : public Option
{
    Q_OBJECT

public:

    SequenceNumberOption();
    ~SequenceNumberOption();

protected:

    virtual QString parseOperation(ParseSettings& settings);

private Q_SLOTS:

    void slotTokenTriggered(const QString& token);

private:

    SequenceNumberOption(const SequenceNumberOption&);
    SequenceNumberOption& operator=(const SequenceNumberOption&);
};

} // namespace Digikam

#endif /* SEQUENCENUMBEROPTION_H */
