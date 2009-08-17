/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-11
 * Description : the main parser object for manual rename
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

#ifndef MANUALRENAMEPARSER_H
#define MANUALRENAMEPARSER_H

// Qt includes

#include <QList>
#include <QString>

// Local includes

#include "digikam_export.h"

class QStringList;

namespace Digikam
{
namespace ManualRename
{

class Parser;
class ParseInformation;

class DIGIKAM_EXPORT ManualRenameParser
{

public:

    ManualRenameParser();
    ~ManualRenameParser();

    QString parse(const QString& parseString, const ParseInformation& info);
    QList<Parser*> parsers() const { return m_parsers; };

private:

     QList<Parser*> m_parsers;
     int            extractTokens(QString& parseString, QStringList& tokens);
     QString        tokenMarker(int index);
};

}  // namespace ManualRename
}  // namespace Digikam


#endif /* MANUALRENAMEPARSER_H */
