/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-18
 * Description : drag and drop camera management 
 * 
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <q3cstring.h>
#include <qwidget.h>
#include <qdatastream.h>

// Local includes.

#include "cameratype.h"
#include "cameradragobject.h"

namespace Digikam
{

CameraDragObject::CameraDragObject(const CameraType& ctype, QWidget *dragSource)
                : Q3StoredDrag("camera/unknown", dragSource)
{
    setCameraType(ctype);
}

CameraDragObject::~CameraDragObject()
{
}

void CameraDragObject::setCameraType(const CameraType& ctype)
{
    QByteArray byteArray;
    QDataStream ds(byteArray, QIODevice::WriteOnly);

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

        QDataStream ds(payload, QIODevice::ReadOnly);
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
