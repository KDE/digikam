/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-15
 * Description : Exiv2 library interface for KDE
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DMETADATADATA_H
#define DMETADATADATA_H

#include <libkexiv2/kexiv2data.h>
#include <libkexiv2/version.h>

namespace Digikam
{

#if KEXIV2_VERSION >= 0x010000
typedef KExiv2Iface::KExiv2Data KExiv2Data;
#else
class KEXIV2_EXPORT KExiv2Data
{
public:

    QByteArray imageComments;
    QByteArray exifData;
    QByteArray iptcData;
    QByteArray xmpData;

};
#endif

}

#endif /* KEXIV2_H */
