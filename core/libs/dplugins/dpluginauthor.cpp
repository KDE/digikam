/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : author data container for external plugin
 *
 * Copyright (C) 2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dpluginauthor.h"

namespace Digikam
{

DPluginAuthor::DPluginAuthor(const QString& n,
                             const QString& e,
                             const QString& y,
                             const QString& r)
    : name(n),
      email(e),
      years(y),
      roles(r)
{
}

DPluginAuthor::~DPluginAuthor()
{
}

QString DPluginAuthor::toString() const
{
    return (QString::fromLatin1("%1 <%2> %3 [%4]").arg(name).arg(email).arg(years).arg(roles));
}

} // namespace Digikam
