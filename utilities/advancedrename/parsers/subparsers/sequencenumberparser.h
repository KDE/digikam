/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : a sequence number parser class
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

#ifndef SEQUENCENUMBERPARSER_H
#define SEQUENCENUMBERPARSER_H

// Qt includes

#include <QString>

// KDE includes

#include <kdialog.h>

// Local includes

#include "subparser.h"

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

class SequenceNumberParser : public SubParser
{
    Q_OBJECT

public:

    SequenceNumberParser();
    ~SequenceNumberParser() {};

protected:

    virtual void parseOperation(const QString& parseString, ParseInformation& info, ParseResults& results);

private Q_SLOTS:

    void slotTokenTriggered(const QString& token);

};

} // namespace Digikam

#endif /* SEQUENCENUMBERPARSER_H */
