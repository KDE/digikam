/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-01-12
 * Description : a DCOP interface.
 * 
 * Copyright (C) 2005 by Leonid Zeitlin <lz@europe.com>
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

// Local includes.

#include "dcopiface.h"
#include "dcopiface.moc"

namespace Digikam
{

DCOPIface::DCOPIface(QObject *parent, const char *name)
         : QObject(parent, name), DCOPObject(name)
{
}

DCOPIface::~DCOPIface()
{
}

void DCOPIface::detectCamera()
{
    emit signalCameraAutoDetect();
}
 
void DCOPIface::downloadFrom( const QString &folder)
{
      emit signalDownloadImages( folder );
}

}  // namespace Digikam

