/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : Abstract backend class for reverse geocoding.
 *
 * Copyright (C) 2010 by Michael G. Hansen <mike at mghansen dot de>
 * Copyright (C) 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
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

#include "backend-rg.h"

namespace Digikam
{

/**
 * @class RGBackend
 *
 * @brief This class is a base class for Open Street Map and Geonames backends.
 */

/**
 * Constructor
 */
RGBackend::RGBackend(QObject* const parent)
    : QObject(parent)
{
}

/**
 * Destructor
 */
RGBackend::~RGBackend()
{
}

QString RGBackend::getErrorMessage()
{
    return QString();
}

QString RGBackend::backendName()
{
    return QString();
}

} // namespace Digikam
