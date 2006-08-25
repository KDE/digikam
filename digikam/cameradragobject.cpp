/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2003-02-18
 * Description : drag and drop camera management 
 * 
 * Copyright 2003-2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier
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

// Qt includes.

#include <qmime.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qcstring.h>
#include <qwidget.h>
#include <qdatastream.h>

// Local includes.

#include "cameratype.h"
#include "cameradragobject.h"

namespace Digikam
{

CameraDragObject::CameraDragObject(const CameraType& ctype, QWidget *dragSource)
                : QStoredDrag("camera/unknown", dragSource)
{
    setCameraType(ctype);
}

CameraDragObject::~CameraDragObject()
{
}

void CameraDragObject::setCameraType(const CameraType& ctype)
{
    QByteArray byteArray;
    QDataStream ds(byteArray, IO_WriteOnly);

    ds << ctype.title();
    ds << ctype.model();
    ds << ctype.port();
    ds << ctype.path();
    ds << ctype.lastAccess();
    
    setEncodedData(byteArray);    
}

bool CameraDragObject::canDecode(const QMimeSource* e)
{
    return e->provides("camera/unknown");
}

bool CameraDragObject::decode(const QMimeSource* e, CameraType& ctype)
{
    QByteArray payload = e->encodedData("camera/unknown");
    if (payload.size()) 
    {
        QString   title, model, port, path;
        QDateTime lastAccess;

        QDataStream ds(payload, IO_ReadOnly);
        ds >> title;
        ds >> model;
        ds >> port;
        ds >> path;
        ds >> lastAccess;

        ctype = CameraType(title, model, port, path, lastAccess);
        
        return true;
    }
    else
        return false;
}

}  // namespace Digikam
