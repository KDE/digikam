/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to add a sequence number to the parser
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

#ifndef SEQUENCENUMBEROPTION_H
#define SEQUENCENUMBEROPTION_H

// Qt includes

#include <QObject>
#include <QString>

// KDE includes

#include <kdialog.h>

// Local includes

#include "option.h"

namespace Digikam
{

class SequenceNumberDialogPriv;

class SequenceNumberDialog : public KDialog
{
    Q_OBJECT

public:

    SequenceNumberDialog();
    ~SequenceNumberDialog();

    int digits() const;
    int start()  const;
    int step()   const;

private:

    SequenceNumberDialogPriv* const d;
};

// --------------------------------------------------------

class SequenceNumberOption : public Option
{
    Q_OBJECT

public:

    SequenceNumberOption();
    ~SequenceNumberOption() {};

protected:

    virtual void parseOperation(const QString& parseString, ParseInformation& info, ParseResults& results);

private Q_SLOTS:

    void slotTokenTriggered(const QString& token);

};

} // namespace Digikam

#endif /* SEQUENCENUMBEROPTION_H */
