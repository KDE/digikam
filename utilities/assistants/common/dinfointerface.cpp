/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-06
 * Description : template interface to image informations.
 *               This class do not depend of digiKam database library
 *               to permit to re-use tools on Showfoto.
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

#include "dinfointerface.h"

namespace Digikam
{

DInfoInterface::DInfoInterface(QObject* const parent)
    : QObject(parent)
{
}

DInfoInterface::~DInfoInterface()
{
}

QList<QUrl> DInfoInterface::currentSelectedItems() const
{
    return QList<QUrl>();
}

QList<QUrl> DInfoInterface::currentAlbumItems() const
{
    return QList<QUrl>();
}

QList<QUrl> DInfoInterface::allAlbumItems() const
{
    return QList<QUrl>();
}

int DInfoInterface::currentAlbum() const
{
    return 0;
}

QList<QUrl> DInfoInterface::albumItems(int) const
{
    return QList<QUrl>();
}

DInfoInterface::DInfoMap DInfoInterface::albumInfo(const QUrl&) const
{
    return DInfoMap();
}

DInfoInterface::DInfoMap DInfoInterface::itemInfo(const QUrl&) const
{
    return DInfoMap();
}

}  // namespace Digikam
