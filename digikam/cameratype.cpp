/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2003-01-29
 * Description : Camera settings container.
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

// KDE includes.

#include <kaction.h>

// Local includes.

#include "cameratype.h"

namespace Digikam
{

class CameraTypePrivate
{
public:

    CameraTypePrivate()
    {
        action = 0;
    }

    QString   title;
    QString   model;
    QString   port;
    QString   path;

    QDateTime lastAccess;

    KAction  *action;
    bool      valid;

    QGuardedPtr<CameraUI> currentCameraUI;
};

CameraType::CameraType()
{
    d = new CameraTypePrivate;
    d->valid  = false;    
}

CameraType::CameraType(const QString& title, const QString& model,
                       const QString& port, const QString& path,
                       const QDateTime& lastAccess, KAction *action)
{
    d = new CameraTypePrivate;
    d->title      = title;
    d->model      = model;
    d->port       = port;
    d->path       = path;
    d->action     = action;
    d->lastAccess = lastAccess;
    d->valid      = true;    
}

CameraType::~CameraType()
{
    delete d;
}

CameraType::CameraType(const CameraType& ctype)
{
    d->title      = ctype.d->title;
    d->model      = ctype.d->model;
    d->port       = ctype.d->port;
    d->path       = ctype.d->path;
    d->action     = ctype.d->action;
    d->lastAccess = ctype.d->lastAccess;
    d->valid      = ctype.d->valid;
}

CameraType& CameraType::operator=(const CameraType& ctype)
{
    if (this != &ctype) 
    {
        d->title      = ctype.d->title;
        d->model      = ctype.d->model;
        d->port       = ctype.d->port;
        d->path       = ctype.d->path;
        d->action     = ctype.d->action;
        d->lastAccess = ctype.d->lastAccess;
        d->valid      = ctype.d->valid;
    }
    return *this;
}

void CameraType::setTitle(const QString& title)
{
    d->title = title;
}

void CameraType::setModel(const QString& model)
{
    d->model = model;
}

void CameraType::setPort(const QString& port)
{
    d->port = port;
}

void CameraType::setPath(const QString& path)
{
    d->path = path;
}

void CameraType::setLastAccess(const QDateTime& lastAccess)
{
    d->lastAccess = lastAccess;
}

void CameraType::setAction(KAction *action)
{
    d->action = action;
}

void CameraType::setValid(bool valid)
{
    d->valid = valid;
}

void CameraType::setCurrentCameraUI(CameraUI *cameraui)
{
    d->currentCameraUI = cameraui;
}

QString CameraType::title() const
{
    return d->title;
}

QString CameraType::model() const
{
    return d->model;
}

QString CameraType::port() const
{
    return d->port;
}

QString CameraType::path() const
{
    return d->path;
}

QDateTime CameraType::lastAccess() const
{
    return d->lastAccess;
}

KAction* CameraType::action() const
{
    return d->action;
}

bool CameraType::valid() const
{
    return d->valid;
}

CameraUI *CameraType::currentCameraUI() const
{
    return d->currentCameraUI;
}

}  // namespace Digikam
