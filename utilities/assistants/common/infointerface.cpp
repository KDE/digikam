/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : template interface to image informations.
 *               This class do not depend of digiKam database library
 *               to permeit to re-use tools on Showfoto.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "infointerface.h"

namespace Digikam
{

InfoInterface::InfoInterface(QObject* const parent)
    : QObject(parent)
{
}

InfoInterface::~InfoInterface()
{
}

QList<QUrl> InfoInterface::currentAlbum() const
{
    return QList<QUrl>();
}

QList<QUrl> InfoInterface::currentSelection() const
{
    return QList<QUrl>();
}

QList<QUrl> InfoInterface::allAlbums() const
{
    return QList<QUrl>();
}

InfoInterface::InfoMap InfoInterface::albumInfo(const QUrl&) const
{
    return InfoMap();
}

InfoInterface::InfoMap InfoInterface::itemInfo(const QUrl&) const
{
    return InfoMap();
}

}  // namespace Digikam
