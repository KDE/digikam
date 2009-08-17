/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an author parser class
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

#include "authorparser.h"
#include "authorparser.moc"

// KDE includes

#include <kiconloader.h>
#include <klocale.h>

namespace Digikam
{
namespace ManualRename
{

AuthorParser::AuthorParser()
          : Parser(i18n("Author"), SmallIcon("list-add-user"))
{
    addToken("[author]", i18n("Author"),
             i18n("author name"));
}

void AuthorParser::parse(QString& parseString, const ParseInformation& info)
{
    Q_UNUSED(parseString)
    Q_UNUSED(info)
};

} // namespace ManualRename
} // namespace Digikam
