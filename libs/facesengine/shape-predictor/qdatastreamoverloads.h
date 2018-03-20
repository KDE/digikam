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

#ifndef QDATASTREAM_OVERLOADS_H
#define QDATASTREAM_OVERLOADS_H

// Qt includes

#include <QDataStream>

// Local includes

#include "digikam_export.h"

DIGIKAM_DATABASE_EXPORT QDataStream& operator >> (QDataStream& dataStream, unsigned long& in);

DIGIKAM_DATABASE_EXPORT QDataStream& operator << (QDataStream& dataStream, const unsigned long& in);

#endif // QDATASTREAM_OVERLOADS_H
