/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-18
 * Description : 
 * 
 * Copyright 2003 by Renchi Raju
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
#include <qcstring.h>
#include <qwidget.h>
#include <qdatastream.h>

// Local includes.

#include "cameratype.h"
#include "cameradragobject.h"

namespace Digikam
{

CameraDragObject::CameraDragObject(const CameraType& ctype,
                                   QWidget *dragSource)
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
    
    setEncodedData(byteArray);    
}


bool CameraDragObject::canDecode(const QMimeSource* e)
{
    return e->provides("camera/unknown");
}

bool CameraDragObject::decode(const QMimeSource* e,
                              CameraType& ctype)
{
    QByteArray payload = e->encodedData("camera/unknown");
    if (payload.size()) {

        QString title, model, port, path;

        QDataStream ds(payload, IO_ReadOnly);
        ds >> title;
        ds >> model;
        ds >> port;
        ds >> path;

        ctype = CameraType(title, model, port, path);
        
        return true;
    }
    else
        return false;
}

}  // namespace Digikam
