/* ============================================================
 * Author: Leonid Zeitlin <lz@europe.com>
 * Date  : 2005-01-12
 * Description : 
 * 
 * Copyright 2005 by Leonid Zeitlin
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

namespace Digikam
{

DCOPIface::DCOPIface(QObject *parent, const char *name)
    : QObject(parent, name), DCOPObject(name)
{
}

DCOPIface::~DCOPIface()
{
}

void DCOPIface::cameraAutoDetect()
{
    emit signalCameraAutoDetect();
}
 
void DCOPIface::downloadFrom( const QString &folder)
{
      emit signalDownloadImages( folder );
}

}  // namespace Digikam

#include "dcopiface.moc"
