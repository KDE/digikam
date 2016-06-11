/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-05-12
 * @brief  Abstract backend class for reverse geocoding.
 *
 * @author Copyright (C) 2010 by Michael G. Hansen
 *         <a href="mailto:mike at mghansen dot de">mike at mghansen dot de</a>
 * @author Copyright (C) 2010 by Gabriel Voicu
 *         <a href="mailto:ping dot gabi at gmail dot com">ping dot gabi at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
