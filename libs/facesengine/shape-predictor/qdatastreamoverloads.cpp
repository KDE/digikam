/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 16/08/2016
 * Description : Serialization utilities to help making long
 *               serialization platform independent
 *
 * Copyright (C) 2016 by Omar Amin <Omar dot moh dot amin at gmail dot com>
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

#include "qdatastreamoverloads.h"

QDataStream& operator >> (QDataStream& dataStream, unsigned long& in)
{
    qint64 x;
    dataStream >> x;
    in = x;

    return dataStream;
}

QDataStream& operator << (QDataStream& dataStream, const unsigned long& in)
{
    qint64 x = in;
    dataStream << x;

    return dataStream;
}
