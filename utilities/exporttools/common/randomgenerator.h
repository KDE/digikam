/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : Simple random string generator
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef RANDOM_GENERATOR_H
#define RANDOM_GENERATOR_H

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/** Generates random string
 */

class DIGIKAM_EXPORT RandomGenerator
{
public:

    explicit RandomGenerator();
    ~RandomGenerator();

    static QString randomString(const int& length);
};

} // namespace Digikam

#endif // RANDOM_GENERATOR_H
